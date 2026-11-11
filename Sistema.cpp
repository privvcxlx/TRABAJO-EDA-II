#include "Sistema.h"
#include "Utilidades.h"
#include <fstream>

Sistema::Sistema() {
    numVuelos = 0;
    contadorLlegada = 0;
    primerId = "";
    ultimoId = "";
}

// ================= VUELOS =================
// Busqueda secuencial: son como maximo 60 vuelos.
int Sistema::indiceVuelo(string codigoVuelo) {
    for (int i = 0; i < numVuelos; i++) {
        if (vuelos[i].codigoVuelo == codigoVuelo) { return i; }
    }
    return -1;
}

Vuelo* Sistema::buscarVuelo(string codigoVuelo) {
    int i = indiceVuelo(codigoVuelo);
    if (i == -1) { return nullptr; }
    return &vuelos[i];
}

int Sistema::getNumVuelos() {
    return numVuelos;
}

Vuelo* Sistema::getVuelo(int i) {
    return &vuelos[i];
}

// Formato de vuelos.csv: codigoVuelo,origen,destino,fecha,hora,capacidad
int Sistema::cargarVuelos(string archivo) {
    ifstream entrada(archivo.c_str());
    if (!entrada.is_open()) { return -1; }

    int cargados = 0;
    string linea;
    while (getline(entrada, linea)) {
        linea = quitarEspacios(linea);
        if (linea == "") { continue; }

        string campos[6];
        if (separarCSV(linea, campos, 6) != 6) { continue; }
        int capacidad;
        if (!textoAEntero(campos[5], capacidad)) { continue; }     // salta un encabezado o una linea mala
        if (capacidad <= 0 || !fechaValida(campos[3]) || !horaValida(campos[4])) { continue; }
        if (indiceVuelo(campos[0]) != -1) { continue; }            // codigo repetido
        if (numVuelos == MAX_VUELOS) { break; }                    // arreglo lleno

        vuelos[numVuelos].codigoVuelo = campos[0];
        vuelos[numVuelos].origen      = campos[1];
        vuelos[numVuelos].destino     = campos[2];
        vuelos[numVuelos].fecha       = campos[3];
        vuelos[numVuelos].hora        = campos[4];
        vuelos[numVuelos].capacidad   = capacidad;
        vuelos[numVuelos].ocupados    = 0;
        numVuelos++;
        cargados++;
    }
    entrada.close();
    return cargados;
}

// ================= ALTA DE UNA RESERVA =================
// Unico lugar donde una reserva entra a las estructuras (lo usan la carga
// masiva y el registro manual), asi las tres siempre quedan iguales.
// Retorna: 0 = insertada, 1 = ID repetido, 2 = vuelo no existe,
//          3 = lista de espera llena.
int Sistema::colocar(Reserva* r, bool& pasoAEspera) {
    pasoAEspera = false;

    if (tabla.buscar(r->idReserva) != nullptr) { return 1; }

    int i = indiceVuelo(r->codigoVuelo);
    if (i == -1) { return 2; }
    Vuelo* vuelo = &vuelos[i];

    // pide asiento pero el vuelo ya esta lleno -> pasa a la lista de espera
    if (ocupaAsiento(r->estado) && vuelo->ocupados >= vuelo->capacidad) {
        r->estado = "EnEspera";
        pasoAEspera = true;
    }

    if (r->estado == "EnEspera") {
        if (vuelo->espera.estaLleno()) { return 3; }
        contadorLlegada++;
        r->ordenLlegada = contadorLlegada;      // turno de llegada: desempata en el heap
        vuelo->espera.insertar(r);
    } else if (ocupaAsiento(r->estado)) {
        vuelo->ocupados++;
    }
    // (una reserva Cancelada no ocupa asiento ni entra al heap)

    tabla.insertar(r);
    arbol.insertar(r);
    return 0;
}

// ================= 1. CARGA MASIVA =================
bool Sistema::cargarReservas(string archivo, ResumenCarga& resumen) {
    ifstream entrada(archivo.c_str());
    if (!entrada.is_open()) { return false; }

    bool esPrimera = true;
    string linea;
    while (getline(entrada, linea)) {
        linea = quitarEspacios(linea);
        if (linea == "") { continue; }
        if (linea.length() >= 9 && linea.substr(0, 9) == "idReserva") { continue; }   // encabezado
        resumen.leidas++;

        Reserva* r = new Reserva();
        if (!leerReservaCSV(linea, r)) {
            resumen.malFormato++;
            delete r;
            continue;
        }

        bool pasoAEspera;
        int resultado = colocar(r, pasoAEspera);
        if (resultado != 0) {
            if (resultado == 1) { resumen.duplicadas++; }
            if (resultado == 2) { resumen.sinVuelo++; }
            if (resultado == 3) { resumen.esperaLlena++; }
            delete r;                           // no entro a ninguna estructura: se libera
            continue;
        }

        resumen.enHash++;
        resumen.enArbol++;
        if (r->estado == "EnEspera") { resumen.enHeap++; }
        if (pasoAEspera)             { resumen.pasadasAEspera++; }

        if (esPrimera) { primerId = r->idReserva; esPrimera = false; }
        ultimoId = r->idReserva;
    }
    entrada.close();
    return true;
}

