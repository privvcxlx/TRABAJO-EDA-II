#include "ArbolAVL.h"

ArbolAVL::ArbolAVL() {
    raiz = nullptr;
    cantidad = 0;
    huboCambio = false;
}

ArbolAVL::~ArbolAVL() {
    liberar(raiz);
}

// Libera solo los nodos. Las reservas las libera la tabla hash.
void ArbolAVL::liberar(NodoAVL* x) {
    if (x == nullptr) { return; }
    liberar(x->izq);
    liberar(x->der);
    delete x;
}

// ================= ALTURA Y FACTOR DE BALANCE =================
int ArbolAVL::altura(NodoAVL* x) {
    if (x == nullptr) { return 0; }
    return x->altura;
}

void ArbolAVL::actualizarAltura(NodoAVL* x) {
    int hi = altura(x->izq);
    int hd = altura(x->der);
    if (hi > hd) { x->altura = 1 + hi; }
    else         { x->altura = 1 + hd; }
}

// factor de balance = altura izquierda - altura derecha
//   > 1  -> pesa a la izquierda
//   < -1 -> pesa a la derecha
int ArbolAVL::factor(NodoAVL* x) {
    if (x == nullptr) { return 0; }
    return altura(x->izq) - altura(x->der);
}

// ================= LAS 2 ROTACIONES BASICAS =================
NodoAVL* ArbolAVL::rotarDerecha(NodoAVL* y) {
    NodoAVL* x = y->izq;
    y->izq = x->der;
    x->der = y;
    actualizarAltura(y);
    actualizarAltura(x);
    return x;
}

NodoAVL* ArbolAVL::rotarIzquierda(NodoAVL* x) {
    NodoAVL* y = x->der;
    x->der = y->izq;
    y->izq = x;
    actualizarAltura(x);
    actualizarAltura(y);
    return y;
}

// ================= LOS 4 CASOS DE DESBALANCE =================
NodoAVL* ArbolAVL::casoLL(NodoAVL* x) {
    return rotarDerecha(x);
}

NodoAVL* ArbolAVL::casoRR(NodoAVL* x) {
    return rotarIzquierda(x);
}

NodoAVL* ArbolAVL::casoLR(NodoAVL* x) {
    x->izq = rotarIzquierda(x->izq);
    return rotarDerecha(x);
}

NodoAVL* ArbolAVL::casoRL(NodoAVL* x) {
    x->der = rotarDerecha(x->der);
    return rotarIzquierda(x);
}

NodoAVL* ArbolAVL::balancear(NodoAVL* x) {
    actualizarAltura(x);
    int fb = factor(x);

    if (fb > 1) {                                   // pesa a la izquierda
        if (factor(x->izq) >= 0) { return casoLL(x); }
        else                     { return casoLR(x); }
    }

    if (fb < -1) {                                  // pesa a la derecha
        if (factor(x->der) <= 0) { return casoRR(x); }
        else                     { return casoRL(x); }
    }

    return x;
}

// ================= INSERTAR (recursivo) =================
NodoAVL* ArbolAVL::insertarRec(NodoAVL* x, Reserva* r) {
    if (x == nullptr) {
        huboCambio = true;
        return new NodoAVL(r);
    }
    int comparacion = compararClave(r, x->reserva);
    if (comparacion < 0) {
        x->izq = insertarRec(x->izq, r);
    } else if (comparacion > 0) {
        x->der = insertarRec(x->der, r);
    } else {
        return x;                                   // clave repetida: no se inserta
    }
    return balancear(x);                            // al volver, se rebalancea
}

bool ArbolAVL::insertar(Reserva* r) {
    huboCambio = false;
    raiz = insertarRec(raiz, r);
    if (huboCambio) { cantidad++; }
    return huboCambio;
}

// ================= ELIMINAR (recursivo) =================
NodoAVL* ArbolAVL::minimoNodo(NodoAVL* x) {
    while (x->izq != nullptr) { x = x->izq; }
    return x;
}

NodoAVL* ArbolAVL::eliminarRec(NodoAVL* x, Reserva* r) {
    if (x == nullptr) { return nullptr; }           // no existe

    int comparacion = compararClave(r, x->reserva);
    if (comparacion < 0)      { x->izq = eliminarRec(x->izq, r); }
    else if (comparacion > 0) { x->der = eliminarRec(x->der, r); }
    else {
        if (x->izq == nullptr || x->der == nullptr) {   // 0 o 1 hijo
            NodoAVL* hijo = x->izq;
            if (hijo == nullptr) { hijo = x->der; }
            delete x;                                   // solo el nodo, no la reserva
            huboCambio = true;
            return hijo;
        }
        // 2 hijos: el nodo toma la reserva del sucesor (minimo del subarbol
        // derecho) y luego se elimina el nodo del sucesor.
        NodoAVL* sucesor = minimoNodo(x->der);
        Reserva* reservaSucesor = sucesor->reserva;
        x->reserva = reservaSucesor;
        x->der = eliminarRec(x->der, reservaSucesor);
    }
    return balancear(x);                                // rebalancea en cada nivel
}

