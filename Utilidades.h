#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>
using namespace std;

// Funciones pequenas de texto que usan varios modulos.

// Rellena con espacios (o corta) para que el texto ocupe "ancho" caracteres.
string ajustar(string texto, int ancho);

// Quita espacios y saltos de linea del inicio y del final.
string quitarEspacios(string texto);

// Separa una linea por comas. Retorna cuantos campos encontro.
int separarCSV(string linea, string campos[], int maximo);

// Convierten texto a numero. Retornan false si el texto no es un numero.
bool textoAEntero(string texto, int& valor);
bool textoADecimal(string texto, double& valor);

// 45 con 6 digitos -> "000045"
string numeroConCeros(int numero, int digitos);

#endif
