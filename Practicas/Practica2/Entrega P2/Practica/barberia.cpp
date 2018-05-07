#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

const int num_clientes = 3; // número de clientes


//**********************************************************************
// plantilla de función para generar un entero aleatorio 
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
    return distribucion_uniforme( generador );
}

// *****************************************************************************
// Clase para monitor Barberia, semántica SU

class Barberia : public HoareMonitor
{
    private:
        CondVar                         // colas condición:
            barbero,                    // cola donde espera el barbero
            silla,                      // cola de la silla en la que se pela a los clientes
            clientes;                // cola donde esperan los clientes

    public:                                 // constructor y métodos públicos
        Barberia();                         // constructor
        void cortarPelo(int num_cliente);   // cortar el pelo (cliente)
        void siguienteCliente();            // llamar al siguiente cliente (barbero)
        void finCliente();                  // avisar al cliente de que ya ha terminado (barbero)
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia(  )
{
    barbero = newCondVar();
    clientes = newCondVar();
    silla = newCondVar();
}
// -----------------------------------------------------------------------------

// Función llamada por el cliente para que el barbero le corte el pelo
void Barberia::cortarPelo(int num_cliente)
{
     cout << "Buenos dias, soy el cliente " << num_cliente << ", venia a pelarme. " << endl;
    // El cliente espera a que el barbero no esté ocupado
    if (!silla.empty()){ //hay cliente pelandose
        cout << "El cliente " << num_cliente << " espera. " << endl;
        clientes.wait();//pasa a la cola de clientes
    }

    // El cliente despierta al barbero
    barbero.signal();

    cout << "El cliente " << num_cliente << " entra. " << endl;

    silla.wait(); //Cliente se pela 

    cout<< "Cliente " << num_cliente << ", ¡Gracias!, me gusta el resultado.\n" << endl;
}
// -----------------------------------------------------------------------------
// Función llamada por el barbero para avisar al siguiente cliente

void Barberia::siguienteCliente()
{
    if (clientes.empty() && silla.empty()){
        cout << "\tNo hay ningun cliente, me duermo... zzzz" << endl;
        
        barbero.wait(); //dormir barbero
 
        cout << "\tBuenos dias, adelante, sientase como en casa." << endl;

    }
    else{

        cout << "\tQue pase el siguiente, por favor. \n ---------  Hay " << clientes.get_nwt() << " clientes en espera.  --------- \n " << endl;

        clientes.signal();
    }   
   
}
// -----------------------------------------------------------------------------
// Función llamada por el barbero para avisar al cliente de que ha terminado de pelarlo

void Barberia::finCliente(){
    // Avisa al cliente de que ha terminado de pelarlo
    cout << "\t¡Listo!, he acabado. ¿Le gusta?" << endl;

    silla.signal(); //llama a otro cliente
}

// *****************************************************************************

// Función que simula el corte de pelo, con una espera aleatoria

void cortarPeloACliente()
{
    // calcular milisegundos aleatorios de duración de la acción de pelar)
    chrono::milliseconds duracion_pelar( aleatorio<150,500>() );
    cout << "\n\t**El barbero está pelando.**\n " << endl;
    this_thread::sleep_for(duracion_pelar);
}

//-------------------------------------------------------------------------
// Función que simula la acción de esperar fuera de la barbería

void esperarFueraBarberia( int num_cliente)
{

    // calcular milisegundos aleatorios de duración de la acción de la espera)
    chrono::milliseconds duracion_espera( aleatorio<400,700>() );

    // espera bloqueada un tiempo igual a ''duracion_espera' milisegundos
    this_thread::sleep_for(duracion_espera);

    // informa de que ha terminado de esperar
    cout << "El cliente " << num_cliente << " : termina de esperar." << endl;
}

//----------------------------------------------------------------------

// Función que ejecuta la hebra del barbero

void funcion_hebra_barbero( MRef<Barberia> monitor )
{
    while(true){
        monitor->siguienteCliente();
        cortarPeloACliente();
        monitor->finCliente();
    }

}

//-------------------------------------------------------------------------

// Función que ejecuta la hebra del cliente
void funcion_hebra_cliente(MRef<Barberia> monitor, int num_cliente)
{
    while(true){
        monitor->cortarPelo(num_cliente);
        esperarFueraBarberia(num_cliente);
    }
}

//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------" << endl
    << "Problema de la barbería con monitores SU." << endl
    << "--------------------------------------------------------" << endl
    << flush ;

    MRef<Barberia> monitor = Create<Barberia>();

    thread hebra_barbero;
    thread hebra_cliente[num_clientes];

    hebra_barbero = thread(funcion_hebra_barbero,monitor);
    for (int i=0; i < num_clientes; i++)
        hebra_cliente[i] = thread(funcion_hebra_cliente,monitor,i);
  
    hebra_barbero.join() ;
    for (int i=0; i < num_clientes; i++)
        hebra_cliente[i].join();

    cout << "Fin" << endl;
}