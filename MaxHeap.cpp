#include "MaxHeap.h"

MaxHeap::MaxHeap() {
    n = 0;
    for (int i = 0; i < CAPACIDAD_HEAP; i++) {
        datos[i] = nullptr;
    }
}

int MaxHeap::padre(int i) {
    return (i - 1) / 2;
}
int MaxHeap::izquierdo(int i) {
    return 2 * i + 1;
}
int MaxHeap::derecho(int i) {
    return 2 * i + 2;
}

// Reemplaza al "heap[a] > heap[b]" del ejemplo con enteros.
bool MaxHeap::mayorPrioridad(Reserva* a, Reserva* b) {
    int categoriaA = valorCategoria(a->categoria);
    int categoriaB = valorCategoria(b->categoria);
    if (categoriaA != categoriaB) { return categoriaA > categoriaB; }
    return a->ordenLlegada < b->ordenLlegada;        // empate: gana el que llego antes
}

void MaxHeap::intercambiar(Reserva* heap[], int i, int j) {
    Reserva* temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

// Sube el elemento de la posicion i mientras tenga mas prioridad que su padre
void MaxHeap::heapifyUp(Reserva* heap[], int i) {
    while (i > 0 && mayorPrioridad(heap[i], heap[padre(i)])) {
        intercambiar(heap, padre(i), i);
        i = padre(i);
    }
}

// Baja el elemento de la posicion i intercambiandolo con su hijo de mayor prioridad
void MaxHeap::heapifyDown(Reserva* heap[], int total, int i) {
    while (true) {
        int mayor = i;
        int izq = izquierdo(i), der = derecho(i);
        if (izq < total && mayorPrioridad(heap[izq], heap[mayor])) { mayor = izq; }
        if (der < total && mayorPrioridad(heap[der], heap[mayor])) { mayor = der; }
        if (mayor == i) { break; }                   // ya cumple la propiedad
        intercambiar(heap, i, mayor);
        i = mayor;
    }
}

// Inserta al final y lo sube.
bool MaxHeap::insertar(Reserva* r) {
    if (n == CAPACIDAD_HEAP) { return false; }
    datos[n] = r;
    n++;
    heapifyUp(datos, n - 1);
    return true;
}

Reserva* MaxHeap::verMax() {
    if (n == 0) { return nullptr; }
    return datos[0];
}

Reserva* MaxHeap::extraerMax() {
    if (n == 0) { return nullptr; }
    Reserva* resultado = datos[0];
    datos[0] = datos[n - 1];                         // el ultimo pasa a la raiz
    n--;
    datos[n] = nullptr;
    if (n > 0) { heapifyDown(datos, n, 0); }
    return resultado;
}

// Secuencial: el enunciado lo permite porque cada lista de espera es pequena.
int MaxHeap::buscarPosicion(string idReserva) {
    for (int i = 0; i < n; i++) {
        if (datos[i]->idReserva == idReserva) { return i; }
    }
    return -1;
}

// ELIMINAR EN UNA POSICION CUALQUIERA:
// el ultimo elemento ocupa el hueco. Ese elemento puede ser mas prioritario
// que su nuevo padre (hay que subirlo) o menos que sus nuevos hijos (hay que
// bajarlo), por eso se decide entre heapifyUp y heapifyDown.
bool MaxHeap::eliminarEn(int posicion) {
    if (posicion < 0 || posicion >= n) { return false; }

    datos[posicion] = datos[n - 1];
    n--;
    datos[n] = nullptr;

    if (posicion < n) {                              // si era el ultimo no hay nada que reordenar
        if (posicion > 0 && mayorPrioridad(datos[posicion], datos[padre(posicion)])) {
            heapifyUp(datos, posicion);
        } else {
            heapifyDown(datos, n, posicion);
        }
    }
    return true;
}

// Copia el arreglo y va sacando el maximo de la COPIA. El heap real no cambia.
int MaxHeap::listarEnOrden(Reserva* salida[]) {
    Reserva* copia[CAPACIDAD_HEAP];
    for (int i = 0; i < n; i++) { copia[i] = datos[i]; }

    int restantes = n;
    int usados = 0;
    while (restantes > 0) {
        salida[usados] = copia[0];
        usados++;
        copia[0] = copia[restantes - 1];
        restantes--;
        if (restantes > 0) { heapifyDown(copia, restantes, 0); }
    }
    return usados;
}

int MaxHeap::tamano() {
    return n;
}

bool MaxHeap::estaVacio() {
    return n == 0;
}

bool MaxHeap::estaLleno() {
    return n == CAPACIDAD_HEAP;
}

bool MaxHeap::esValido() {
    for (int i = 1; i < n; i++) {
        if (mayorPrioridad(datos[i], datos[padre(i)])) { return false; }
    }
    return true;
}
