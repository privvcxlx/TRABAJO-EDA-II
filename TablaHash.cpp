#include "TablaHash.h"

TablaHash::TablaHash() {
    for (int i = 0; i < CAPACIDAD_HASH; i++) {
        tabla[i] = nullptr;
    }
    cantidad = 0;
}

// Libera todos los nodos y, como la tabla es la duena, tambien las reservas.
TablaHash::~TablaHash() {
    for (int i = 0; i < CAPACIDAD_HASH; i++) {
        NodoHash* actual = tabla[i];
        while (actual != nullptr) {
            NodoHash* siguiente = actual->siguiente;
            delete actual->reserva;
            delete actual;
            actual = siguiente;
        }
        tabla[i] = nullptr;
    }
}

// FUNCION HASH: une las dos que vimos en clase.
//  - Las letras se SUMAN, como en hash_nombre (acum = acum + k[i]).
//  - Los digitos forman el numero del ID, y a eso se le aplica el modulo,
//    como en la tabla de enteros (valor % CAPACIDAD).
// Si solo sumaramos caracteres, "RES-000123" y "RES-000321" caerian en la
// misma posicion y los 10,000 IDs se repartirian en apenas unas 50 posiciones.
// Con el numero completo, IDs consecutivos caen en posiciones consecutivas.
int TablaHash::funcionHash(string clave) {
    long long letras = 0;
    long long numero = 0;
    for (int i = 0; i < (int)clave.length(); i++) {
        char c = clave[i];
        if (c >= '0' && c <= '9') {
            numero = (numero * 10 + (c - '0')) % CAPACIDAD_HASH;
        } else {
            letras = letras + c;
        }
    }
    int salida = (int)((letras + numero) % CAPACIDAD_HASH);
    if (salida < 0) { salida = salida + CAPACIDAD_HASH; }   // por si hay caracteres negativos (tildes)
    return salida;
}

// No revisa repetidos: el sistema ya llamo a buscar() antes de insertar.
void TablaHash::insertar(Reserva* r) {
    int idx = funcionHash(r->idReserva);

    NodoHash* nuevo = new NodoHash();
    nuevo->reserva = r;
    nuevo->siguiente = tabla[idx];        // se agrega al inicio de la lista
    tabla[idx] = nuevo;
    cantidad++;
}

Reserva* TablaHash::buscar(string id) {
    int idx = funcionHash(id);
    NodoHash* actual = tabla[idx];

    while (actual != nullptr) {
        if (actual->reserva->idReserva == id) { return actual->reserva; }
        actual = actual->siguiente;
    }
    return nullptr;
}

bool TablaHash::eliminar(string id) {
    int idx = funcionHash(id);
    NodoHash* actual = tabla[idx];
    NodoHash* anterior = nullptr;

    while (actual != nullptr) {
        if (actual->reserva->idReserva == id) {
            if (anterior == nullptr) { tabla[idx] = actual->siguiente; }        // era el primero
            else                     { anterior->siguiente = actual->siguiente; }
            delete actual->reserva;
            delete actual;
            cantidad--;
            return true;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    return false;
}

int TablaHash::getCantidad() {
    return cantidad;
}

double TablaHash::factorCarga() {
    return (double)cantidad / CAPACIDAD_HASH;
}

int TablaHash::longitudCadena(int posicion) {
    int largo = 0;
    NodoHash* actual = tabla[posicion];
    while (actual != nullptr) {
        largo++;
        actual = actual->siguiente;
    }
    return largo;
}

int TablaHash::cadenaMasLarga() {
    int mayor = 0;
    for (int i = 0; i < CAPACIDAD_HASH; i++) {
        int largo = longitudCadena(i);
        if (largo > mayor) { mayor = largo; }
    }
    return mayor;
}

int TablaHash::posicionesUsadas() {
    int usadas = 0;
    for (int i = 0; i < CAPACIDAD_HASH; i++) {
        if (tabla[i] != nullptr) { usadas++; }
    }
    return usadas;
}
