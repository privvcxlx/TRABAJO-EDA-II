#ifndef MAXHEAP_H
#define MAXHEAP_H

#include "Reserva.h"

// Capacidad fija del arreglo de cada lista de espera. El enunciado no da un
// numero, asi que se usa la mayor cantidad de pasajeros certificada para un
// avion comercial (Airbus A380 en configuracion de una sola clase: 853).
const int CAPACIDAD_HEAP = 853;

// MAX-HEAP EN ARREGLO: lista de espera de UN vuelo.
// Prioridad: primero la categoria (Premium > Frecuente > Regular) y, si
// empatan, el que llego antes a la lista (ordenLlegada menor).
class MaxHeap {
private:
    Reserva* datos[CAPACIDAD_HEAP];
    int n;                                   // elementos que hay actualmente

    int padre(int i);
    int izquierdo(int i);
    int derecho(int i);

    bool mayorPrioridad(Reserva* a, Reserva* b);     // true si a se atiende antes que b
    void intercambiar(Reserva* heap[], int i, int j);
    void heapifyUp(Reserva* heap[], int i);
    void heapifyDown(Reserva* heap[], int total, int i);

public:
    MaxHeap();

    bool insertar(Reserva* r);               // false si el arreglo esta lleno
    Reserva* verMax();                       // siguiente a atender (nullptr si esta vacio)
    Reserva* extraerMax();                   // lo saca del heap y lo retorna

    int buscarPosicion(string idReserva);    // busqueda secuencial; -1 si no esta
    bool eliminarEn(int posicion);           // elimina desde una posicion cualquiera

    // Copia en salida[] toda la lista en orden de prioridad SIN alterar el
    // heap (trabaja sobre una copia del arreglo). Retorna cuantos copio.
    int listarEnOrden(Reserva* salida[]);

    int tamano();
    bool estaVacio();
    bool estaLleno();
    bool esValido();                         // ningun hijo tiene mas prioridad que su padre
};

#endif
