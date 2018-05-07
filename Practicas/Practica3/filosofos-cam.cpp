// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos_efectivos = 2 * num_filosofos, //un tenedor por cada filosofo
   num_procesos_esperados = num_procesos_efectivos + 1, //Hay un camarero
   id_camarero = num_procesos_efectivos;


//ETIQUETAS MPI
   const int 
      etiq_levantarse = 1,
      etiq_sentarse = 0;

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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id + 1) % num_procesos_efectivos, //id. tenedor izq.
      id_ten_der = (id + num_procesos_efectivos - 1) % num_procesos_efectivos, //id. tenedor der.
      peticion;

  while ( true )
  {

    //--------------------- 1. SENTARSE ----------------------
    cout << "Filósofo " << id << " solicita permiso para sentarse a la mesa" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD );

    //--------------------- 2. COGER TENEDORES ----------------------
    cout <<"Filósofo " <<id << " solicita tenedor izq. (" <<id_ten_izq << ")" << endl;
    // ... solicitar tenedor izquierdo (completar)
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD );

    cout <<"Filósofo " <<id <<" solicita tenedor der. (" <<id_ten_der<< ")" <<endl;
    // ... solicitar tenedor derecho (completar)
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD );
    
    //--------------------- 3.COMER ----------------------
    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    //--------------------- 4. SOLTAR TENEDORES ----------------------
    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD );

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD );
    
    //--------------------- 5. LEVANTARSE ----------------------    
    cout << "Filósofo " << id << " solicita permiso para levantarse" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD );


    //--------------------- 6. PENSAR ----------------------
    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------


void funcion_camarero()
{
  int        filosofos_sentados = 0,  // número de filósofos sentados en la mesa
             peticion,                // petición recibida
             id_filosofo,             // filósofo que realiza la petición
             etiqueta_aceptable;      // identificador de etiqueta aceptable
  MPI_Status estado;                  // metadatos del mensaje recibido

  while( true )
  {
     // 1. Determinar si atiende peticiones de levantarse o de sentarse y levantarse
     if ( filosofos_sentados < num_filosofos - 1 )  // si hay $num_filosofos-2$ o menos
        etiqueta_aceptable = MPI_ANY_TAG;       // $~~~$ cualquiera
     else                                       // si mesa llena
        etiqueta_aceptable = etiq_levantarse;   // $~~~$ solo levantarse

     // 2. Recibir petición con etiqueta aceptable
     MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiqueta_aceptable, MPI_COMM_WORLD, &estado );
     id_filosofo = estado.MPI_SOURCE;

     // 3. Procesar el mensaje recibido
     switch( estado.MPI_TAG ) // leer etiqueta del mensaje en metadatos
     {
        case etiq_levantarse:
           cout << "\tFilósofo " << id_filosofo << " se levanta de la mesa" << endl;
           filosofos_sentados--;
           break;

        case etiq_sentarse:
           cout << "\tFilósofo " << id_filosofo << " se sienta a la mesa" << endl;
           filosofos_sentados++;
           break;
     }

     cout << "\t --- Actualmente hay " << filosofos_sentados << " filósofos sentados --- " << endl;
  }
}




void funcion_tenedores( int id )
{
  int         valor, 
              id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
    // ...... recibir petición de cualquier filósofo (completar)
    MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );
    // ...... guardar en 'id_filosofo' el id. del emisor (completar)
    id_filosofo = estado.MPI_SOURCE;

    cout <<"Tenedor " <<id <<" ha sido cogido por filosofo " <<id_filosofo <<endl;

    // ...... recibir liberación de filósofo 'id_filosofo' (completar)
    MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
    
    cout <<"Tenedor "<< id << " ha sido liberado por filosofo " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos_esperados == num_procesos_actual )
   {

      // ejecutar la función correspondiente a 'id_propio'
      if(id_propio == id_camarero)
        funcion_camarero();
      else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio );      //   es un filósofo
      else                                    // si es impar
         funcion_tenedores( id_propio );      //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperados << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
