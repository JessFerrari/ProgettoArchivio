#define _GNU_SOURCE   /* See feature_test_macros(7) */
#include <stdio.h>    // permette di usare scanf printf etc ...
#include <stdlib.h>   // conversioni stringa/numero exit() etc ...
#include <stdbool.h>  // gestisce tipo bool (variabili booleane)
#include <assert.h>   // permette di usare la funzione assert
#include <string.h>   // confronto/copia/etc di stringhe
#include <errno.h>
#include <search.h>
#include <signal.h>
#include <unistd.h>  // per sleep
#include "xerrori.h"

#define Num_elem 1000000


//struttura dei dati da memorizzare nella tabella hash
typedef struct {
  int valore;
  ENTRY *next;
} coppia;

//struttura thread capo scrittore
typedef struct{
  int pipe_sc;
  int numero_scrittori;
  char *buffsc;
} capoScittore;

//struttura thread capo lettore
typedef struct{
  int pipe_let;
  int numero_lettore;
  char *bufflet;
} capoLettore;

//struttura thread scrittore

//struttura thread lettore


// Testa della hash table
ENTRY *testa_lista_entry = NULL;


//creo un oggetto della hash table
ENTRY *crea_entry(char *s){
  ENTRY *e = malloc(sizeof(ENTRY));
  if(e==NULL) termina("errore malloc entry 1");
  e->key = strdup(s); // salva copia di s
  e->data = malloc(sizeof(coppia));
  if(e->key==NULL || e->data==NULL)
    termina("errore malloc entry 2");
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

/*funzioni per la hash table
    - aggiungi
    - conta
*/

void aggiungi(char *s){
  ENTRY *e = crea_entry(s);
  ENTRY *r = hsearch(*e,FIND);
  if(r==NULL) {
    r = hsearch(*e,ENTER);
    if(r==NULL) termina("errore o tabella piena");
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

/*thread capo scrittore
  - legge dalla pipe caposc seq di byte
  - aggiunge 0 in fondo alla seq
  - tokenizza con  ".,:; \n\r\t"
  - mette il token nel buffer buffSC
*/
void *capo_scrittore(void *arg){
  
}

/*thread capo lettore
  - legge dalla pipe capolet seq di byte
  - tokenizza con  ".,:; \n\r\t"
  - mette il token nel buffer bufflet
*/
void *capo_lettore(void *arg){

}

/*corpo dei thread scrittori
void *thread_scrittore(void *arg){
  
}
//corpo dei thread lettori
void *thread_lettore(void *arg){

}*/

//gestione dei segnali

int main(int argc, char const *argv[])
{
  //THREAD GESTORE DEI SEGNALI

  //controllo degli elementi della linea di comando
  if(argc!=3){
    fprintf(stderr, "Uso : %s <num_thread_lettori> <num_thread_scrittori>\n", argv[0]);
    exit(1);
  }

  int r = atoi(argv[1]);
  int w = atoi(argv[2]);

  /*Ora creo qui le FIFO, ma poi le dovrò far creare dal server*/
  int e = mkfifo("caposc",0666);
  if(e==0)
    puts("FIFO caposc creata\n");
  else if(errno == EEXIST)
    puts("La FIFO caposc esiste già; procedo...\n");
  else    
    termina("Errore creazione named pipe caposc");
  e = mkfifo("capolet",0666);
  if(e==0)
    puts("FIFO capolet creata\n");
  else if(errno == EEXIST)
    puts("La FIFO capolet esiste già; procedo...\n");
  else    
    termina("Errore creazione named pipe capolet");

  /*namedpipe: LO DEVONO FARE I THREAD
    - apro in lettura la FIFO caposc
    - apro in lettura la FIFO capolet
  */  

  int fsc = open("caposc",O_RDONLY);
  int flet = open("capolet",O_RDONLY);
  if ( fsc < 0 )   termina("Errore apertura named pipe");
  if ( flet < 0 )  termina("Errore apertura named pipe");

  //buffer condivisi 
  char buffSC[Num_elem];
  char bufflet[Num_elem];

  //creazione della hash table
  int ht = hcreate(Num_elem);
  if( ht == 0 ) {
      termina("Errore creazione HT");
  }

  //threads

  //thread CAPO SCRITTORE
  

  //thread CAPO LETTORE
  


  //distruggo la hash table
  hdestroy();

  /*restituisce sulla linea di comando due interi :
    - r : che indica il numero di di thread lettori che eseguono l'operazione conta
    - w : che indica il numero di thread scrittori che eseguono l'operazione aggiungi
  */ 
  return 0;

}

void termina(const char *s) {
  fprintf(stderr,"%s\n",s);
  exit(1);
}