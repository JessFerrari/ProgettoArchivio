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

// Testa della hash table
ENTRY *testa_lista_entry = NULL;

//struttura dei dati da memorizzare nella tabella hash
typedef struct {
  char *key;
  int data;
  ENTRY *next;
} HashEntry;

//creo un oggetto della hash table
ENTRY *crea_entry(char *s){
  ENTRY *e = malloc(sizeof(ENTRY));
  if(e==NULL) termina("errore malloc entry 1");
  e->key = strdup(s); // salva copia di s
  e->data = 1;
}

//distruggo oggetto della hash table

/*funzioni per la hash table
    - aggiungi
    - conta
*/

void aggiungi(char *s){

}

int conta(char *s){

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

//corpo dei thread scrittori
void *thread_scrittore(void *arg){
  
}
//corpo dei thread lettori
void *thread_lettore(void *arg){

}

//gestione dei segnali

int main(int argc, char const *argv[])
{
  //THREAD GESTORE DEI SEGNALI

  //controllo degli elementi della linea di comando
  if(argc!=3){
    fprintf(stderr, "Uso : %s <num_thread_lettori> <num_thread_scrittori>\n", argv[0]);
    exit(1);
  }

  int num_thread_lettori = atoi(argv[1]);
  int num_thread_scrittori = atoi(argv[2]);

  /*Ora creo qui le FIFO, ma poi le dovrò far creare dal server*/
  int e = mkfifo("caposc",0666);
  if(e==0)
    puts("FIFO caposc creata\n");
  else if(errno== EEXIST)
    puts("La FIFO caposc esiste già; procedo...\n");
  else    
    xtermina("Errore creazione named pipe caposc",__LINE__,__FILE__);
  e = mkfifo("capolet",0666);
  if(e==0)
    puts("FIFO capolet creata\n");
  else if(errno== EEXIST)
    puts("La FIFO capolet esiste già; procedo...\n");
  else    
    xtermina("Errore creazione named pipe capolet",__LINE__,__FILE__);

  /*namedpipe: 
    - apro in lettura la FIFO caposc
    - apro in lettura la FIFO capolet
  */

  //buffer condivisi 
  char buffSC[Num_elem];
  char bufflet[Num_elem];

  //creazione della hash table
  int ht = hcreate(Num_elem);
  if( ht == 0 ) {
      xtermina("Errore creazione HT",__LINE__,__FILE__);
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

