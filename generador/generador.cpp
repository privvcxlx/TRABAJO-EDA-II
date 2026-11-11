// GENERADOR DE DATOS DE PRUEBA
// Crea los tres archivos que pide el proyecto:
//   vuelos.csv            50 vuelos (capacidad de 150 a 300 asientos)
//   reservas_pequeno.csv  30 reservas, para verificar a mano
//   reservas_masivo.csv   mas de 10,000 reservas
//
// Compilar (desde la carpeta del proyecto):  g++ generador/generador.cpp -o generar
// Ejecutar (desde la carpeta del proyecto):  ./generar     (Windows: .\generar.exe)
//
// Condiciones que cumple el archivo masivo:
//   - 6 vuelos llenos, cada uno con 6 a 12 pasajeros EnEspera de categorias mezcladas.
//   - Muchas reservas con la misma fecha y hora (todas las de un mismo vuelo, y
//     ademas hay vuelos distintos que salen el mismo dia a la misma hora).
//   - Reservas Premium, Frecuente y Regular en todos los vuelos.

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// ================= NUMEROS PSEUDOALEATORIOS =================
// Generador congruencial lineal propio (no usa rand). Con la misma semilla
// siempre produce los mismos archivos, asi las pruebas se pueden repetir.
unsigned long long semilla = 20261201ULL;

int aleatorio(int limite) {                 // numero entre 0 y limite-1
    semilla = semilla * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((semilla >> 33) % (unsigned long long)limite);
}

int aleatorioEntre(int minimo, int maximo) {
    return minimo + aleatorio(maximo - minimo + 1);
}

// ================= DATOS BASE =================
const int NUM_VUELOS = 50;
const int NUM_LLENOS = 6;
const int VUELOS_LLENOS[NUM_LLENOS] = {3, 10, 17, 24, 31, 38};
const int OBJETIVO_MASIVO = 10200;          // reservas del archivo masivo
const int NUM_PEQUENO = 30;
const int MAX_RESERVAS = 20000;

const int NUM_CIUDADES = 12;
string ciudades[NUM_CIUDADES] = {"Cusco", "Arequipa", "Piura", "Iquitos", "Trujillo", "Tarapoto",
                                 "Tacna", "Juliaca", "Pucallpa", "Chiclayo", "Ayacucho", "Cajamarca"};

const int NUM_AEROLINEAS = 4;
string aerolineas[NUM_AEROLINEAS] = {"LA", "AV", "JA", "SK"};

const int NUM_HORAS = 11;
string horas[NUM_HORAS] = {"05:30", "06:45", "08:10", "09:20", "11:05", "13:40",
                           "15:15", "17:50", "19:25", "21:00", "22:35"};

const int NUM_NOMBRES = 30;
string nombres[NUM_NOMBRES] = {"Ana", "Luis", "Maria", "Jose", "Carmen", "Jorge", "Rosa", "Carlos",
                               "Lucia", "Miguel", "Sofia", "Diego", "Valeria", "Pedro", "Camila",
                               "Andres", "Daniela", "Fernando", "Gabriela", "Ricardo", "Paola",
                               "Hugo", "Natalia", "Oscar", "Milagros", "Renzo", "Fiorella",
                               "Alonso", "Ximena", "Bruno"};

const int NUM_APELLIDOS = 30;
string apellidos[NUM_APELLIDOS] = {"Torres", "Ramos", "Soto", "Quispe", "Flores", "Rojas", "Mendoza",
                                   "Vargas", "Castillo", "Huaman", "Garcia", "Chavez", "Diaz",
                                   "Salazar", "Paredes", "Vega", "Campos", "Aguilar", "Medina",
                                   "Cruz", "Reyes", "Mamani", "Espinoza", "Leon", "Cabrera",
                                   "Navarro", "Luna", "Herrera", "Pizarro", "Delgado"};

struct VueloGen {
    string codigo, origen, destino, fecha, hora;
    int capacidad;
};

struct ReservaGen {
    string dni, pasajero, clase, categoria, precio, estado;
    int vuelo;                              // indice del vuelo
};

VueloGen vuelos[NUM_VUELOS];
ReservaGen* lista = nullptr;                // reservas del archivo que se esta armando
int cantidad = 0;

