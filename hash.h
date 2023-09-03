#include "xerrori.h"
#include <search.h>



//struttura dei dati da memorizzare nella tabella hash
typedef struct{
  int valore;
  ENTRY *next;
} coppia;


ENTRY *crea_entry(char *s);
void distruggi_entry(ENTRY *entry);
void aggiungi(char *s);
int conta(char *s);
