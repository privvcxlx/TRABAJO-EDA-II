#include "Pruebas.h"
#include "Sistema.h"
#include "Utilidades.h"
#include <iostream>
#include <chrono>
using namespace std;

// Archivos y fechas que usan las pruebas (coinciden con lo que crea el generador:
// los vuelos salen entre 2026-12-01 y 2027-02-28).
const string PRUEBA_VUELOS   = "vuelos.csv";
const string PRUEBA_RESERVAS = "reservas_masivo.csv";
const string SEMANA_INI = "2026-12-01", SEMANA_FIN = "2026-12-07";
const string MES_INI    = "2026-12-01", MES_FIN    = "2026-12-31";
const string VACIO_INI  = "2030-01-01", VACIO_FIN  = "2030-01-31";

typedef chrono::steady_clock Reloj;

// microsegundos entre dos instantes
static double microsegundos(Reloj::time_point inicio, Reloj::time_point fin) {
    return chrono::duration<double, micro>(fin - inicio).count();
}

static void titulo(string texto) {
    cout << endl << "---------- " << texto << " ----------" << endl;
}

static void filaTiempo(string operacion, string datos, int repeticiones, double valor, string unidad) {
    cout << ajustar(operacion, 48) << ajustar(datos, 18);
    cout << ajustar(to_string(repeticiones), 8);
    cout.precision(3);
    cout << valor << " " << unidad << endl;
    cout.precision(2);
}

static string siNo(bool valor) {
    if (valor) { return "SI"; }
    return "NO";
}