// ================= TEXTO =================
string conCeros(int numero, int digitos) {
    string salida = "";
    for (int i = 0; i < digitos; i++) {
        char digito = '0' + (numero % 10);
        salida = digito + salida;
        numero = numero / 10;
    }
    return salida;
}

// dia 0 = 2026-12-01 ... dia 89 = 2027-02-28
string fechaDelDia(int dia) {
    if (dia < 31) { return "2026-12-" + conCeros(dia + 1, 2); }
    if (dia < 62) { return "2027-01-" + conCeros(dia - 31 + 1, 2); }
    return "2027-02-" + conCeros(dia - 62 + 1, 2);
}

// ================= VUELOS =================
void generarVuelos() {
    int numero = 200;
    for (int i = 0; i < NUM_VUELOS; i++) {
        numero = numero + aleatorioEntre(11, 59);          // siempre crece: no se repite ningun codigo
        vuelos[i].codigo = aerolineas[i % NUM_AEROLINEAS] + to_string(numero);

        string ciudad = ciudades[aleatorio(NUM_CIUDADES)];
        if (aleatorio(10) < 7) { vuelos[i].origen = "Lima"; vuelos[i].destino = ciudad; }
        else                   { vuelos[i].origen = ciudad; vuelos[i].destino = "Lima"; }

        if (i % 5 == 1) {
            // sale el mismo dia y a la misma hora que el vuelo anterior
            vuelos[i].fecha = vuelos[i - 1].fecha;
            vuelos[i].hora  = vuelos[i - 1].hora;
        } else {
            int dia = aleatorio(90);
            if (i == 0) { dia = 0; }                       // asegura vuelos en la primera semana
            if (i == 2) { dia = 3; }
            vuelos[i].fecha = fechaDelDia(dia);
            vuelos[i].hora  = horas[aleatorio(NUM_HORAS)];
        }
        vuelos[i].capacidad = aleatorioEntre(150, 300);
    }

    ofstream salida("vuelos.csv");
    for (int i = 0; i < NUM_VUELOS; i++) {
        salida << vuelos[i].codigo << "," << vuelos[i].origen << "," << vuelos[i].destino << ","
               << vuelos[i].fecha << "," << vuelos[i].hora << "," << vuelos[i].capacidad << "\n";
    }
    salida.close();
}

// ================= RESERVAS =================
string categoriaAleatoria() {
    int n = aleatorio(100);
    if (n < 12) { return "Premium"; }
    if (n < 40) { return "Frecuente"; }
    return "Regular";
}

string estadoConAsiento() {
    int n = aleatorio(100);
    if (n < 65) { return "Confirmada"; }
    if (n < 85) { return "Pendiente"; }
    return "CheckIn";
}

// agrega una reserva a la lista con datos personales al azar
void agregar(int vuelo, string estado, string categoria) {
    ReservaGen& r = lista[cantidad];
    r.vuelo = vuelo;
    r.estado = estado;
    r.categoria = categoria;
    r.dni = to_string(10000000 + aleatorio(80000000));
    r.pasajero = nombres[aleatorio(NUM_NOMBRES)] + " " + apellidos[aleatorio(NUM_APELLIDOS)];

    int n = aleatorio(100);
    int soles;
    if (n < 70)      { r.clase = "Economica"; soles = aleatorioEntre(120, 259); }
    else if (n < 90) { r.clase = "Premium";   soles = aleatorioEntre(260, 399); }
    else             { r.clase = "Ejecutiva"; soles = aleatorioEntre(400, 650); }
    r.precio = to_string(soles) + "." + conCeros(aleatorio(10) * 10, 2);
    cantidad++;
}

// mezcla las posiciones [desde, hasta) de la lista (Fisher-Yates)
void mezclar(int desde, int hasta) {
    for (int i = hasta - 1; i > desde; i--) {
        int j = desde + aleatorio(i - desde + 1);
        ReservaGen temp = lista[i];
        lista[i] = lista[j];
        lista[j] = temp;
    }
}

void escribir(string archivo) {
    ofstream salida(archivo.c_str());
    for (int i = 0; i < cantidad; i++) {
        VueloGen& v = vuelos[lista[i].vuelo];
        salida << "RES-" << conCeros(i + 1, 6) << "," << lista[i].dni << "," << lista[i].pasajero << ","
               << v.codigo << "," << v.origen << "," << v.destino << "," << v.fecha << "," << v.hora << ","
               << lista[i].clase << "," << lista[i].categoria << "," << lista[i].precio << ","
               << lista[i].estado << "\n";
    }
    salida.close();
}

