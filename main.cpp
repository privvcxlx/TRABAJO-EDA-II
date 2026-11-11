// Proyecto EDA II: Sistema de Reservas Aereas con Lista de Espera
//
// Compilar (desde la carpeta del proyecto):   g++ *.cpp -o reservas
// Ejecutar:                                   ./reservas      (Windows: .\reservas.exe)

#include <iostream>
#include "Sistema.h"
#include "Menu.h"
using namespace std;

int main() {
    // los precios y tiempos se muestran con 2 decimales
    cout.setf(ios::fixed);
    cout.precision(2);

    // El sistema es grande (tabla de 10007 posiciones y 60 heaps), por eso
    // se crea con new en vez de dejarlo en la pila.
    Sistema* sistema = new Sistema();

    ejecutarMenu(sistema);

    // El destructor del sistema llama a los destructores de la hash (libera
    // nodos y reservas) y del arbol (libera sus nodos).
    delete sistema;
    return 0;
}