// ================= 2. REGISTRO DE NUEVA RESERVA =================
bool Sistema::registrar(Reserva* nueva, bool confirmada, string& mensaje) {
    if (nueva->idReserva == "") {
        mensaje = "El ID no puede estar vacio.";
        delete nueva;
        return false;
    }
    if (tabla.buscar(nueva->idReserva) != nullptr) {
        mensaje = "Ya existe una reserva con el ID " + nueva->idReserva + ".";
        delete nueva;
        return false;
    }
    Vuelo* vuelo = buscarVuelo(nueva->codigoVuelo);
    if (vuelo == nullptr) {
        mensaje = "El vuelo " + nueva->codigoVuelo + " no existe.";
        delete nueva;
        return false;
    }
    if (!claseValida(nueva->clase) || !categoriaValida(nueva->categoria) || nueva->precio < 0) {
        mensaje = "Clase, categoria o precio invalido.";
        delete nueva;
        return false;
    }

    // la ruta y la salida se copian del vuelo, asi siempre coinciden con el
    nueva->origen  = vuelo->origen;
    nueva->destino = vuelo->destino;
    nueva->fecha   = vuelo->fecha;
    nueva->hora    = vuelo->hora;

    if (confirmada) { nueva->estado = "Confirmada"; }
    else            { nueva->estado = "Pendiente"; }

    bool pasoAEspera;
    int resultado = colocar(nueva, pasoAEspera);     // si el vuelo esta lleno la deja EnEspera
    if (resultado != 0) {
        mensaje = "La lista de espera del vuelo esta llena.";
        delete nueva;
        return false;
    }

    if (pasoAEspera) { mensaje = "Vuelo lleno: la reserva quedo EnEspera y entro a la lista de espera."; }
    else             { mensaje = "Reserva registrada con estado " + nueva->estado + "."; }
    return true;
}

// ================= 3. BUSQUEDA POR ID =================
Reserva* Sistema::buscar(string idReserva) {
    return tabla.buscar(idReserva);
}

// ================= 4 y 5. CONSULTAS DEL ARBOL =================
int Sistema::listarEnOrden(Reserva* salida[], int maximo) {
    return arbol.enOrden(salida, maximo);
}

int Sistema::consultarRango(string fechaIni, string fechaFin, Reserva* muestra[],
                            int maxMuestra, int& visitados) {
    return arbol.consultarRango(fechaIni, fechaFin, muestra, maxMuestra, visitados);
}

// ================= 6. ACTUALIZACION DE ESTADO =================
// Solo Pendiente -> Confirmada y Confirmada -> CheckIn.
// Las dos transiciones son entre estados que ocupan asiento, asi que no hay
// que tocar el contador del vuelo ni el heap. Como el estado no forma parte
// de la clave del arbol, tampoco hay que mover el nodo: el arbol guarda un
// puntero a la misma reserva y ve el cambio.
bool Sistema::actualizarEstado(string idReserva, string nuevoEstado, string& mensaje) {
    Reserva* r = tabla.buscar(idReserva);
    if (r == nullptr) {
        mensaje = "No existe la reserva " + idReserva + ".";
        return false;
    }
    if (r->estado == "Cancelada") {
        mensaje = "Una reserva Cancelada no se puede reactivar.";
        return false;
    }

    bool permitida = (r->estado == "Pendiente"  && nuevoEstado == "Confirmada") ||
                     (r->estado == "Confirmada" && nuevoEstado == "CheckIn");
    if (!permitida) {
        mensaje = "Transicion no permitida: " + r->estado + " -> " + nuevoEstado +
                  ". Solo Pendiente -> Confirmada y Confirmada -> CheckIn.";
        return false;
    }

    mensaje = "Estado actualizado: " + r->estado + " -> " + nuevoEstado + ".";
    r->estado = nuevoEstado;
    return true;
}

