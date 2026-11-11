#include "Menu.h"
#include "Pruebas.h"
#include "Utilidades.h"
#include <iostream>
#include <chrono>
using namespace std;

const string ARCHIVO_VUELOS  = "vuelos.csv";
const string ARCHIVO_PEQUENO = "reservas_pequeno.csv";
const string ARCHIVO_MASIVO  = "reservas_masivo.csv";

static bool finDeEntrada = false;        // true si ya no hay mas teclado que leer

// Todo se lee con getline para no mezclar cin >> con getline.
static string leerLinea(string mensaje) {
    cout << mensaje;
    string texto;
    if (!getline(cin, texto)) {
        finDeEntrada = true;
        return "";
    }
    return quitarEspacios(texto);
}

static void mostrarVuelo(Vuelo* v) {
    cout << "  Vuelo " << v->codigoVuelo << ": " << v->origen << " -> " << v->destino
         << ", sale " << v->fecha << " " << v->hora
         << " | asientos " << v->ocupados << "/" << v->capacidad
         << " | en espera: " << v->espera.tamano() << endl;
}

// ================= 1. CARGA MASIVA =================
static void opcionCargaMasiva(Sistema* s) {
    if (s->getNumVuelos() == 0) {
        int vuelos = s->cargarVuelos(ARCHIVO_VUELOS);
        if (vuelos <= 0) {
            cout << "No se pudo leer " << ARCHIVO_VUELOS << ". Debe estar en la misma carpeta que el programa." << endl;
            return;
        }
        cout << "Vuelos cargados desde " << ARCHIVO_VUELOS << ": " << vuelos << endl;
    } else {
        cout << "Vuelos ya cargados: " << s->getNumVuelos() << endl;
    }

    cout << "Archivo de reservas:" << endl;
    cout << "  1) " << ARCHIVO_PEQUENO << endl;
    cout << "  2) " << ARCHIVO_MASIVO << endl;
    cout << "  3) Otro archivo" << endl;
    string opcion = leerLinea("Opcion: ");
    string archivo = ARCHIVO_MASIVO;
    if (opcion == "1") { archivo = ARCHIVO_PEQUENO; }
    if (opcion == "3") { archivo = leerLinea("Nombre del archivo: "); }

    ResumenCarga resumen;
    chrono::steady_clock::time_point inicio = chrono::steady_clock::now();
    bool abierto = s->cargarReservas(archivo, resumen);
    chrono::steady_clock::time_point fin = chrono::steady_clock::now();

    if (!abierto) {
        cout << "No se pudo abrir el archivo " << archivo << "." << endl;
        return;
    }

    cout << endl << "Reservas leidas del archivo : " << resumen.leidas << endl;
    cout << "Insertadas en la tabla hash : " << resumen.enHash << endl;
    cout << "Insertadas en el arbol AVL  : " << resumen.enArbol << endl;
    cout << "Insertadas en heaps (espera): " << resumen.enHeap << endl;
    if (resumen.pasadasAEspera > 0) { cout << "Pasadas a EnEspera por vuelo lleno: " << resumen.pasadasAEspera << endl; }
    if (resumen.duplicadas > 0)     { cout << "Rechazadas por ID repetido   : " << resumen.duplicadas << endl; }
    if (resumen.sinVuelo > 0)       { cout << "Rechazadas por vuelo inexistente: " << resumen.sinVuelo << endl; }
    if (resumen.malFormato > 0)     { cout << "Rechazadas por formato       : " << resumen.malFormato << endl; }
    if (resumen.esperaLlena > 0)    { cout << "Rechazadas por espera llena  : " << resumen.esperaLlena << endl; }

    cout << "Heaps con pasajeros en espera:" << endl;
    bool hayEspera = false;
    for (int i = 0; i < s->getNumVuelos(); i++) {
        Vuelo* v = s->getVuelo(i);
        if (v->espera.tamano() > 0) {
            cout << "  " << v->codigoVuelo << ": " << v->espera.tamano() << " en espera" << endl;
            hayEspera = true;
        }
    }
    if (!hayEspera) { cout << "  (ninguno)" << endl; }

    double ms = chrono::duration<double, milli>(fin - inicio).count();
    cout << "Tiempo de carga: " << ms << " ms" << endl;
    cout << "Total en el sistema: " << s->totalReservas() << " reservas" << endl;
}

