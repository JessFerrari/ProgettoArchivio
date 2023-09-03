/*Libreria per la costruzione di una tabella hash e le funzioni per le operazioni*/

#include "hash.h"


ENTRY *testa_lista_entry = NULL;

//creo un oggetto della hash table
ENTRY *crea_entry(char *s){
  ENTRY *e = malloc(sizeof(ENTRY));
  if(e==NULL) xtermina("errore malloc entry 1",__LINE__, __FILE__);
  e->key = strdup(s); // salva copia di s
  e->data = malloc(sizeof(coppia));
  if(e->key==NULL || e->data==NULL)
    xtermina("errore malloc entry 2", __LINE__, __FILE__);
  // inizializzo coppia
  coppia *c = (coppia *) e->data;
  c->valore = 1;
  c->next = NULL;
  return e;
}

//distruggo oggetto della hash table
void distruggi_entry(ENTRY *e){
  free(e->key); 
  free(e->data); 
  free(e);
}

void aggiungi(char *s){
  ENTRY *e = crea_entry(s);
  ENTRY *r = hsearch(*e,FIND);
  if(r==NULL) {
    r = hsearch(*e,ENTER);
    if(r==NULL) xtermina("errore o tabella piena", __LINE__, __FILE__);
    coppia *c = (coppia *) e->data;
    // inserisco in testa
    c->next = testa_lista_entry;
    testa_lista_entry = e;
  } else {
    // la stringa è gia' presente
    assert(strcmp(e->key,r->key)==0);
    coppia *d = (coppia *) r->data;
    d->valore +=1;
    distruggi_entry(e);
  }
}

int conta(char *s){
  //cerco s nella tabella hash
  ENTRY *e = crea_entry(s);
  ENTRY *r = hsearch(*e,FIND);
  if(r==NULL) return 0;
  coppia *c = (coppia *) e->data;
  int conto = c->valore;
  distruggi_entry(e);
  return conto;
}


void print_hashtable(){
  puts("\n stampo tablella hash");
  for (ENTRY *e  = testa_lista_entry ; e != NULL;) {
    coppia *c = (coppia *) e->data;
    printf("%s: %d\n", e->key, c->valore);
    e = c->next;
  }
  puts("\n");
}