bool ArbolAVL::eliminar(Reserva* r) {
    huboCambio = false;
    raiz = eliminarRec(raiz, r);
    if (huboCambio) { cantidad--; }
    return huboCambio;
}

bool ArbolAVL::contiene(Reserva* r) {
    NodoAVL* actual = raiz;
    while (actual != nullptr) {
        int comparacion = compararClave(r, actual->reserva);
        if (comparacion == 0) { return true; }
        if (comparacion < 0)  { actual = actual->izq; }
        else                  { actual = actual->der; }
    }
    return false;
}

// ================= RECORRIDO EN ORDEN =================
// izquierda - nodo - derecha. Se detiene apenas junta "maximo" reservas,
// asi que para mostrar las primeras 20 no recorre las 10,000.
void ArbolAVL::enOrdenRec(NodoAVL* x, Reserva* salida[], int maximo, int& usados) {
    if (x == nullptr || usados >= maximo) { return; }
    enOrdenRec(x->izq, salida, maximo, usados);
    if (usados < maximo) {
        salida[usados] = x->reserva;
        usados++;
    }
    enOrdenRec(x->der, salida, maximo, usados);
}

int ArbolAVL::enOrden(Reserva* salida[], int maximo) {
    int usados = 0;
    enOrdenRec(raiz, salida, maximo, usados);
    return usados;
}

// ================= CONSULTA POR RANGO =================
// La poda esta en los dos if:
//  - Si la fecha del nodo es MENOR que fechaIni, todo su subarbol izquierdo
//    es todavia mas antiguo: no se baja por la izquierda.
//  - Si la fecha del nodo es MAYOR que fechaFin, todo su subarbol derecho
//    es todavia mas nuevo: no se baja por la derecha.
void ArbolAVL::rangoRec(NodoAVL* x, const string& fechaIni, const string& fechaFin,
                        Reserva* muestra[], int maxMuestra, int& encontrados, int& visitados) {
    if (x == nullptr) { return; }
    visitados++;

    if (x->reserva->fecha >= fechaIni) {
        rangoRec(x->izq, fechaIni, fechaFin, muestra, maxMuestra, encontrados, visitados);
    }

    if (x->reserva->fecha >= fechaIni && x->reserva->fecha <= fechaFin) {
        if (encontrados < maxMuestra) { muestra[encontrados] = x->reserva; }
        encontrados++;
    }

    if (x->reserva->fecha <= fechaFin) {
        rangoRec(x->der, fechaIni, fechaFin, muestra, maxMuestra, encontrados, visitados);
    }
}

int ArbolAVL::consultarRango(string fechaIni, string fechaFin, Reserva* muestra[],
                             int maxMuestra, int& visitados) {
    int encontrados = 0;
    visitados = 0;
    rangoRec(raiz, fechaIni, fechaFin, muestra, maxMuestra, encontrados, visitados);
    return encontrados;
}

// ================= DATOS Y VERIFICACION =================
int ArbolAVL::getAltura() {
    return altura(raiz);
}

int ArbolAVL::getCantidad() {
    return cantidad;
}

// Recorre en orden y revisa: (1) que cada clave sea mayor que la anterior,
// (2) que ningun nodo tenga factor de balance fuera de -1, 0, 1,
// (3) que la altura guardada sea la real. Retorna la altura real del subarbol.
int ArbolAVL::verificarRec(NodoAVL* x, Reserva*& anterior, bool& correcto) {
    if (x == nullptr) { return 0; }

    int hi = verificarRec(x->izq, anterior, correcto);

    if (anterior != nullptr && compararClave(anterior, x->reserva) >= 0) { correcto = false; }
    anterior = x->reserva;

    int hd = verificarRec(x->der, anterior, correcto);

    int diferencia = hi - hd;
    if (diferencia > 1 || diferencia < -1) { correcto = false; }

    int real = 1 + (hi > hd ? hi : hd);
    if (real != x->altura) { correcto = false; }
    return real;
}

bool ArbolAVL::esValido() {
    bool correcto = true;
    Reserva* anterior = nullptr;
    verificarRec(raiz, anterior, correcto);
    return correcto;
}
