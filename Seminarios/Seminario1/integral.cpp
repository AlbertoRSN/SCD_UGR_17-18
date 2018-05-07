#include <iostream>
#include <pthread.h>
#include <chrono>
using namespace std ;
//compilar con: g++ -o pi pi.cpp  -lpthread  -std=gnu++0x
//el switch -lpthreads sirve para incluir las funciones de hebras POSIX
//el switch -std=gnu++0x sirve para incluir las funciones chronos

unsigned long m=999999; //muestas
unsigned long n=4; //hebras
double resultado_parcial[4];
//double suma=0;

double f(double x) {
    return 4.0/(1+x*x);
}

double calcular_integral_secuencial() {//calculo de la integral de forma secuencial.
    double suma=0.0;
    for(unsigned long i=0; i<m; i++) suma +=f((i+0.5)/m);
    return suma/m;
}

void* funcion_concurrente(void* ih_void) {//calcule los resultados parcuales dependiendo de n(identificador de la hebra).
    int ih=(unsigned long) ih_void;
    double sumap=0;
    for(unsigned long i=ih; i<m; i+=n) {
        sumap +=f((i+0.5)/m);
    }
    resultado_parcial[ih]=sumap;
}

double calcular_integral_concurrente() {//calculo de la integral de forma concurrente.
    pthread_t id_hebras[n];//declaramos un vector de hebras
    double resultado=0;
    for(unsigned i=0; i<n; i++) {//creamos las hebras
        pthread_create(&(id_hebras[i]),NULL,funcion_concurrente,(void*) i);
    }

    for(unsigned i=0; i<n; i++) {//lanzamos las hebras
        pthread_join(id_hebras[i],NULL);
    }
    for(int i=0; i<n; i++) resultado+=resultado_parcial[i];//sumamos los diferentes resultados
    return resultado/m;
}



int main() {
    //variables para el tiempo
    std::chrono::high_resolution_clock::time_point tantes, tdespues;
    std::chrono::duration<double> transcurrido;

    cout.precision(20);
    tantes = std::chrono::high_resolution_clock::now(); //coger tiempo antes de la operación
    cout<<"Calculo secuencial:"<<calcular_integral_secuencial()<<endl;
    tdespues = std::chrono::high_resolution_clock::now();//coger tiempo después de la operación
    transcurrido = std::chrono::duration_cast<std::chrono::duration<double> >(tdespues - tantes);
    cout <<  "Tiempo secuencial: "<<transcurrido.count() << "\n";
    tantes = std::chrono::high_resolution_clock::now(); //coger tiempo antes de la operación
    cout<<"Calculo concurrente:"<<calcular_integral_concurrente()<<endl;
    tdespues = std::chrono::high_resolution_clock::now();//coger tiempo después de la operación
    transcurrido = std::chrono::duration_cast<std::chrono::duration<double> >(tdespues - tantes);
    cout << "Tiempo concurrente: "<<transcurrido.count() << "\n";
    return 0;
}