// ================= 2. REGISTRO DE NUEVA RESERVA =================
static void opcionRegistrar(Sistema* s) {
    if (s->getNumVuelos() == 0) {
        cout << "Primero cargue los vuelos con la opcion 1." << endl;
        return;
    }

    string id = leerLinea("ID de la reserva (ej. RES-020001): ");
    if (id == "") { cout << "El ID no puede estar vacio." << endl; return; }
    if (s->buscar(id) != nullptr) {
        cout << "Ya existe una reserva con ese ID." << endl;
        return;
    }

    string codigo = leerLinea("Codigo de vuelo: ");
    Vuelo* vuelo = s->buscarVuelo(codigo);
    if (vuelo == nullptr) {
        cout << "El vuelo " << codigo << " no existe." << endl;
        return;
    }
    mostrarVuelo(vuelo);
    bool hayAsiento = vuelo->ocupados < vuelo->capacidad;
    if (!hayAsiento) { cout << "  El vuelo esta LLENO: la reserva quedara EnEspera." << endl; }

    Reserva* nueva = new Reserva();
    nueva->idReserva = id;
    nueva->codigoVuelo = codigo;
    nueva->dni = leerLinea("DNI: ");
    nueva->pasajero = leerLinea("Nombre del pasajero: ");

    string clase = leerLinea("Clase (1 Economica, 2 Premium, 3 Ejecutiva): ");
    if (clase == "2")      { nueva->clase = "Premium"; }
    else if (clase == "3") { nueva->clase = "Ejecutiva"; }
    else                   { nueva->clase = "Economica"; }

    string categoria = leerLinea("Categoria del cliente (1 Regular, 2 Frecuente, 3 Premium): ");
    if (categoria == "2")      { nueva->categoria = "Frecuente"; }
    else if (categoria == "3") { nueva->categoria = "Premium"; }
    else                       { nueva->categoria = "Regular"; }

    if (!textoADecimal(leerLinea("Precio: "), nueva->precio) || nueva->precio < 0) {
        cout << "Precio invalido. No se registro la reserva." << endl;
        delete nueva;
        return;
    }

    bool confirmada = false;
    if (hayAsiento) {
        string estado = leerLinea("Estado inicial (1 Pendiente, 2 Confirmada): ");
        confirmada = (estado == "2");
    }

    string mensaje;
    bool registrada = s->registrar(nueva, confirmada, mensaje);   // el sistema libera "nueva" si falla
    cout << mensaje << endl;
    if (registrada) { mostrarReserva(s->buscar(id)); }
}

// ================= 3. BUSQUEDA POR ID =================
static void opcionBuscar(Sistema* s) {
    string id = leerLinea("ID de la reserva: ");
    Reserva* r = s->buscar(id);
    if (r == nullptr) {
        cout << "ERROR: no existe ninguna reserva con el ID " << id << "." << endl;
        return;
    }
    mostrarReserva(r);
}

// ================= 4. LISTADO CRONOLOGICO =================
static void opcionListado(Sistema* s) {
    string opcion = leerLinea("Cuantas mostrar (1 = primeras 20, 2 = primeras 50): ");
    int maximo = 20;
    if (opcion == "2") { maximo = 50; }

    Reserva* lista[50];
    int cantidad = s->listarEnOrden(lista, maximo);
    if (cantidad == 0) { cout << "No hay reservas cargadas." << endl; return; }

    cout << "Primeras " << cantidad << " reservas por fecha y hora de salida (recorrido en orden del AVL):" << endl;
    mostrarEncabezadoLista();
    for (int i = 0; i < cantidad; i++) { mostrarReservaLinea(lista[i]); }
}

