#include "xerrori.h"
#include <search.h>
#define Num_elem 1000000 


//struttura dei dati da memorizzare nella tabella hash
typedef struct coppia{
  int valore;
  ENTRY *next;
} coppia;

typedef struct hashtable {
  int fd;
  pthread_cond_t  *cv_ht; // condition variable x ht
  pthread_mutex_t *mutex_ht; // mutex x ht
} hashtable;

ENTRY *crea_entry(char *s);
void distruggi_entry(ENTRY *entry);
void aggiungi(char *s);
int conta(char *s);
void print_hashtable();