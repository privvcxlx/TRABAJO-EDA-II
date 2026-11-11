#ifndef SISTEMA_H
#define SISTEMA_H

#include "Reserva.h"
#include "TablaHash.h"
#include "ArbolAVL.h"
#include "Vuelo.h"

// Lo que paso durante una carga masiva (funcionalidad 1)
struct ResumenCarga {
    int leidas;            // lineas con datos en el archivo
    int enHash;            // insertadas en la tabla hash
    int enArbol;           // insertadas en el arbol
    int enHeap;            // insertadas en algun heap (EnEspera)
    int duplicadas;        // rechazadas: el ID ya existia
    int sinVuelo;          // rechazadas: el vuelo no existe
    int malFormato;        // rechazadas: linea incompleta o con datos invalidos
    int pasadasAEspera;    // decian tener asiento pero el vuelo ya estaba lleno
    int esperaLlena;       // rechazadas: la lista de espera del vuelo esta llena

    ResumenCarga() {
        leidas = 0; enHash = 0; enArbol = 0; enHeap = 0; duplicadas = 0;
        sinVuelo = 0; malFormato = 0; pasadasAEspera = 0; esperaLlena = 0;
    }
};

// SISTEMA DE RESERVAS: aqui se mantiene la CONSISTENCIA entre las tres
// estructuras. Ningun otro modulo inserta o elimina directamente en la hash,
// el arbol o los heaps: todo pasa por los metodos de esta clase.
//
//   Toda reserva        -> esta en la tabla hash Y en el arbol.
//   Reserva EnEspera    -> ademas esta en el heap de su vuelo.
//   Reserva con asiento -> esta contada en "ocupados" de su vuelo.
class Sistema {
private:
    TablaHash tabla;               // clave: idReserva
    ArbolAVL arbol;                // clave: fecha + hora + idReserva
    Vuelo vuelos[MAX_VUELOS];      // cada vuelo tiene su heap de espera
    int numVuelos;
    int contadorLlegada;           // turno que se entrega al entrar a una lista de espera
    string primerId;               // primer y ultimo ID de la ultima carga masiva
    string ultimoId;

    int indiceVuelo(string codigoVuelo);                 // -1 si no existe
    int colocar(Reserva* r, bool& pasoAEspera);          // inserta en las estructuras

public:
    Sistema();

    // 1. Carga masiva
    int cargarVuelos(string archivo);                    // retorna cuantos cargo, -1 si no abre
    bool cargarReservas(string archivo, ResumenCarga& resumen);

    // 2. Registro. El sistema se queda con la reserva: si no se puede
    //    registrar, la libera con delete.
    bool registrar(Reserva* nueva, bool confirmada, string& mensaje);

    // 3. Busqueda por ID (hash)
    Reserva* buscar(string idReserva);

    // 4 y 5. Consultas del arbol
    int listarEnOrden(Reserva* salida[], int maximo);
    int consultarRango(string fechaIni, string fechaFin, Reserva* muestra[],
                       int maxMuestra, int& visitados);

    // 6, 7 y 8
    bool actualizarEstado(string idReserva, string nuevoEstado, string& mensaje);
    bool cancelar(string idReserva, string& mensaje, Reserva*& promovida);
    bool eliminarFisico(string idReserva, string& mensaje);

    // vuelos
    Vuelo* buscarVuelo(string codigoVuelo);              // nullptr si no existe
    int getNumVuelos();
    Vuelo* getVuelo(int i);

    // datos para estadisticas y pruebas
    int totalReservas();
    TablaHash* getTabla();
    ArbolAVL* getArbol();
    string getPrimerId();
    string getUltimoId();

    // Revisa que las tres estructuras digan lo mismo. Si falla, explica en "detalle".
    bool verificarConsistencia(string& detalle);
};

#endif