// Dia siguiente de una fecha AAAA-MM-DD (sin anos bisiestos; alcanza para la prueba).
static string siguienteFecha(string fecha) {
    int diasDelMes[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int anio, mes, dia;
    textoAEntero(fecha.substr(0, 4), anio);
    textoAEntero(fecha.substr(5, 2), mes);
    textoAEntero(fecha.substr(8, 2), dia);
    dia++;
    if (dia > diasDelMes[mes - 1]) { dia = 1; mes++; }
    if (mes > 12)                  { mes = 1; anio++; }
    return numeroConCeros(anio, 4) + "-" + numeroConCeros(mes, 2) + "-" + numeroConCeros(dia, 2);
}

// Busqueda secuencial en un arreglo: sirve para comparar contra la hash.
static Reserva* buscarSecuencial(Reserva* arreglo[], int cantidad, const string& id) {
    for (int i = 0; i < cantidad; i++) {
        if (arreglo[i]->idReserva == id) { return arreglo[i]; }
    }
    return nullptr;
}

// Busca en el arreglo una reserva con asiento del vuelo indicado, empezando en "desde".
static int buscarConAsiento(Reserva* arreglo[], int cantidad, string codigoVuelo, int desde) {
    for (int i = desde; i < cantidad; i++) {
        if (arreglo[i]->codigoVuelo == codigoVuelo && ocupaAsiento(arreglo[i]->estado)) { return i; }
    }
    return -1;
}

static void mostrarEspera(Vuelo* vuelo) {
    Reserva* ordenada[CAPACIDAD_HEAP];
    int cantidad = vuelo->espera.listarEnOrden(ordenada);
    for (int i = 0; i < cantidad; i++) {
        cout << "   " << ajustar(to_string(i + 1) + ".", 4) << ajustar(ordenada[i]->idReserva, 12)
             << ajustar(ordenada[i]->categoria, 11) << "turno " << ordenada[i]->ordenLlegada << endl;
    }
}

// ================= CASOS EXTREMOS =================
// Colisiones: 50 IDs cuyo numero difiere en multiplos del tamano de la tabla
// caen en la misma posicion (mismo resto al dividir).
static void pruebaColisiones() {
    TablaHash* tabla = new TablaHash();
    const int CANTIDAD = 50;
    string ids[CANTIDAD];

    for (int i = 0; i < CANTIDAD; i++) {
        ids[i] = "RES-" + numeroConCeros(5 + i * CAPACIDAD_HASH, 6);
        Reserva* r = new Reserva();
        r->idReserva = ids[i];
        tabla->insertar(r);
    }

    int posicion = tabla->funcionHash(ids[0]);
    bool mismaPosicion = true;
    int encontrados = 0;
    for (int i = 0; i < CANTIDAD; i++) {
        if (tabla->funcionHash(ids[i]) != posicion) { mismaPosicion = false; }
        if (tabla->buscar(ids[i]) != nullptr)       { encontrados++; }
    }

    cout << "Colisiones: 50 IDs (" << ids[0] << ", " << ids[1] << ", " << ids[2] << ", ...)" << endl;
    cout << "  Todos caen en la posicion " << posicion << ": " << siNo(mismaPosicion) << endl;
    cout << "  Largo de la cadena en esa posicion: " << tabla->longitudCadena(posicion) << endl;
    cout << "  IDs encontrados al buscarlos: " << encontrados << " de " << CANTIDAD << endl;

    delete tabla;            // el destructor libera los 50 nodos y sus reservas
}

// Orden desfavorable: 1000 fechas estrictamente crecientes. Un arbol binario
// sin balanceo quedaria como una lista de altura 1000.
static int pruebaFechasCrecientes() {
    const int CANTIDAD = 1000;
    ArbolAVL* arbol = new ArbolAVL();
    Reserva** reservas = new Reserva*[CANTIDAD];

    string fecha = "2027-01-01";
    for (int i = 0; i < CANTIDAD; i++) {
        reservas[i] = new Reserva();
        reservas[i]->idReserva = "ORD-" + numeroConCeros(i + 1, 6);
        reservas[i]->fecha = fecha;
        reservas[i]->hora = "08:00";
        arbol->insertar(reservas[i]);
        fecha = siguienteFecha(fecha);
    }

    int altura = arbol->getAltura();
    cout << "Orden desfavorable: 1000 reservas con fechas crecientes (2027-01-01 en adelante)" << endl;
    cout << "  Reservas en el arbol: " << arbol->getCantidad() << endl;
    cout << "  Altura del arbol: " << altura << "  (sin balanceo seria 1000)" << endl;
    cout << "  Arbol valido (orden y balance): " << siNo(arbol->esValido()) << endl;

    delete arbol;                                    // libera los nodos
    for (int i = 0; i < CANTIDAD; i++) { delete reservas[i]; }
    delete[] reservas;
    return altura;
}

// ================= PRUEBA COMPLETA =================
void ejecutarPruebas() {
    cout << "=========== PRUEBAS Y TIEMPOS ===========" << endl;
    cout << "Se usa un sistema aparte con " << PRUEBA_VUELOS << " y " << PRUEBA_RESERVAS << "." << endl;

    Sistema* s = new Sistema();
    if (s->cargarVuelos(PRUEBA_VUELOS) <= 0) {
        cout << "No se pudo leer " << PRUEBA_VUELOS << "." << endl;
        delete s;
        return;
    }

    // ---------- Escenario 1: carga masiva ----------
    ResumenCarga resumen;
    Reloj::time_point t0 = Reloj::now();
    bool abierto = s->cargarReservas(PRUEBA_RESERVAS, resumen);
    Reloj::time_point t1 = Reloj::now();
    if (!abierto || resumen.enHash == 0) {
        cout << "No se pudo leer " << PRUEBA_RESERVAS << "." << endl;
        delete s;
        return;
    }
    double msCarga = microsegundos(t0, t1) / 1000.0;

    int total = s->totalReservas();
    Reserva** todas = new Reserva*[total];           // todas las reservas, en orden de fecha
    s->listarEnOrden(todas, total);

    titulo("Escenario 1: carga masiva");
    cout << "Vuelos: " << s->getNumVuelos() << " | lineas leidas: " << resumen.leidas << endl;
    cout << "Insertadas en hash: " << resumen.enHash << " | en arbol: " << resumen.enArbol
         << " | en heaps: " << resumen.enHeap << endl;
    for (int i = 0; i < s->getNumVuelos(); i++) {
        Vuelo* v = s->getVuelo(i);
        if (v->espera.tamano() > 0) {
            cout << "  Heap del vuelo " << v->codigoVuelo << ": " << v->espera.tamano()
                 << " en espera (asientos " << v->ocupados << "/" << v->capacidad << ")" << endl;
        }
    }

    // las metricas se toman ahora, antes de que las pruebas cambien los datos
    double factorCarga = s->getTabla()->factorCarga();
    int cadenaMasLarga = s->getTabla()->cadenaMasLarga();
    int posicionesUsadas = s->getTabla()->posicionesUsadas();
    int alturaArbol = s->getArbol()->getAltura();

    // ---------- Escenario 2: busqueda por ID ----------
    titulo("Escenario 2: busqueda por ID");
    string idsBuscar[4] = { s->getPrimerId(), todas[total / 2]->idReserva, s->getUltimoId(), "RES-XXXXXX" };
    string nombres[4] = { "Primera    ", "Intermedia ", "Ultima     ", "Inexistente" };
    for (int i = 0; i < 4; i++) {
        Reserva* r = s->buscar(idsBuscar[i]);
        cout << nombres[i] << " " << idsBuscar[i] << ": ";
        if (r != nullptr) { cout << "encontrada (" << r->pasajero << ", " << r->fecha << " " << r->hora << ")" << endl; }
        else              { cout << "no existe" << endl; }
    }

    // ---------- Escenario 4: consulta por rango ----------
    titulo("Escenario 4: consulta por rango");
    Reserva* muestra[5];
    int visitados;
    int enSemana = s->consultarRango(SEMANA_INI, SEMANA_FIN, muestra, 5, visitados);
    cout << "Una semana (" << SEMANA_INI << " a " << SEMANA_FIN << "): " << enSemana
         << " reservas, " << visitados << " nodos visitados de " << total << endl;
    int enMes = s->consultarRango(MES_INI, MES_FIN, muestra, 5, visitados);
    cout << "Un mes     (" << MES_INI << " a " << MES_FIN << "): " << enMes
         << " reservas, " << visitados << " nodos visitados de " << total << endl;
    int enVacio = s->consultarRango(VACIO_INI, VACIO_FIN, muestra, 5, visitados);
    cout << "Sin datos  (" << VACIO_INI << " a " << VACIO_FIN << "): " << enVacio
         << " reservas, " << visitados << " nodos visitados de " << total << endl;

    // ---------- Escenario 7: lista de espera ----------
    titulo("Escenario 7: lista de espera");
    Vuelo* lleno = nullptr;
    for (int i = 0; i < s->getNumVuelos() && lleno == nullptr; i++) {
        if (s->getVuelo(i)->espera.tamano() >= 5) { lleno = s->getVuelo(i); }
    }
    if (lleno == nullptr) {
        cout << "No hay ningun vuelo con 5 o mas pasajeros en espera." << endl;
    } else {
        cout << "Vuelo " << lleno->codigoVuelo << " (asientos " << lleno->ocupados << "/"
             << lleno->capacidad << "). Lista de espera en orden de prioridad:" << endl;
        mostrarEspera(lleno);

        // (a) cancelar una reserva con asiento: debe subir la raiz del heap
        Reserva* esperado = lleno->espera.verMax();
        int posicion = buscarConAsiento(todas, total, lleno->codigoVuelo, 0);
        string mensaje;
        Reserva* promovida;
        s->cancelar(todas[posicion]->idReserva, mensaje, promovida);
        cout << "Se cancela " << todas[posicion]->idReserva << " (tenia asiento)." << endl;
        cout << "  Promovido: " << promovida->idReserva << " (" << promovida->categoria << ", turno "
             << promovida->ordenLlegada << "), ahora " << promovida->estado << endl;
        cout << "  Era el de mayor prioridad: " << siNo(promovida == esperado) << endl;

        // (b) cancelar una reserva EnEspera que NO esta en la raiz
        Reserva* ordenada[CAPACIDAD_HEAP];
        int enLista = lleno->espera.listarEnOrden(ordenada);
        Reserva* delMedio = ordenada[enLista / 2];
        string idDelMedio = delMedio->idReserva;
        int posicionHeap = lleno->espera.buscarPosicion(idDelMedio);
        s->cancelar(idDelMedio, mensaje, promovida);
        cout << "Se cancela " << idDelMedio << " (EnEspera, posicion " << posicionHeap << " del arreglo, no es la raiz)." << endl;
        cout << "  Sigue en el heap: " << siNo(lleno->espera.buscarPosicion(idDelMedio) != -1)
             << " | heap valido: " << siNo(lleno->espera.esValido())
             << " | quedan " << lleno->espera.tamano() << " en espera:" << endl;
        mostrarEspera(lleno);
    }

    // ================= TIEMPOS =================
    const int REP_BUSQUEDA = 1000;
    const int REP_CONSULTA = 100;
    const int REP_INSERCION = 100;
    const int REP_CAMBIOS = 10;

    // -- busqueda por ID: hash contra secuencial, con los mismos 1000 IDs --
    string* idsPrueba = new string[REP_BUSQUEDA];
    for (int i = 0; i < REP_BUSQUEDA; i++) {
        idsPrueba[i] = todas[(int)(((long long)i * 7919) % total)]->idReserva;
    }
    int halladosHash = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_BUSQUEDA; i++) {
        if (s->buscar(idsPrueba[i]) != nullptr) { halladosHash++; }
    }
    t1 = Reloj::now();
    double usHash = microsegundos(t0, t1) / REP_BUSQUEDA;

    int halladosSecuencial = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_BUSQUEDA; i++) {
        if (buscarSecuencial(todas, total, idsPrueba[i]) != nullptr) { halladosSecuencial++; }
    }
    t1 = Reloj::now();
    double usSecuencial = microsegundos(t0, t1) / REP_BUSQUEDA;
    delete[] idsPrueba;

    // -- consulta ordenada (primeras 50) --
    Reserva* primeras[50];
    int sumaOrdenadas = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_CONSULTA; i++) { sumaOrdenadas = sumaOrdenadas + s->listarEnOrden(primeras, 50); }
    t1 = Reloj::now();
    double usOrdenada = microsegundos(t0, t1) / REP_CONSULTA;

    // -- consulta por rango (un mes) --
    int sumaRango = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_CONSULTA; i++) { sumaRango = sumaRango + s->consultarRango(MES_INI, MES_FIN, muestra, 5, visitados); }
    t1 = Reloj::now();
    double usRango = microsegundos(t0, t1) / REP_CONSULTA;

    // -- insercion individual: 100 reservas nuevas (se preparan antes de medir) --
    Reserva* nuevas[REP_INSERCION];
    string idsNuevas[REP_INSERCION];
    int vueloActual = 0;
    for (int i = 0; i < REP_INSERCION; i++) {
        // se busca un vuelo con asientos libres, avanzando en circulo
        int intentos = 0;
        while (intentos < s->getNumVuelos() &&
               s->getVuelo(vueloActual)->ocupados + 3 >= s->getVuelo(vueloActual)->capacidad) {
            vueloActual = (vueloActual + 1) % s->getNumVuelos();
            intentos++;
        }
        idsNuevas[i] = "PRB-" + numeroConCeros(900001 + i, 6);
        nuevas[i] = new Reserva();
        nuevas[i]->idReserva = idsNuevas[i];
        nuevas[i]->dni = "00000000";
        nuevas[i]->pasajero = "Pasajero De Prueba";
        nuevas[i]->codigoVuelo = s->getVuelo(vueloActual)->codigoVuelo;
        nuevas[i]->clase = "Economica";
        nuevas[i]->categoria = "Regular";
        nuevas[i]->precio = 100;
        vueloActual = (vueloActual + 1) % s->getNumVuelos();
    }
    string mensaje;
    int insertadas = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_INSERCION; i++) {
        if (s->registrar(nuevas[i], false, mensaje)) { insertadas++; }      // quedan Pendiente
    }
    t1 = Reloj::now();
    double usInsercion = microsegundos(t0, t1) / REP_INSERCION;

    int nuevasEnHash = 0, nuevasEnArbol = 0;
    for (int i = 0; i < REP_INSERCION; i++) {
        Reserva* r = s->buscar(idsNuevas[i]);
        if (r != nullptr) {
            nuevasEnHash++;
            if (s->getArbol()->contiene(r)) { nuevasEnArbol++; }
        }
    }

    // -- actualizacion de estado: 10 reservas Pendiente -> Confirmada --
    int actualizadas = 0;
    t0 = Reloj::now();
    for (int i = 0; i < REP_CAMBIOS; i++) {
        if (s->actualizarEstado(idsNuevas[i], "Confirmada", mensaje)) { actualizadas++; }
    }
    t1 = Reloj::now();
    double usActualizar = microsegundos(t0, t1) / REP_CAMBIOS;

    // -- cancelacion con promocion: 10 reservas con asiento de vuelos con lista de espera --
    string idsCancelar[REP_CAMBIOS];
    int porCancelar = 0;
    for (int i = 0; i < s->getNumVuelos() && porCancelar < REP_CAMBIOS; i++) {
        Vuelo* v = s->getVuelo(i);
        int tomar = v->espera.tamano();              // no mas cancelaciones que pasajeros en espera
        if (tomar > 3) { tomar = 3; }
        int desde = 0;
        while (tomar > 0 && porCancelar < REP_CAMBIOS) {
            int p = buscarConAsiento(todas, total, v->codigoVuelo, desde);
            if (p == -1) { break; }
            idsCancelar[porCancelar] = todas[p]->idReserva;
            porCancelar++;
            desde = p + 1;
            tomar--;
        }
    }
    int canceladas = 0, promovidas = 0;
    Reserva* promovida;
    t0 = Reloj::now();
    for (int i = 0; i < porCancelar; i++) {
        if (s->cancelar(idsCancelar[i], mensaje, promovida)) { canceladas++; }
        if (promovida != nullptr) { promovidas++; }
    }
    t1 = Reloj::now();
    double usCancelar = 0;
    if (porCancelar > 0) { usCancelar = microsegundos(t0, t1) / porCancelar; }

    // -- eliminacion fisica de esas mismas reservas (ya estan Canceladas) --
    delete[] todas;          // desde aqui el arreglo tendria punteros a reservas borradas: no se usa mas
    int eliminadas = 0;
    t0 = Reloj::now();
    for (int i = 0; i < porCancelar; i++) {
        if (s->eliminarFisico(idsCancelar[i], mensaje)) { eliminadas++; }
    }
    t1 = Reloj::now();
    double usEliminar = 0;
    if (porCancelar > 0) { usEliminar = microsegundos(t0, t1) / porCancelar; }

    int siguenEnHash = 0;
    for (int i = 0; i < porCancelar; i++) {
        if (s->buscar(idsCancelar[i]) != nullptr) { siguenEnHash++; }
    }

    titulo("Tabla de tiempos (seccion Resultados del informe)");
    cout << ajustar("Operacion evaluada", 48) << ajustar("Cantidad de datos", 18)
         << ajustar("Reps", 8) << "Tiempo promedio" << endl;
    string datos = to_string(total) + " reservas";
    filaTiempo("Carga masiva", datos, 1, msCarga, "ms");
    filaTiempo("Busqueda por ID (hash)", datos, REP_BUSQUEDA, usHash, "us");
    filaTiempo("Busqueda secuencial del mismo ID (comparacion)", datos, REP_BUSQUEDA, usSecuencial, "us");
    filaTiempo("Insercion individual", datos, REP_INSERCION, usInsercion, "us");
    filaTiempo("Consulta ordenada (primeras 50)", datos, REP_CONSULTA, usOrdenada, "us");
    filaTiempo("Consulta por rango (un mes)", datos, REP_CONSULTA, usRango, "us");
    filaTiempo("Actualizacion de estado", "10 reservas", REP_CAMBIOS, usActualizar, "us");
    filaTiempo("Cancelacion con promocion desde el heap", "10 reservas", porCancelar, usCancelar, "us");
    filaTiempo("Eliminacion fisica", "10 reservas", porCancelar, usEliminar, "us");
    if (usHash > 0) {
        cout << "La hash fue " << usSecuencial / usHash << " veces mas rapida que la busqueda secuencial." << endl;
    }

    titulo("Comprobaciones de las operaciones medidas (escenarios 5 y 6)");
    cout << "Busquedas con resultado: hash " << halladosHash << "/" << REP_BUSQUEDA
         << ", secuencial " << halladosSecuencial << "/" << REP_BUSQUEDA << endl;
    cout << "Consulta ordenada: " << sumaOrdenadas / REP_CONSULTA << " reservas por consulta"
         << " | rango de un mes: " << sumaRango / REP_CONSULTA << " reservas por consulta" << endl;
    cout << "Inserciones: " << insertadas << " registradas, " << nuevasEnHash << " aparecen en la hash y "
         << nuevasEnArbol << " en el arbol" << endl;
    cout << "Actualizaciones Pendiente -> Confirmada: " << actualizadas << " de " << REP_CAMBIOS << endl;
    cout << "Cancelaciones: " << canceladas << " | promovidos desde el heap: " << promovidas << endl;
    cout << "Eliminaciones fisicas: " << eliminadas << " | siguen apareciendo en la hash: " << siguenEnHash << endl;
    cout << "Total ahora: hash " << s->getTabla()->getCantidad() << ", arbol " << s->getArbol()->getCantidad() << endl;

    // ================= CASOS EXTREMOS =================
    titulo("Escenario 8: casos extremos");
    pruebaColisiones();
    int alturaCreciente = pruebaFechasCrecientes();

    // ================= METRICAS =================
    titulo("Metricas de las estructuras (seccion Resultados del informe)");
    cout << ajustar("Metrica", 58) << ajustar("Esperado", 30) << "Medido" << endl;
    cout << ajustar("Factor de carga de la hash (reservas / tamano)", 58)
         << ajustar("cercano a 1 o menor", 30) << factorCarga
         << "  (" << total << " / " << CAPACIDAD_HASH << ")" << endl;
    cout << ajustar("Longitud de la cadena mas larga de la hash", 58)
         << ajustar("pocos nodos", 30) << cadenaMasLarga
         << "  (" << posicionesUsadas << " posiciones usadas)" << endl;
    cout << ajustar("Altura del arbol con " + to_string(total) + " reservas", 58)
         << ajustar("cerca de 14, maximo 19 (AVL)", 30) << alturaArbol << endl;
    cout << ajustar("Altura tras 1,000 inserciones con fechas crecientes", 58)
         << ajustar("logaritmica, no cerca de 1000", 30) << alturaCreciente << endl;

    // ================= CONSISTENCIA =================
    titulo("Consistencia final entre hash, arbol y heaps");
    string detalle;
    bool consistente = s->verificarConsistencia(detalle);
    cout << "Consistente: " << siNo(consistente) << " (" << detalle << ")" << endl;

    delete s;                // libera todas las reservas, nodos de la hash y nodos del arbol
    cout << endl << "=========== FIN DE LAS PRUEBAS ===========" << endl;
}
