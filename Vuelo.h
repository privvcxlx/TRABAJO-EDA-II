#ifndef VUELO_H
#define VUELO_H

#include <string>
#include "MaxHeap.h"
using namespace std;

// Los vuelos son pocos (40 a 60), por eso van en un arreglo de tamano fijo.
const int MAX_VUELOS = 60;

// Un vuelo se identifica por su codigoVuelo. Cada vuelo tiene su propia
// lista de espera (un max-heap).
struct Vuelo {
    string codigoVuelo;
    string origen;
    string destino;
    string fecha;
    string hora;
    int capacidad;       // asientos del avion
    int ocupados;        // reservas Confirmada + Pendiente + CheckIn
    MaxHeap espera;      // reservas EnEspera de este vuelo

    Vuelo() {
        capacidad = 0;
        ocupados = 0;
    }
};

#endif
