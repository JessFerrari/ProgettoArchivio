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

#define Num_elem 1000000

// messaggio errore e stop
void termina(const char *messaggio){
  if(errno!=0) perror(messaggio);
	else fprintf(stderr,"%s\n", messaggio);
  exit(1);
}

// oggetto della hash table

//distruggo oggetto della hash table

/*funzioni per la hash table
    - aggiungi
    - conta
*/
void aggiungi(char *s){

}

int conta(char *s){

}
//thread capo scrittore

//thread capo lettore

//corpo dei thread scrittori

//corpo dei thread lettori

//gestione dei segnali

int main(int argc, char const *argv[])
{
  //THREAD GESTORE DEI SEGNALI

  //controllo degli elementi della linea di comando

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
