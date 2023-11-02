#include "hashtable.h"
ENTRY *testa_lista_entry = NULL;
atomic_int tot_stringhe_inHT = 0;
#define QUI __LINE__,__FILE__



// ---- soluzone per il problema lettori/scrittori ----
// ------------ unfair per gli scrittori --------------

void read_lock(rwHT *z) {
  pthread_mutex_lock(&z->mutexHT); 
  // tutti sono magari svegli ma solo il più veloce lockerà la mutex
  while (z->writersHT == true) {
  // chi va ad accedere in lettura rimane bloccato finché c'è qualcuno che sta accedendo in scrittura
    pthread_cond_wait(&z->condHT, &z->mutexHT); // attende fine scrittura
  }
  z->readersHT++;
  pthread_mutex_unlock(&z->mutexHT);
}

void read_unlock(rwHT *z) { // quando un thread ha finito l'accesso in lettura
  assert(z->readersHT > 0);
  pthread_mutex_lock (&z->mutexHT);
  z->readersHT--; // cambio di stato
  if (z->readersHT == 0)
    pthread_cond_signal(&z->condHT); // da segnalare ad un solo writer che magari
  // era in attesa che non ci fossero più lettori
  pthread_mutex_unlock(&z->mutexHT);
}

void write_lock(rwHT *z) { // un thread vuole accedere in scrittura
  pthread_mutex_lock(&z->mutexHT);
  while (z->writersHT || z->readersHT > 0)
    // attende fine scrittura di un eventuale altro writer o se c'è qualche
    // lettore
    pthread_cond_wait(&z->condHT, &z->mutexHT);
  z->writersHT = true; // segnalo che sto scrivendo
  pthread_mutex_unlock(&z->mutexHT);
}

void write_unlock(rwHT *z) {
  assert(z->writersHT);
  pthread_mutex_lock(&z->mutexHT);
  z->writersHT = false; // cambio stato
  // segnala a tutti quelli in attesa che ho smesso di scrivere
  pthread_cond_broadcast(&z->condHT); // non uso la signal perché i thread
  // in lettura possono accedere contemporaneamente e possono essercene di più
  pthread_mutex_unlock(&z->mutexHT);
}

//-----------------Funzioni per la tabella hash-------------------

ENTRY *crea_entry(char *s, int n) {
  ENTRY *e = malloc(sizeof(ENTRY));
  if (e == NULL)
    xtermina("[ARCHIVIO] Errore ht 1 malloc crea_entry", QUI);
  e->key = strdup(s); // Salva copia di s
  e->data = malloc(sizeof(coppia));
  if (e->key == NULL || e->data == NULL)
    xtermina("[ARCHIVIO] Errore ht 2 malloc crea_entry", QUI);
  // Inizializzo coppia
  coppia *c = (coppia *)e->data; // Cast obbligatorio
  c->valore = n;
  c->next = NULL;
  return e;
}

void distruggi_entry(ENTRY *e){
  free(e->key); 
  free(e->data); 
  free(e);
}

void distruggi_hash() {
  while (testa_lista_entry != NULL) {
    ENTRY *h = testa_lista_entry;
    testa_lista_entry = ((coppia *)testa_lista_entry->data)->next;
    distruggi_entry(h);
  }
}

void aggiungi(char *s) {
  ENTRY *e = crea_entry(s, 1);
  ENTRY *r = hsearch(*e, FIND);
  if (r == NULL) { //La stringa non è stata trovata all'intero nella tabella hash e quindi la si deve inserire
    r = hsearch(*e, ENTER); // Inserisco la entry creata nella ht
    if (r == NULL)
      xtermina("[AGGIUNGI] Errore o tabella piena", QUI);

    // La metto anche in cima alla lista delle entry inserite
    coppia *c = (coppia *)e->data;
    // Salvo la vecchia lista dentro c->next
    c->next = testa_lista_entry;
    // e diventa la testa della lista
    testa_lista_entry = e;
    // Incremento anche il numero di stringhe totali distinte inserite nella ht
    tot_stringhe_inHT += 1;
  
  } else {
    // Altrimenti la stringa è già presente incremento solo il valore
    assert(strcmp(e->key, r->key) == 0);
    coppia *c = (coppia *)r->data;
    c->valore += 1;
    distruggi_entry(e); // Questa non la devo memorizzare
  }

}

int conta(char *s) {
  int tmp;
  // printf("Thread lettore %d conta %s\n", gettid(), s);
  ENTRY *e = crea_entry(s, 1);
  ENTRY *r = hsearch(*e, FIND);
  if (r == NULL) { // Se non c'è la stringa nella ht restituisco 0
    printf("%s non trovata\n", s);
    tmp = 0;
  } else {
    printf("%s -> %d\n", s, *((int *)r->data));
    tmp = *((int *)r->data);
  }
  // Distruggo la entry 'creata' perché non va allocata
  distruggi_entry(e);
  return tmp;
}

void stampa_entry(ENTRY *e) {
  coppia *c = (coppia *)e->data;
  printf("%s ----------- %d\n", e->key, c->valore);
} 

void stampa_lista_entry() {
  ENTRY *lis = testa_lista_entry;
  if (lis == NULL) {
    printf("Lista vuota\n");
  }
  // In input do il puntatore al primo elemento che chiamo lis
  while (lis != NULL) {
    coppia *c = (coppia *)lis->data;
    stampa_entry(lis);
    lis = c->next;
  }
}

void clear_hash(){
  //distruggo la hash table
  distruggi_hash();
  hdestroy();
  testa_lista_entry = NULL; 
  tot_stringhe_inHT = 0;
}

int numero_stringhe(){
  return tot_stringhe_inHT;
}