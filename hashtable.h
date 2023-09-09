#include "xerrori.h"
#include <search.h>

// Testa della lista per la tabella hash 


// -------Struct per l'accesso concorrente di lettori e scrittori alla hash table --------------
typedef struct {
  int readersHT; 
	bool writersHT; 
  pthread_cond_t condHT;   
  pthread_mutex_t mutexHT; 
} rwHT;

//-----Struct per la tabella hash-----//

//Struct per la lista di entry
typedef struct {
  int valore;    // numero di occorrenze della stringa 
  ENTRY *next;  
} coppia;



void read_lock(rwHT *z);
void read_unlock(rwHT *z);
void write_lock(rwHT *z);
void write_unlock(rwHT *z);

