/*Libreria per la creazione dei thread lettori e scrittori*/

#include "xerrori.h"
#include "hash.h"
#define QUI __LINE__,__FILE__

#define PC_buffer_len 10

//struttura capo scrittore
typedef struct datiCapoScrittore{
  int *numero_scrittori;
  char **buffsc;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  hashtable *hasht;
} datiCapoScrittore;

//struttura scrittore
typedef struct datiScrittori{
    int id;
    char **buffsc;
    int *index;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    hashtable *hasht;
} datiScrittori;

//struttura capo lettore
typedef struct datiCapoLettore{
  int *numero_lettori;
  char **bufflet;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  hashtable *hasht;
} datiCapoLettore;

//struttura lettore
typedef struct datiLettori{
    int id;
    char **bufflet;
    int *index;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    hashtable *hasht;
} datiLettori;

void *scrittore_body (void *arg);
void *capo_scrittore_body (void *arg);
void *lettore_body (void *arg);
void *capo_lettore_body (void *arg);