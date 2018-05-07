#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp" //Monitores SU

using namespace std ;
using namespace HM;

//Clase Monitor
class Estanco : public HoareMonitor{
private:
  int ing_colocados;      //Variable permanente para saber si mostrador esta vacio

  //Variables condicion
  CondVar fumadores[3]; //Cola de espera de fumadores -> Array de variables condicion para fumadores. 
  CondVar estanquero; //cola mostrador-ingredientes (Estanquero)


public:

  //Constructor
  Estanco(); 

  //Procedimientos
  void obtenerIngrediente(int i);
  void ponerIngrediente(int i);
  void esperarRecogidaIngrediente();

};

Estanco::Estanco(){
  //En el constructor incializar cada variable condicion con newCondVar();
  ing_colocados = -1;
  for(int i =0; i<3; i++)
    fumadores[i] = newCondVar();
  estanquero = newCondVar();
}


//PROCEDIMIENTOS DE LA CLASE ESTANCOI (MONITOR)
void Estanco::ponerIngrediente(int i){
  ing_colocados = i;
  fumadores[i].signal();
}

void Estanco::obtenerIngrediente(int i){ //Donde i es el numero de fumador/ingrediente que esperan
  if(ing_colocados != i)
    fumadores[i].wait();
  ing_colocados = -1;
  estanquero.signal();
}

void Estanco::esperarRecogidaIngrediente(){
  if(ing_colocados != -1)
    estanquero.wait();
}

//----------------------------------------------------------------------
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------

//Funcion para producir ingrediente aleatorio
int ProducirIngrediente(){
  return aleatorio<0,2>(); //Produce entero aleatorio
}


// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero(MRef<Estanco> monitor){

  int ingre;

	while(true){		
    ingre = ProducirIngrediente(); //Produce ingrediente aleatorio 0,1,2

    const int ms = aleatorio<0,30>();
    this_thread::sleep_for(chrono::milliseconds(ms));

    monitor->ponerIngrediente(ingre);
    monitor->esperarRecogidaIngrediente();

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
void  funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador )
{
   while( true ){
      monitor->obtenerIngrediente(num_fumador);      
   		cout<<"\nRetirado ingrediente: " << num_fumador << "\n";
   		fumar(num_fumador);
   }
}

//----------------------------------------------------------------------



int main()
{

	const int num_hebras = 3;

  //Crear Monitor
  MRef<Estanco> monitor = Create<Estanco>();

	thread hebra_fumador[num_hebras];

	thread hebra_estanquero(funcion_hebra_estanquero, monitor);

	for (int i=0; i<num_hebras; i++){
		hebra_fumador[i] = thread (funcion_hebra_fumador, monitor, i);
	}

   for(int i=0; i<num_hebras; i++){
   		hebra_fumador[i].join();
   }

   hebra_estanquero.join();
   

}
