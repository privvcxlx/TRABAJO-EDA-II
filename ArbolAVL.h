#ifndef ARBOLAVL_H
#define ARBOLAVL_H

#include "Reserva.h"

struct NodoAVL {
    Reserva* reserva;    // puntero prestado: la reserva es de la tabla hash
    int altura;          // altura de una hoja = 1
    NodoAVL* izq;
    NodoAVL* der;

    NodoAVL(Reserva* r) {
        reserva = r;
        altura = 1;
        izq = nullptr;
        der = nullptr;
    }
};

// ARBOL AVL. Clave compuesta: fecha + hora + idReserva (ver compararClave).
// Mantiene las reservas ordenadas por fecha y hora de salida.
class ArbolAVL {
private:
    NodoAVL* raiz;
    int cantidad;
    bool huboCambio;     // true si la ultima insercion/eliminacion modifico el arbol

    // altura y factor de balance
    int altura(NodoAVL* x);
    void actualizarAltura(NodoAVL* x);
    int factor(NodoAVL* x);

    // las 2 rotaciones basicas
    NodoAVL* rotarDerecha(NodoAVL* y);
    NodoAVL* rotarIzquierda(NodoAVL* x);

    // los 4 casos de desbalance
    NodoAVL* casoLL(NodoAVL* x);
    NodoAVL* casoRR(NodoAVL* x);
    NodoAVL* casoLR(NodoAVL* x);
    NodoAVL* casoRL(NodoAVL* x);
    NodoAVL* balancear(NodoAVL* x);

    NodoAVL* insertarRec(NodoAVL* x, Reserva* r);
    NodoAVL* minimoNodo(NodoAVL* x);
    NodoAVL* eliminarRec(NodoAVL* x, Reserva* r);

    void enOrdenRec(NodoAVL* x, Reserva* salida[], int maximo, int& usados);
    void rangoRec(NodoAVL* x, const string& fechaIni, const string& fechaFin,
                  Reserva* muestra[], int maxMuestra, int& encontrados, int& visitados);
    int verificarRec(NodoAVL* x, Reserva*& anterior, bool& correcto);
    void liberar(NodoAVL* x);

public:
    ArbolAVL();
    ~ArbolAVL();

    bool insertar(Reserva* r);       // false si la clave ya estaba
    bool eliminar(Reserva* r);       // false si no estaba
    bool contiene(Reserva* r);

    // Recorrido en orden: copia en salida[] las primeras "maximo" reservas
    // (las de fecha y hora mas temprana). Retorna cuantas copio.
    int enOrden(Reserva* salida[], int maximo);

    // Consulta por rango de fechas (incluye ambos extremos). Retorna cuantas
    // reservas hay en el rango y copia hasta maxMuestra en muestra[].
    // "visitados" devuelve cuantos nodos toco: sirve para comprobar que
    // descarta ramas y no recorre todo el arbol.
    int consultarRango(string fechaIni, string fechaFin, Reserva* muestra[],
                       int maxMuestra, int& visitados);

    int getAltura();
    int getCantidad();
    bool esValido();                 // revisa el orden y el balance de todos los nodos
};

#endif