// ================= 5. CONSULTA POR RANGO DE FECHAS =================
static void opcionRango(Sistema* s) {
    string fechaIni = leerLinea("Fecha inicial (AAAA-MM-DD): ");
    string fechaFin = leerLinea("Fecha final   (AAAA-MM-DD): ");
    if (!fechaValida(fechaIni) || !fechaValida(fechaFin)) {
        cout << "Formato de fecha invalido." << endl;
        return;
    }
    if (fechaIni > fechaFin) {
        cout << "La fecha inicial no puede ser posterior a la final." << endl;
        return;
    }

    const int MUESTRA = 10;
    Reserva* muestra[MUESTRA];
    int visitados;
    int encontradas = s->consultarRango(fechaIni, fechaFin, muestra, MUESTRA, visitados);

    cout << "Reservas entre " << fechaIni << " y " << fechaFin << ": " << encontradas << endl;
    cout << "Nodos visitados: " << visitados << " de " << s->totalReservas()
         << " (el resto de ramas se descarto)" << endl;
    if (encontradas == 0) { return; }

    int aMostrar = encontradas;
    if (aMostrar > MUESTRA) { aMostrar = MUESTRA; }
    cout << "Muestra (primeras " << aMostrar << "):" << endl;
    mostrarEncabezadoLista();
    for (int i = 0; i < aMostrar; i++) { mostrarReservaLinea(muestra[i]); }
}

// ================= 6. ACTUALIZACION DE ESTADO =================
static void opcionActualizar(Sistema* s) {
    string id = leerLinea("ID de la reserva: ");
    Reserva* r = s->buscar(id);
    if (r == nullptr) {
        cout << "ERROR: no existe la reserva " << id << "." << endl;
        return;
    }
    cout << "Estado actual: " << r->estado << endl;

    string opcion = leerLinea("Nuevo estado (1 Confirmada, 2 CheckIn): ");
    string nuevoEstado = "";
    if (opcion == "1") { nuevoEstado = "Confirmada"; }
    if (opcion == "2") { nuevoEstado = "CheckIn"; }
    if (nuevoEstado == "") { cout << "Opcion invalida." << endl; return; }

    string mensaje;
    s->actualizarEstado(id, nuevoEstado, mensaje);
    cout << mensaje << endl;
}

// ================= 7. CANCELACION DE RESERVA =================
static void opcionCancelar(Sistema* s) {
    string id = leerLinea("ID de la reserva a cancelar: ");
    string mensaje;
    Reserva* promovida;
    bool cancelada = s->cancelar(id, mensaje, promovida);
    cout << mensaje << endl;
    if (!cancelada) { return; }

    if (promovida != nullptr) {
        cout << "Promovido desde la lista de espera (ahora Confirmada):" << endl;
        cout << "  " << promovida->idReserva << " - " << promovida->pasajero
             << " - categoria " << promovida->categoria
             << " - turno de llegada " << promovida->ordenLlegada << endl;
    }
}

// ================= 8. ELIMINACION FISICA =================
static void opcionEliminar(Sistema* s) {
    string id = leerLinea("ID de la reserva Cancelada a eliminar: ");
    Reserva* r = s->buscar(id);
    string fecha = "";
    if (r != nullptr) { fecha = r->fecha; }      // se guarda antes de borrar, para verificar despues

    string mensaje;
    bool eliminada = s->eliminarFisico(id, mensaje);
    cout << mensaje << endl;
    if (!eliminada) { return; }

    // Verificacion 1: ya no aparece en la busqueda por ID
    cout << "Verificacion en la hash : ";
    if (s->buscar(id) == nullptr) { cout << "ya no se encuentra (correcto)" << endl; }
    else                          { cout << "TODAVIA APARECE" << endl; }

    // Verificacion 2: ya no aparece en las consultas del arbol (se revisa su fecha)
    int total = s->totalReservas();
    Reserva** delDia = new Reserva*[total + 1];
    int visitados;
    int cantidad = s->consultarRango(fecha, fecha, delDia, total, visitados);
    bool aparece = false;
    for (int i = 0; i < cantidad; i++) {
        if (delDia[i]->idReserva == id) { aparece = true; }
    }
    delete[] delDia;
    cout << "Verificacion en el arbol: ";
    if (!aparece) { cout << "ya no aparece en las reservas del " << fecha << " (correcto)" << endl; }
    else          { cout << "TODAVIA APARECE" << endl; }
    cout << "Reservas en hash: " << s->getTabla()->getCantidad()
         << " | en arbol: " << s->getArbol()->getCantidad() << endl;
}

