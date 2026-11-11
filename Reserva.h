#ifndef RESERVA_H
#define RESERVA_H

#include <string>
using namespace std;

// Una reserva se crea UNA sola vez con new. La tabla hash, el arbol AVL y el
// heap guardan punteros a ese mismo objeto: si cambia el estado, las tres
// estructuras ven el cambio sin copiar nada.
struct Reserva {
    string idReserva;
    string dni;
    string pasajero;
    string codigoVuelo;
    string origen;
    string destino;
    string fecha;        // AAAA-MM-DD
    string hora;         // HH:MM
    string clase;        // Economica, Premium, Ejecutiva
    string categoria;    // Premium, Frecuente, Regular
    double precio;
    string estado;       // Confirmada, Pendiente, EnEspera, CheckIn, Cancelada
    int ordenLlegada;    // turno con el que entro a la lista de espera (0 = nunca)

    Reserva() {
        precio = 0;
        ordenLlegada = 0;
    }
};

// Confirmada, Pendiente y CheckIn ocupan asiento. EnEspera y Cancelada no.
bool ocupaAsiento(string estado);

// Premium = 3, Frecuente = 2, Regular = 1 (sirve para la prioridad del heap)
int valorCategoria(string categoria);

bool claseValida(string clase);
bool categoriaValida(string categoria);
bool estadoValido(string estado);
bool fechaValida(string fecha);
bool horaValida(string hora);

// CLAVE COMPUESTA DEL ARBOL: fecha + hora + idReserva.
// Retorna -1 si a va antes que b, 1 si va despues, 0 si son la misma clave.
int compararClave(Reserva* a, Reserva* b);

// Llena una reserva a partir de una linea del .csv. Retorna false si la linea esta mal.
bool leerReservaCSV(string linea, Reserva* r);

void mostrarReserva(Reserva* r);           // todos los datos, uno por linea
void mostrarEncabezadoLista();             // titulos de las columnas
void mostrarReservaLinea(Reserva* r);      // una reserva en una sola linea

#endif
