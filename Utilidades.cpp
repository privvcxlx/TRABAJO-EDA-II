#include "Utilidades.h"
#include <sstream>

string ajustar(string texto, int ancho) {
    string salida = "";
    for (int i = 0; i < ancho; i++) {
        if (i < (int)texto.length()) { salida += texto[i]; }
        else                         { salida += ' '; }
    }
    return salida;
}

string quitarEspacios(string texto) {
    int inicio = 0;
    int fin = (int)texto.length() - 1;
    while (inicio <= fin && (texto[inicio] == ' ' || texto[inicio] == '\t' ||
                             texto[inicio] == '\r' || texto[inicio] == '\n')) {
        inicio++;
    }
    while (fin >= inicio && (texto[fin] == ' ' || texto[fin] == '\t' ||
                             texto[fin] == '\r' || texto[fin] == '\n')) {
        fin--;
    }
    string salida = "";
    for (int i = inicio; i <= fin; i++) { salida += texto[i]; }
    return salida;
}

int separarCSV(string linea, string campos[], int maximo) {
    int cantidad = 0;
    string actual = "";
    for (int i = 0; i <= (int)linea.length(); i++) {
        // al llegar a una coma (o al final) se cierra el campo actual
        if (i == (int)linea.length() || linea[i] == ',') {
            if (cantidad < maximo) { campos[cantidad] = quitarEspacios(actual); }
            cantidad++;
            actual = "";
        } else {
            actual += linea[i];
        }
    }
    return cantidad;
}

bool textoAEntero(string texto, int& valor) {
    stringstream flujo(quitarEspacios(texto));
    char sobra;
    if (!(flujo >> valor)) { return false; }     // no empieza con un numero
    if (flujo >> sobra)    { return false; }     // quedo texto despues del numero
    return true;
}

bool textoADecimal(string texto, double& valor) {
    stringstream flujo(quitarEspacios(texto));
    char sobra;
    if (!(flujo >> valor)) { return false; }
    if (flujo >> sobra)    { return false; }
    return true;
}

string numeroConCeros(int numero, int digitos) {
    string salida = "";
    for (int i = 0; i < digitos; i++) {
        char digito = '0' + (numero % 10);
        salida = digito + salida;
        numero = numero / 10;
    }
    return salida;
}