// ================= 9. LISTA DE ESPERA DE UN VUELO =================
static void opcionListaEspera(Sistema* s) {
    cout << "Vuelos con lista de espera:" << endl;
    bool hay = false;
    for (int i = 0; i < s->getNumVuelos(); i++) {
        Vuelo* v = s->getVuelo(i);
        if (v->espera.tamano() > 0) {
            cout << "  " << v->codigoVuelo << " (" << v->espera.tamano() << ")";
            hay = true;
        }
    }
    if (!hay) { cout << "  (ninguno)"; }
    cout << endl;

    string codigo = leerLinea("Codigo de vuelo: ");
    Vuelo* vuelo = s->buscarVuelo(codigo);
    if (vuelo == nullptr) {
        cout << "El vuelo " << codigo << " no existe." << endl;
        return;
    }
    mostrarVuelo(vuelo);

    Reserva* siguiente = vuelo->espera.verMax();
    if (siguiente == nullptr) {
        cout << "Este vuelo no tiene pasajeros en espera." << endl;
        return;
    }
    cout << "Siguiente a atender (raiz del heap): " << siguiente->idReserva << " - "
         << siguiente->pasajero << " (" << siguiente->categoria << ")" << endl;

    Reserva* ordenada[CAPACIDAD_HEAP];
    int cantidad = vuelo->espera.listarEnOrden(ordenada);     // usa una copia: el heap no cambia
    cout << "Lista completa en orden de prioridad:" << endl;
    cout << ajustar("#", 4) << ajustar("ID", 12) << ajustar("Pasajero", 22)
         << ajustar("Categoria", 11) << "Turno de llegada" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << ajustar(to_string(i + 1), 4) << ajustar(ordenada[i]->idReserva, 12)
             << ajustar(ordenada[i]->pasajero, 22) << ajustar(ordenada[i]->categoria, 11)
             << ordenada[i]->ordenLlegada << endl;
    }
}

