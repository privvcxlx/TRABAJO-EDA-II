#ifndef TABLAHASH_H
#define TABLAHASH_H

#include "Reserva.h"

// Tamano de la tabla: numero primo cercano a 10,000 para que el factor de
// carga (reservas / tamano) quede cerca de 1, como pide el informe.
const int CAPACIDAD_HASH = 10007;

// Nodo de la lista enlazada de cada posicion (igual al de la clase,
// pero guarda un puntero a la reserva en vez de un int).
struct NodoHash {
    Reserva* reserva;
    NodoHash* siguiente;
};

// TABLA HASH CON ENCADENAMIENTO. Clave: idReserva.
// Es la DUENA de las reservas: cuando elimina un nodo tambien libera la
// reserva con delete. El arbol y el heap solo guardan punteros prestados.
class TablaHash {
private:
    NodoHash* tabla[CAPACIDAD_HASH];
    int cantidad;

public:
    TablaHash();
    ~TablaHash();

    int funcionHash(string clave);        // posicion que le toca a un ID

    void insertar(Reserva* r);            // al inicio de la lista de su posicion
    Reserva* buscar(string id);           // nullptr si no existe
    bool eliminar(string id);             // quita el nodo y libera la reserva

    int getCantidad();
    double factorCarga();                 // cantidad / CAPACIDAD_HASH
    int longitudCadena(int posicion);     // nodos que hay en una posicion
    int cadenaMasLarga();
    int posicionesUsadas();               // posiciones con al menos un nodo
};

#endif
