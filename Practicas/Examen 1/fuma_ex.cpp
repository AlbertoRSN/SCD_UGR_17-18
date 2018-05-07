#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


//Ingredientes en orden: papel - tabaco - cerillas
Semaphore ingr_disp[3] = {0, 0, 0};
Semaphore mostr_vacio = 1;

//Garantizar exclusion mutua a la hora de hacer cout
mutex mtx;

//Variable cigarros para que cada hebra muestre que cigarro fumará.
int cigarros=0;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
	int i, contador=0, anterior, num;
	bool primera_vez=true;

	while(true){
		i = aleatorio<0,2>(); //Produce entero aleatorio con retraso;
		
		sem_wait(mostr_vacio);
		
		cout<<"\nPuesto ingrediente: " << i << endl;		

		anterior = i;
		
		if(i==3 && primera_vez){
			aleatorio<2,3>();
			sem_signal(ingr_disp[num]);
			primera_vez=false;		
		}
	
		sem_signal(ingr_disp[i]);
			
		mtx.lock();
		if(i==anterior || contador == 3)
			contador++;
			if(contador==3)
				cout<<"\n\n\nEl estanquero ha colocado tres veces el ingrediente "<< i <<" \n" <<endl;
		i=-1;	
		contador=0;	
		mtx.unlock();
			
		
		
		
		
	}

}
//-------------------------------------------------------------------------

// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "\nFumador " << num_fumador << ":"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "\nFumador " << num_fumador << ": termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true ){
   		sem_wait(ingr_disp[num_fumador]);

   		cout<<"\nRetirado ingrediente: " << num_fumador << "\n";
		
   		sem_signal(mostr_vacio);
		

		cigarros+=1;		

		if(cigarros%7==0){
			cout<<"\n \n El fumador " <<num_fumador<< " va a fumar el cigarro numero " << cigarros <<". \n"<<endl;
		}
	
		
   		fumar(num_fumador);
		
   }
}

//----------------------------------------------------------------------



int main()
{

	//modificacion examen -4 fumadores-
	const int num_hebras = 4;
	thread hebra_fumador[4];

	thread hebra_estanquero (funcion_hebra_estanquero);

	for (int i=0; i<num_hebras; i++){
		hebra_fumador[i] = thread (funcion_hebra_fumador, i);
	}

   	for(int i=0; i<num_hebras; i++){
   		hebra_fumador[i].join();
   	}

   hebra_estanquero.join();
   

}