// ================= 7. CANCELACION =================
bool Sistema::cancelar(string idReserva, string& mensaje, Reserva*& promovida) {
    promovida = nullptr;

    Reserva* r = tabla.buscar(idReserva);
    if (r == nullptr) {
        mensaje = "No existe la reserva " + idReserva + ".";
        return false;
    }
    if (r->estado == "Cancelada") {
        mensaje = "La reserva ya estaba Cancelada.";
        return false;
    }

    Vuelo* vuelo = buscarVuelo(r->codigoVuelo);

    if (ocupaAsiento(r->estado)) {
        // CASO A: tenia asiento. Se libera y, si hay lista de espera, sube el
        // pasajero de mayor prioridad (la raiz del heap).
        r->estado = "Cancelada";
        vuelo->ocupados--;
        mensaje = "Reserva cancelada. Se libero un asiento del vuelo " + vuelo->codigoVuelo + ".";

        if (!vuelo->espera.estaVacio()) {
            promovida = vuelo->espera.extraerMax();
            promovida->estado = "Confirmada";
            vuelo->ocupados++;
        }
    } else {
        // CASO B: estaba EnEspera. Se retira del heap desde la posicion donde este.
        int posicion = vuelo->espera.buscarPosicion(r->idReserva);
        vuelo->espera.eliminarEn(posicion);
        r->estado = "Cancelada";
        mensaje = "Reserva cancelada. Se retiro de la lista de espera del vuelo " + vuelo->codigoVuelo + ".";
    }
    return true;
}

// ================= 8. ELIMINACION FISICA =================
// Solo se depuran reservas Canceladas. Una Cancelada no ocupa asiento ni esta
// en ningun heap, asi que basta con sacarla del arbol y de la hash.
// Primero el arbol (necesita la reserva para comparar la clave) y al final
// la hash, que es la que hace el delete de la reserva.
bool Sistema::eliminarFisico(string idReserva, string& mensaje) {
    Reserva* r = tabla.buscar(idReserva);
    if (r == nullptr) {
        mensaje = "No existe la reserva " + idReserva + ".";
        return false;
    }
    if (r->estado != "Cancelada") {
        mensaje = "Solo se pueden eliminar reservas Canceladas (esta es " + r->estado + ").";
        return false;
    }

    arbol.eliminar(r);
    tabla.eliminar(idReserva);
    mensaje = "Reserva " + idReserva + " eliminada de la tabla hash y del arbol.";
    return true;
}

// ================= DATOS =================
int Sistema::totalReservas() {
    return tabla.getCantidad();
}

TablaHash* Sistema::getTabla() {
    return &tabla;
}

ArbolAVL* Sistema::getArbol() {
    return &arbol;
}

string Sistema::getPrimerId() {
    return primerId;
}

string Sistema::getUltimoId() {
    return ultimoId;
}

// ================= VERIFICACION DE CONSISTENCIA =================
bool Sistema::verificarConsistencia(string& detalle) {
    int total = arbol.getCantidad();
    if (tabla.getCantidad() != total) {
        detalle = "La hash y el arbol tienen distinta cantidad de reservas.";
        return false;
    }
    if (!arbol.esValido()) {
        detalle = "El arbol no cumple el orden o el balance AVL.";
        return false;
    }

    Reserva** todas = new Reserva*[total + 1];
    arbol.enOrden(todas, total);

    int ocupadosReales[MAX_VUELOS];
    for (int i = 0; i < MAX_VUELOS; i++) { ocupadosReales[i] = 0; }
    int enEspera = 0;
    bool correcto = true;

    for (int i = 0; i < total && correcto; i++) {
        Reserva* r = todas[i];
        if (tabla.buscar(r->idReserva) != r) {
            detalle = "La reserva " + r->idReserva + " esta en el arbol pero no en la hash.";
            correcto = false;
            break;
        }
        int v = indiceVuelo(r->codigoVuelo);
        if (v == -1) {
            detalle = "La reserva " + r->idReserva + " apunta a un vuelo que no existe.";
            correcto = false;
            break;
        }
        if (ocupaAsiento(r->estado)) { ocupadosReales[v]++; }
        if (r->estado == "EnEspera") {
            enEspera++;
            if (vuelos[v].espera.buscarPosicion(r->idReserva) == -1) {
                detalle = "La reserva " + r->idReserva + " esta EnEspera pero no en el heap.";
                correcto = false;
            }
        }
    }
    delete[] todas;
    if (!correcto) { return false; }

    int enHeaps = 0;
    for (int i = 0; i < numVuelos; i++) {
        if (ocupadosReales[i] != vuelos[i].ocupados) {
            detalle = "El contador de asientos del vuelo " + vuelos[i].codigoVuelo + " no coincide.";
            return false;
        }
        if (vuelos[i].ocupados > vuelos[i].capacidad) {
            detalle = "El vuelo " + vuelos[i].codigoVuelo + " tiene mas pasajeros que asientos.";
            return false;
        }
        if (!vuelos[i].espera.esValido()) {
            detalle = "El heap del vuelo " + vuelos[i].codigoVuelo + " no cumple la propiedad de max-heap.";
            return false;
        }
        enHeaps = enHeaps + vuelos[i].espera.tamano();
    }
    if (enHeaps != enEspera) {
        detalle = "Hay reservas en los heaps que no estan EnEspera.";
        return false;
    }

    detalle = "Hash, arbol y heaps coinciden.";
    return true;
}
