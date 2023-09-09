#include "hashtable.h"

// Possibile soluzione al problema lettori/scrittori
// Questa soluzione è unfair per gli scrittori che
// potrebbero essere messi in attesa indefinita
// se continuano ad arrivare lettori


void read_lock(rwHT *z) {
  pthread_mutex_lock(&z->mutexHT); 
  // tutti sono magari svegli ma solo il più veloce lockerà la mutex
  while (z->writersHT == true) 
  // chi va ad accedere in lettura rimane bloccato finché c'è qualcuno che sta accedendo in scrittura
    pthread_cond_wait(&z->condHT, &z->mutexHT); // attende fine scrittura
  z->readersHT++;
  pthread_mutex_unlock(&z->mutexHT);
}

void read_unlock(rwHT *z) { // quando un thread ha finito l'accesso in lettura
  assert(z->readersHT > 0);
  pthread_mutex_lock(&z->mutexHT);
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