bool esVueloLleno(int vuelo) {
    for (int i = 0; i < NUM_LLENOS; i++) {
        if (VUELOS_LLENOS[i] == vuelo) { return true; }
    }
    return false;
}

void generarMasivo() {
    cantidad = 0;
    int ocupados[NUM_VUELOS];

    // 1) reservas con asiento y canceladas de cada vuelo
    for (int v = 0; v < NUM_VUELOS; v++) {
        if (esVueloLleno(v)) { ocupados[v] = vuelos[v].capacidad; }                       // lleno
        else                 { ocupados[v] = vuelos[v].capacidad * aleatorioEntre(70, 92) / 100; }

        for (int i = 0; i < ocupados[v]; i++) { agregar(v, estadoConAsiento(), categoriaAleatoria()); }

        int canceladas = ocupados[v] * aleatorioEntre(3, 6) / 100;
        for (int i = 0; i < canceladas; i++) { agregar(v, "Cancelada", categoriaAleatoria()); }
    }

    // 2) cuantos pasajeros en espera tendra cada vuelo lleno
    int enEspera[NUM_LLENOS];
    int totalEspera = 0;
    for (int i = 0; i < NUM_LLENOS; i++) {
        enEspera[i] = aleatorioEntre(6, 12);
        totalEspera = totalEspera + enEspera[i];
    }

    // 3) completar hasta el objetivo con mas reservas en vuelos que NO estan
    //    llenos (se dejan al menos 5 asientos libres para probar inserciones)
    int intentos = 0;
    while (cantidad + totalEspera < OBJETIVO_MASIVO) {
        int v = aleatorio(NUM_VUELOS);
        intentos++;
        if (!esVueloLleno(v) && ocupados[v] + 5 < vuelos[v].capacidad) {
            agregar(v, estadoConAsiento(), categoriaAleatoria());
            ocupados[v]++;
        } else if (intentos > 200000) {
            agregar(v, "Cancelada", categoriaAleatoria());      // ya no quedan asientos: se completa con canceladas
        }
    }
    mezclar(0, cantidad);                    // las fechas no quedan ordenadas en el archivo

    // 4) las reservas EnEspera van al final del archivo, porque llegaron
    //    cuando su vuelo ya estaba lleno. Las tres primeras de cada vuelo
    //    tienen una categoria distinta para asegurar la mezcla.
    int inicioEspera = cantidad;
    for (int i = 0; i < NUM_LLENOS; i++) {
        agregar(VUELOS_LLENOS[i], "EnEspera", "Regular");
        agregar(VUELOS_LLENOS[i], "EnEspera", "Premium");
        agregar(VUELOS_LLENOS[i], "EnEspera", "Frecuente");
        for (int k = 3; k < enEspera[i]; k++) { agregar(VUELOS_LLENOS[i], "EnEspera", categoriaAleatoria()); }
    }
    mezclar(inicioEspera, cantidad);

    escribir("reservas_masivo.csv");
    cout << "reservas_masivo.csv : " << cantidad << " reservas (" << cantidad - inicioEspera
         << " EnEspera en " << NUM_LLENOS << " vuelos llenos)" << endl;
}

// Archivo pequeno: 30 reservas en los primeros 6 vuelos. Los vuelos 0 y 1
// salen el mismo dia a la misma hora, asi se puede comprobar a mano el
// desempate por idReserva en el listado cronologico.
void generarPequeno() {
    cantidad = 0;
    for (int i = 0; i < NUM_PEQUENO; i++) {
        string estado = estadoConAsiento();
        if (i % 10 == 9) { estado = "Cancelada"; }
        agregar(i % 6, estado, categoriaAleatoria());
    }
    mezclar(0, cantidad);
    escribir("reservas_pequeno.csv");
    cout << "reservas_pequeno.csv: " << cantidad << " reservas" << endl;
}

int main() {
    lista = new ReservaGen[MAX_RESERVAS];

    generarVuelos();
    cout << "vuelos.csv          : " << NUM_VUELOS << " vuelos" << endl;
    generarPequeno();
    generarMasivo();

    delete[] lista;
    return 0;
}
