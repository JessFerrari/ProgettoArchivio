#include "xerrori.h"
#include <search.h>
#include <stdatomic.h>

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

//funzioni per la creazione e distruzione della hash table
ENTRY *crea_entry(char *s, int n);
void distruggi_entry(ENTRY *e);
void distruggi_hash();
void clear_hash();

//funzioni usate dai thread scrittori e dai thread lettori
void aggiungi (char *s);
int conta(char *s);

//funzioni per la stampa 
void stampa_entry(ENTRY *e);
void stampa_lista_entry();
int numero_stringhe();