// ================= 10. ESTADISTICAS GENERALES =================
static void opcionEstadisticas(Sistema* s) {
    int total = s->totalReservas();
    if (total == 0) { cout << "No hay reservas cargadas." << endl; return; }

    // se recorren todas las reservas con el recorrido en orden del arbol
    Reserva** todas = new Reserva*[total];
    s->listarEnOrden(todas, total);

    const int MAX_DESTINOS = 100;
    string destinos[MAX_DESTINOS];
    int porDestino[MAX_DESTINOS];
    int numDestinos = 0;

    string clases[3]  = {"Economica", "Premium", "Ejecutiva"};
    int porClase[3]   = {0, 0, 0};
    string estados[5] = {"Confirmada", "Pendiente", "EnEspera", "CheckIn", "Cancelada"};
    int porEstado[5]  = {0, 0, 0, 0, 0};

    double ingresos = 0;
    double sumaPrecios = 0;
    Reserva* mayor = todas[0];
    Reserva* menor = todas[0];

    for (int i = 0; i < total; i++) {
        Reserva* r = todas[i];

        sumaPrecios = sumaPrecios + r->precio;
        if (r->estado == "Confirmada" || r->estado == "CheckIn") { ingresos = ingresos + r->precio; }
        if (r->precio > mayor->precio) { mayor = r; }
        if (r->precio < menor->precio) { menor = r; }

        for (int c = 0; c < 3; c++) { if (r->clase == clases[c])   { porClase[c]++; } }
        for (int e = 0; e < 5; e++) { if (r->estado == estados[e]) { porEstado[e]++; } }

        int d = 0;
        while (d < numDestinos && destinos[d] != r->destino) { d++; }
        if (d == numDestinos && numDestinos < MAX_DESTINOS) {     // destino nuevo
            destinos[numDestinos] = r->destino;
            porDestino[numDestinos] = 0;
            numDestinos++;
        }
        if (d < numDestinos) { porDestino[d]++; }
    }

    cout << "Total de reservas : " << total << endl;
    cout << "Ingresos totales  : " << ingresos << "  (Confirmada + CheckIn)" << endl;
    cout << "Precio promedio   : " << sumaPrecios / total << endl;

    cout << endl << "Por destino:" << endl;
    for (int d = 0; d < numDestinos; d++) { cout << "  " << ajustar(destinos[d], 12) << porDestino[d] << endl; }
    cout << "Por clase:" << endl;
    for (int c = 0; c < 3; c++) { cout << "  " << ajustar(clases[c], 12) << porClase[c] << endl; }
    cout << "Por estado:" << endl;
    for (int e = 0; e < 5; e++) { cout << "  " << ajustar(estados[e], 12) << porEstado[e] << endl; }

    cout << endl << "Reserva de mayor precio: " << mayor->idReserva << " - " << mayor->pasajero
         << " - " << mayor->precio << endl;
    cout << "Reserva de menor precio: " << menor->idReserva << " - " << menor->pasajero
         << " - " << menor->precio << endl;

    Vuelo* masEspera = nullptr;
    for (int i = 0; i < s->getNumVuelos(); i++) {
        Vuelo* v = s->getVuelo(i);
        if (masEspera == nullptr || v->espera.tamano() > masEspera->espera.tamano()) { masEspera = v; }
    }
    if (masEspera != nullptr && masEspera->espera.tamano() > 0) {
        cout << "Vuelo con mas pasajeros en espera: " << masEspera->codigoVuelo
             << " (" << masEspera->espera.tamano() << " en espera)" << endl;
    } else {
        cout << "Vuelo con mas pasajeros en espera: ninguno tiene lista de espera" << endl;
    }

    delete[] todas;
}

// ================= MENU =================
static void mostrarMenu() {
    cout << endl;
    cout << "========== SISTEMA DE RESERVAS AEREAS ==========" << endl;
    cout << " 1. Carga masiva (vuelos y reservas)" << endl;
    cout << " 2. Registrar nueva reserva" << endl;
    cout << " 3. Buscar reserva por ID" << endl;
    cout << " 4. Listado cronologico" << endl;
    cout << " 5. Consulta por rango de fechas" << endl;
    cout << " 6. Actualizar estado" << endl;
    cout << " 7. Cancelar reserva" << endl;
    cout << " 8. Eliminacion fisica (reserva Cancelada)" << endl;
    cout << " 9. Lista de espera de un vuelo" << endl;
    cout << "10. Estadisticas generales" << endl;
    cout << "11. Pruebas y tiempos (para el informe)" << endl;
    cout << " 0. Salir" << endl;
}

void ejecutarMenu(Sistema* sistema) {
    while (!finDeEntrada) {
        mostrarMenu();
        string opcion = leerLinea("Opcion: ");
        if (finDeEntrada) { break; }
        cout << endl;

        if (opcion == "0")       { break; }
        else if (opcion == "1")  { opcionCargaMasiva(sistema); }
        else if (opcion == "2")  { opcionRegistrar(sistema); }
        else if (opcion == "3")  { opcionBuscar(sistema); }
        else if (opcion == "4")  { opcionListado(sistema); }
        else if (opcion == "5")  { opcionRango(sistema); }
        else if (opcion == "6")  { opcionActualizar(sistema); }
        else if (opcion == "7")  { opcionCancelar(sistema); }
        else if (opcion == "8")  { opcionEliminar(sistema); }
        else if (opcion == "9")  { opcionListaEspera(sistema); }
        else if (opcion == "10") { opcionEstadisticas(sistema); }
        else if (opcion == "11") { ejecutarPruebas(); }
        else                     { cout << "Opcion invalida." << endl; }
    }
    cout << "Fin del programa." << endl;
}
