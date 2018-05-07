#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp" //Monitores SU

using namespace std ;
using namespace HM;

const int n_clientes = 5;

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void EsperaAleatoria(){
  // calcular milisegundos aleatorios de duración de la espera
  const int ms = aleatorio<10,200>();
  // espera bloqueada un tiempo igual a 'espera' milisegundos
  this_thread::sleep_for(chrono::milliseconds(ms));
}


void EsperarFueraBarberia(){
  EsperaAleatoria();
}
void CortarPeloACliente(){
  EsperaAleatoria();
}


//------------------MONITOR BARBERIA--------------------
class Barberia : public HoareMonitor{
private:
  CondVar clientes, barbero, silla;
  bool silla_libre;

public:

  //Constructor
  Barberia(); 
  //Procedimientos
  void CortarPelo(int i);
  void SiguienteCliente();
  void FinCliente();

};

//Constructor
Barberia::Barberia(){
  silla_libre = true;
  clientes = newCondVar();
  barbero = newCondVar();
  silla = newCondVar(); 
}

void Barberia::CortarPelo(int i){
  cout<< "El cliente " << i << " ha entrado."<<endl;

  // Cuando el cliente entra y hay alguien sentado o algún cliente en
  // la cola esperando este tiene que ponerse a la cola
  if(!clientes.empty() || !silla_libre){
    clientes.wait();
  }
  // Una vez sale de la cola es porque la silla se ha quedado libre,
  // entonces se sienta y avisa al barbero de que está listo.
  silla_libre = false;
  barbero.signal();

  cout << "El cliente " << i << " ha avisado al barbero y ha ocupado la silla" << endl;

  silla.wait();

  // Cuando el barbero le haya avisado el cliente podrá irse y desocupará la silla
  cout << "El cliente " << i << " ha dejado la silla libre" << endl;
}


void Barberia::SiguienteCliente(){
  // Si no hay clientes en la cola se duerme el barbero y en caso contrario, sigue con el siguiente cliente.
  if(clientes.empty()){
    cout << "\tNo hay nadie esperando, el barbero se duerme." << endl;
    barbero.wait();
  } else
    clientes.signal();

  cout << "\tEl barbero recibe al siguiente cliente." << endl;
}

void Barberia::FinCliente(){
  cout << "\tEl barbero termina de pelar al cliente." << endl;
  silla_libre = true;
  silla.signal();
}




// Funciones Hebras:

void funcion_hebra_barbero(MRef<Barberia> monitor){
  while(true){
    monitor->SiguienteCliente();
    CortarPeloACliente();
    monitor->FinCliente();
  }
}

void funcion_hebra_cliente( MRef<Barberia> monitor, int cliente ){
   while( true ){
     monitor->CortarPelo(cliente);
     cout<<"\nCliente se sienta a pelarse: " << cliente << "\n";
     EsperarFueraBarberia();
   }
}



int main()
{

   MRef<Barberia> monitor = Create<Barberia>();

  thread clientes[n_clientes]; //Creacion hebras clientes
  thread barbero (funcion_hebra_barbero,monitor); //Crea hebra barbero

  for(int i = 0; i < n_clientes; i++)
    clientes[i] = thread(funcion_hebra_cliente,monitor, i);

  for(int i = 0; i < n_clientes; i++)
    clientes[i].join();

  barbero.join();
}


