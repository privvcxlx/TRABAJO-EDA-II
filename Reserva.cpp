#include "Reserva.h"
#include "Utilidades.h"
#include <iostream>

bool ocupaAsiento(string estado) {
    return estado == "Confirmada" || estado == "Pendiente" || estado == "CheckIn";
}

int valorCategoria(string categoria) {
    if (categoria == "Premium")   { return 3; }
    if (categoria == "Frecuente") { return 2; }
    return 1;                                       // Regular
}

bool claseValida(string clase) {
    return clase == "Economica" || clase == "Premium" || clase == "Ejecutiva";
}

bool categoriaValida(string categoria) {
    return categoria == "Premium" || categoria == "Frecuente" || categoria == "Regular";
}

bool estadoValido(string estado) {
    return estado == "Confirmada" || estado == "Pendiente" || estado == "EnEspera" ||
           estado == "CheckIn" || estado == "Cancelada";
}

static bool esDigito(char c) {
    return c >= '0' && c <= '9';
}

// formato AAAA-MM-DD
bool fechaValida(string fecha) {
    if (fecha.length() != 10) { return false; }
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (fecha[i] != '-') { return false; }
        } else if (!esDigito(fecha[i])) {
            return false;
        }
    }
    int mes = (fecha[5] - '0') * 10 + (fecha[6] - '0');
    int dia = (fecha[8] - '0') * 10 + (fecha[9] - '0');
    return mes >= 1 && mes <= 12 && dia >= 1 && dia <= 31;
}

// formato HH:MM
bool horaValida(string hora) {
    if (hora.length() != 5 || hora[2] != ':') { return false; }
    if (!esDigito(hora[0]) || !esDigito(hora[1]) || !esDigito(hora[3]) || !esDigito(hora[4])) {
        return false;
    }
    int h = (hora[0] - '0') * 10 + (hora[1] - '0');
    int m = (hora[3] - '0') * 10 + (hora[4] - '0');
    return h <= 23 && m <= 59;
}

// Como la fecha es AAAA-MM-DD y la hora HH:MM, comparar los textos con < y >
// da el mismo orden que el calendario. El idReserva al final desempata cuando
// dos reservas salen el mismo dia a la misma hora, asi la clave nunca se repite.
int compararClave(Reserva* a, Reserva* b) {
    if (a->fecha < b->fecha) { return -1; }
    if (a->fecha > b->fecha) { return 1; }
    if (a->hora < b->hora)   { return -1; }
    if (a->hora > b->hora)   { return 1; }
    if (a->idReserva < b->idReserva) { return -1; }
    if (a->idReserva > b->idReserva) { return 1; }
    return 0;
}

bool leerReservaCSV(string linea, Reserva* r) {
    string campos[12];
    int cantidad = separarCSV(linea, campos, 12);
    if (cantidad != 12) { return false; }

    r->idReserva   = campos[0];
    r->dni         = campos[1];
    r->pasajero    = campos[2];
    r->codigoVuelo = campos[3];
    r->origen      = campos[4];
    r->destino     = campos[5];
    r->fecha       = campos[6];
    r->hora        = campos[7];
    r->clase       = campos[8];
    r->categoria   = campos[9];
    r->estado      = campos[11];

    if (r->idReserva == "")                  { return false; }
    if (!textoADecimal(campos[10], r->precio)) { return false; }
    if (!fechaValida(r->fecha))              { return false; }
    if (!horaValida(r->hora))                { return false; }
    if (!claseValida(r->clase))              { return false; }
    if (!categoriaValida(r->categoria))      { return false; }
    if (!estadoValido(r->estado))            { return false; }
    return true;
}

void mostrarReserva(Reserva* r) {
    cout << "  ID reserva : " << r->idReserva << endl;
    cout << "  DNI        : " << r->dni << endl;
    cout << "  Pasajero   : " << r->pasajero << endl;
    cout << "  Vuelo      : " << r->codigoVuelo << "  (" << r->origen << " -> " << r->destino << ")" << endl;
    cout << "  Salida     : " << r->fecha << " " << r->hora << endl;
    cout << "  Clase      : " << r->clase << endl;
    cout << "  Categoria  : " << r->categoria << endl;
    cout << "  Precio     : " << r->precio << endl;
    cout << "  Estado     : " << r->estado << endl;
}

void mostrarEncabezadoLista() {
    cout << ajustar("ID", 12) << ajustar("Fecha", 11) << ajustar("Hora", 6)
         << ajustar("Vuelo", 8) << ajustar("Destino", 11) << ajustar("Pasajero", 21)
         << ajustar("Clase", 10) << ajustar("Categoria", 10) << ajustar("Estado", 11)
         << "Precio" << endl;
    cout << "------------------------------------------------------------"
         << "----------------------------------------------" << endl;
}

void mostrarReservaLinea(Reserva* r) {
    cout << ajustar(r->idReserva, 12) << ajustar(r->fecha, 11) << ajustar(r->hora, 6)
         << ajustar(r->codigoVuelo, 8) << ajustar(r->destino, 11) << ajustar(r->pasajero, 21)
         << ajustar(r->clase, 10) << ajustar(r->categoria, 10) << ajustar(r->estado, 11)
         << r->precio << endl;
}
