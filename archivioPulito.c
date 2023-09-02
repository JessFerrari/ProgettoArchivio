#include "xerrori.h"
#define QUI __LINE__,__FILE__

#define PC_buffer_len 10

//struttura capo scrittore
typedef struct{
  int *numero_scrittori;
  char **buffsc;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoScrittore;

//struttura scrittore
typedef struct{
    int id;
    char **buffsc;
    int *index;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiScrittori;

//struttura capo lettore
typedef struct{
  int *numero_lettori;
  char **bufflet;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoLettore;

//struttura lettore
typedef struct{
    int id;
    char **bufflet;
    int *index;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiLettori;


//Funzione scrittore
void *scrittore_body(void *arg){
    //recupero i dati
    datiScrittori *ds = (datiScrittori *) arg;
    fprintf(stdout, "Scrittore %d partito:\n", ds->id);

    char *parola;
    int np = 0;

    do{
        fprintf(stdout,"[INDEX SCRITORE %d] : %d\n", ds->id, *(ds->index)%PC_buffer_len);
        //faccio la sem wait sul semaforo dei dati (sto per togliere un dato quindi se è 0 aspetterò)
        xsem_wait(ds->sem_data_items, __LINE__, __FILE__);
        //per leggere una parola dal buffer devo acquisire la mutex
        xpthread_mutex_lock(ds->mutex, QUI);
        parola = ds->buffsc[*(ds->index) % PC_buffer_len];
        //parola = ds->buffsc[*(ds->index)];
        //fprintf(stdout, "SCRITTORE %d, INDEX %d, PAROLA %s\n", ds->id, *(ds->index), parola);
        *(ds->index) += 1;
        //rilascio la mutex
        xpthread_mutex_unlock(ds->mutex, QUI);
        //ho liberato un posto nel buffer e quindi faccio la post
        xsem_post(ds->sem_free_slots, __LINE__, __FILE__);
        np++;

        //devo poi aggiungere la parola nella tabella hash

    }while(parola != NULL);

    fprintf(stdout, "SCRITTORE %d HA LETTO %d PAROLE\n", ds->id, np);
    pthread_exit(NULL);
}

//Funzione capo scrittore
void *capo_scrittore_body(void *arg){
    //recupero i dati
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;
    fprintf(stdout, "CAPO SCRITTORE PARTITO\n");

    //inizializzo i dati per gli scrittori
    pthread_mutex_t mutexS = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tS[*(cs->numero_scrittori)];
    datiScrittori ds [*(cs->numero_scrittori)];
    int indexS = 0;

    //creo i thread scrittori
    for(int i=0; i<*(cs->numero_scrittori); i++){
        ds[i].buffsc = cs->buffsc;
        ds[i].index = &indexS;
        ds[i].sem_free_slots = cs->sem_free_slots;
        ds[i].sem_data_items = cs->sem_data_items;
        ds[i].mutex = &mutexS;
        ds[i].id = i;
        xpthread_create(&tS[i], NULL, scrittore_body, ds+i, __LINE__, __FILE__);
    }

    //apro la pipe caposc in lettura
    int fd = open("caposc", O_RDONLY);
    if(fd==-1){
        xtermina("[PIPE] Errore apertura caposc.\n", __LINE__, __FILE__);
    } 

    //dati per leggere dalla pipe
    int size = 10;
    char *input_buffer = malloc(size * sizeof(char));
    if(input_buffer==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    size_t bytes_letti;

    //leggo dal buffer
    int np = 0;
    while(true){
        //leggo la dimensione della sequenza di bytes
        bytes_letti = read(fd, &size, sizeof(int));
        if(bytes_letti==0){
            printf("FIFO chiusa in lettura\n");
            break;
        }
        if(bytes_letti != sizeof(int)){
            perror("Errore nella lettura della lunghezza della sequenza di byte");
            break;
        }

        //realloco il buffer con la dimensione giusta
        input_buffer = realloc(input_buffer, size * sizeof(char));
        if(input_buffer==NULL){
            xtermina("[REALLOC] Errore allocazione memoria", __LINE__, __FILE__);
        }

        //leggo la sequenza di n byte
        bytes_letti = read(fd, input_buffer, size);
        if(bytes_letti==0){
            printf("FIFO chiusa in scrittura\n");
            break;
        }
        if(bytes_letti != size){
            perror("Errore nella lettura della sequenza di byte");
        }

        //aggiungo 0 alla fine della stringa
        input_buffer[bytes_letti] = 0x00; 
        input_buffer[bytes_letti+1] = '\0';

        //tokenizzo la stringa
        char *copia;
        char *token = strtok(input_buffer, ".,:; \n\r\t");
        while(token != NULL){
            copia = strdup(token);
            //aggiungo copia al buffer
            //faccio la wait sugli slot liberi, devo aggiungere, quindi se il buffer è pieno aspetto
            xsem_wait(cs->sem_free_slots, __LINE__, __FILE__);
            if(copia != NULL){
                cs->buffsc[*(cs->index) % PC_buffer_len] = copia;
                //cs->buffsc[*(cs->index)] = copia;
                fprintf(stdout, "BUFFER[%d] : %s\n", *(cs->index)%PC_buffer_len, cs->buffsc[*(cs->index)%PC_buffer_len]);
                *(cs->index) += 1;
            }
            //faccio la post sul sem dei dati in quanto ne ho aggiunto uno
            xsem_post(cs->sem_data_items, __LINE__, __FILE__);

            np++;
            token = strtok(NULL, ".,:; \n\r\t");
        }

    }

    fprintf(stdout, "CAPO SCRITTORE HA SCRITTO %d PAROLE\n", np);

    

    fprintf(stdout, "\n Prima di terminare gli scrittori lindice è %d\n\n", *(cs->index)%PC_buffer_len);
    //termino gli scrittori aggiungendo null nel buffer
    for(int i=0; i<*(cs->numero_scrittori); i++){
        xsem_wait(cs->sem_free_slots, __LINE__, __FILE__);
        cs->buffsc[*(cs->index) % PC_buffer_len] = NULL;
        //cs->buffsc[*(cs->index)] = NULL;
        *(cs->index) += 1;
        fprintf(stdout, "BUFFER[%d] : %s\n", *(cs->index)%PC_buffer_len, cs->buffsc[*(cs->index)%PC_buffer_len]);
        xsem_post(cs->sem_data_items, __LINE__, __FILE__);
    }

    //aspetto i thread scrittori
    for (int i=0; i<*(cs->numero_scrittori); i++){
        pthread_join(tS[i], NULL);
    }
    pthread_mutex_destroy(&mutexS);

    close(fd);
    pthread_exit(NULL);

}





//main
int main (int argc, char *argv[]){
    
    if(argc!=3){
        fprintf(stderr, "Uso : %s <num_thread_scrittori> <num_thread_lettori>\n", argv[0]);
        exit(1);
    }

    int w = atoi(argv[1]);

    //buffer per gli scrittori
    int index = 0;
    char **buffsc = malloc(PC_buffer_len * sizeof(char));
    if(buffsc==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }

    //semafori per gli scrittori
    sem_t sem_data_items;
    sem_t sem_free_slots;
    xsem_init(&sem_data_items, 0, 0, __LINE__, __FILE__ );
    xsem_init(&sem_free_slots, 0, PC_buffer_len, __LINE__, __FILE__);

    //capo scrittore
    pthread_t capo_scrittore;
    datiCapoScrittore cs;
    cs.numero_scrittori = &w;
    cs.buffsc = buffsc;
    cs.index = &index;
    cs.sem_free_slots = &sem_free_slots;
    cs.sem_data_items = &sem_data_items;

    //creo il thread capo scrittore
    xpthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs, __LINE__, __FILE__);

    //aspetto il capo scrittore
    pthread_join(capo_scrittore, NULL);

    //dealloco il buffer scrittori
    free(buffsc);

    return 0;
}