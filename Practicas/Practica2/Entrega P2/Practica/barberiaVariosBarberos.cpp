#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <mutex>
#include "HoareMonitor.hpp"

using namespace HM;
using namespace std;



//Variables globales------------------------------------------------------------
const int num_clientes = 7,           // número de clientes
          num_barberos = 1,           // número de barberos
          tamanio_sala = 5;

mutex mtx ;                           // mutex de escritura en pantalla



//Generador de números aleatorios-----------------------------------------------
template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//Funciones espera--------------------------------------------------------------
void esperarFueraBarberia(int i){
  chrono::milliseconds esperar( aleatorio<300,600>() );

  mtx.lock();
  cout << "\t Cliente " << i << ": Creciendole el pelo..." << endl;
  mtx.unlock();

  this_thread::sleep_for( esperar );

  mtx.lock();
  cout << "\t Cliente " << i << ": Me ha crecido el pelo, voy a pelarme" << endl;
  mtx.unlock();
}

void cortarPeloACliente(int i){
  chrono::milliseconds esperar( aleatorio<100,300>() );

  mtx.lock();
  cout << "Barbero "<< i << ": Pelando..." << endl;
  mtx.unlock();

  this_thread::sleep_for( esperar );

  mtx.lock();
  cout << "Barbero "<< i << ": Pelado listo" << endl;
  mtx.unlock();
}


//Monitor para gestionar el acceso a una barbería-------------------------------
class Barberia : public HoareMonitor{
private:
  CondVar clientes, barbero, cliente_pelandose;   //Condiciones
public:
  Barberia();

  void siguienteCliente(int i);
  void cortarPelo(int i);
  void finCliente(int i);
};

//Implementación de los metodos de la barbería----------------------------------
Barberia::Barberia(){
  clientes = newCondVar();
  barbero = newCondVar();
  cliente_pelandose = newCondVar();
}

void Barberia::siguienteCliente(int i){
  if (clientes.empty()) { //Si no hay ningun cliente, el barbero se duerme
    mtx.lock();
    cout << "Barbero "<< i << ": No hay ningun cliente, me duermo zzz..." << endl;
    mtx.unlock();

    barbero.wait(); //Dormir barbero

    mtx.lock();
    cout << "Barbero "<< i << ": Buenos días zzz... Pase pase" << endl;
    mtx.unlock();
  }
  else{
    mtx.lock();
    cout << "Barbero "<< i << ": Que pase el siguiente cliente!" << endl;
    mtx.unlock();
    clientes.signal();
  }                                                          //El barbero avisa al siguiente cliente para que pase
}

void Barberia::cortarPelo(int i) {
  mtx.lock();
  cout << "\t Cliente " << i << ": Buenos dias!" << endl;
  mtx.unlock();

  if (barbero.get_nwt() != 0)
    barbero.signal();                                                         //El cliente despierta al barbero en caso de que este dormido
  else{
    if (clientes.get_nwt() >= tamanio_sala) {
      mtx.lock();
      cout << "\tCliente " << i << ": Hay mucha cola, vuelvo luego!" << '\n';
      mtx.unlock();
      return;
    }
    mtx.lock();
    cout << "\t Cliente " << i << ": Entro a la sala de espera" << endl;         //El cliente espera a que el barberlo le de paso                                                            //El cliente notifica que está esperando
    mtx.unlock();
    clientes.wait();
  }
  mtx.lock();
  cout << "\t Cliente" << i << ": Pelándose..." << endl;
  mtx.unlock();
  cliente_pelandose.wait();                                                   //El cliente espera a que el barbero le pele
  mtx.lock();
  cout << "\t Cliente" << i << ": Perfecto! Hasta luego!" << endl;
  mtx.unlock();
}

void Barberia::finCliente(int i){
  
  mtx.lock();
  cout << "Barbero"<< i << ": Listo, le gusta como ha quedado?" << endl;
  mtx.unlock();

  cliente_pelandose.signal();                                                 //El cliente ha sido pelado y sale de la barbería
}

//Funciones que realizan el trabajo de cliente y barbero------------------------
void hebra_cliente(MRef<Barberia> barberia, int i){
  while (true) {
    barberia->cortarPelo(i);        
    esperarFueraBarberia(i);
  }
}

void hebra_barbero(MRef<Barberia> barberia, int i){
  while (true) {
    barberia->siguienteCliente(i);
    cortarPeloACliente(i);
    barberia->finCliente(i);
  }
}

//Función principal-------------------------------------------------------------
int main(int argc, char const *argv[]) {

  mtx.lock();
  cout << "------------------------" << endl
       << "Problema de la barberia." << endl
       << "------------------------" << endl;
  mtx.unlock();


  MRef<Barberia> barberia = Create<Barberia>();


  thread barberos[num_barberos];
  thread clientes[num_clientes];

  for (size_t i = 0; i < num_barberos; i++) {
    barberos[i] = thread(hebra_barbero, barberia, i);
  }
  for (size_t i = 0; i < num_clientes; i++) {
    clientes[i] = thread(hebra_cliente, barberia, i);
  }

  for (size_t i = 0; i < num_barberos; i++) {
    barberos[i].join();
  }
  for (size_t i = 0; i < num_clientes; i++) {
    clientes[i].join();
  }
  return 0;
}
