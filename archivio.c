#include "xerrori.h"
#define QUI __LINE__,__FILE__


#define Num_elem 1000000
#define PC_buffer_len 10


//struttura thread capo scrittore e scrittori
typedef struct{
  int *numero_scrittori;
  char **buffsc;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoScrittore;

typedef struct{
    int ident;
    char **buffsc;
    int *index;
    pthread_mutex_t *mutexsc;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiScrittori;


//struttura thread capo lettore e lettori
typedef struct{
  int numero_lettori;
  char **bufflet;
} datiCapoLettore;

typedef struct{
    int ident;
} datiLettore;



/*Funzione scrittore :
    - legge dal buffer degli scrittori le parole
    - aggiunge ogni parola nella hash table
*/

void *scrittore_body(void *arg){
    puts("Scrittore partito"); 
    
    datiScrittori *ds = (datiScrittori *) arg;
    //printf("Scrittore %d\n", ds->ident);
    char *parola;
    int nparole = 0;
    do{
        printf("[INDEX SCRITTORI]: %d\n", *(ds->index));
        //faccio la wait sul semaforo dei dati
        xsem_wait(ds->sem_data_items, __LINE__, __FILE__);
        //acquisto la mutex
        xpthread_mutex_lock(ds->mutexsc, QUI);
        //leggo nel buffer
        parola = ds->buffsc[*(ds->index) % PC_buffer_len];
        *(ds->index) += 1;
        //rilascio la mutex
        xpthread_mutex_unlock(ds->mutexsc, QUI);
        //faccio la signal sul semaforo dei free slot
        xsem_post(ds->sem_free_slots, __LINE__, __FILE__);

        if(parola != NULL) {
            printf("Il thread %d ha letto la parola %s\n", ds->ident, parola);
            nparole++;
        }
    }while(parola != NULL);

    printf("Il thread %d ha letto %d parole\n", ds->ident, nparole);

    //puts("scrittore sta per finire");
    pthread_exit(NULL);
}



//funzione capo scrittore
/*
    - apre la FIFO capolet in lettura
    - legge una seq di byte
    - aggiunge 0 in fondo alla seq
    - tokenizza con  ".,:; \n\r\t"
    - mette il token nel buffer buffsc
*/
void *capo_scrittore_body(void *arg){
    //puts("Capo scrittore partito\n");
   
    //prendo i dati allegati al thread
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;

    pthread_mutex_t ms = PTHREAD_MUTEX_INITIALIZER;
    pthread_t ts[*(cs->numero_scrittori)];
    datiScrittori ds[*(cs->numero_scrittori)];
    int sindex = 0;
    int nparole = 0;

    //creo i thread scrittori
    for(int i=0; i<*(cs->numero_scrittori); i++){
        ds[i].buffsc = cs->buffsc;
        ds[i].index = &sindex;
        ds[i].sem_free_slots = cs->sem_free_slots;
        ds[i].sem_data_items = cs->sem_data_items;
        ds[i].mutexsc = &ms;
        ds[i].ident = i;
        xpthread_create(&ts[i], NULL, scrittore_body, ds+i, __LINE__, __FILE__);
    }
    //puts("Thread scrittori creati\n");


    //apro la pipe caposc in lettura
    int fd = open("caposc", O_RDONLY);
    if(fd==-1){
        xtermina("Errore apertura caposc.\n", __LINE__, __FILE__);
    } 
    //printf ("Aperto la pipe caposc\n");
    
    int dimensione = 0;
    char *input_buffer = malloc(dimensione * sizeof(char));
    if(input_buffer==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    size_t bytes_letti;


    while(true){
        //leggo la lunghezza della sequenza di byte
        bytes_letti = read(fd, &dimensione, sizeof(int));
        if(bytes_letti==0){
            printf("FIFO chiusa in lettura\n");
            break;
        }
        if(bytes_letti != sizeof(int)){
            perror("Errore nella lettura della lunghezza della sequenza di byte");
            break;
        }
        //printf("dimensione %d\n", dimensione);

        //realloco il buffer con la dimensione giusta
        input_buffer = realloc(input_buffer, dimensione * sizeof(char));
        if(input_buffer==NULL){
            xtermina("[REALLOC] Errore allocazione memoria", __LINE__, __FILE__);
        }

        //leggo la sequenza di n byte
        bytes_letti = read(fd, input_buffer, dimensione); 
        //printf("lettura %zu bytes : %s\n", bytes_letti, input_buffer);
        
        if(bytes_letti==0){
            printf("FIFO chiusa in scrittura\n");
            break;
        }

        if(bytes_letti != dimensione){
            perror("Errore nella lettura della sequenza di byte");
        }
    


        //aggiungo 0 alla fine della stringa
        input_buffer[bytes_letti] = 0x00; 
        input_buffer[bytes_letti+1] = '\0';
        //printf("aggiungo 0 alla fine della stringa : %s\n", input_buffer);
        //printf("La dimensione della seq ora è : %ld\n", bytes_letti+1);

        //tokenizzo la stringa
    
        char *copia;
        char *token = strtok(input_buffer, ".,:; \n\r\t");
        while(token != NULL){
            //printf("Token %d: %s\n",*(cs->index), token);
            //duplico il token
            copia = strdup(token);
            //aggiungo la copia del token al buffer
            //printf("aggiungo %s al buffer\n", copia);

            sem_wait(cs->sem_free_slots);
            if (copia != NULL ) {
                cs->buffsc[*(cs->index)%PC_buffer_len] = copia;
                printf("buffer[%d]: %s\n",*(cs->index)%PC_buffer_len ,cs->buffsc[*(cs->index)%PC_buffer_len]);    
                *(cs->index) = *(cs->index) + 1;
            }
            //printf("%s\n", cs->buffsc[*(cs->index)]);
            sem_post(cs->sem_data_items); 
            nparole += 1;
            token = strtok(NULL, ".,:; \n\r\t");
            
        }
        
        /*stampo il buffer
        puts("\n\nbuffer :");
        for(int j=0; j<=*(cs->index); j++){
            printf("%s\n", cs->buffsc[j]);
        }*/
        
    }

    printf("Il thread capo ha scritto %d parole\n", nparole);

    //Termino scrittori : aggiungo null nel buffer
    for(int j=0; j<=*(cs->numero_scrittori); j++){
        sem_wait(cs->sem_free_slots);
        cs->buffsc[*(cs->index) % PC_buffer_len] = NULL;
        sem_post(cs->sem_data_items);
        //*(cs->index) = *(cs->index) + 1;
        printf("buffer[%d]: %s\n",*(cs->index)%PC_buffer_len ,cs->buffsc[*(cs->index)%PC_buffer_len]);    
    }

    

 
    for(int i=0; i<*(cs->numero_scrittori); i++){
        pthread_join(ts[i], NULL);
    }
    pthread_mutex_destroy(&ms);


    close(fd);
    pthread_exit(NULL);

}

//funzione capo lettore
/*void *capo_lettore_body(void *arg){
    datiCapoLettore *cl = (datiCapoLettore *) arg;
    //apro la pipe capolet in lettura
    int fd = open("capolet", O_RDONLY);
    if(fd==-1){
        xtermina("Errore apertura capolet", __LINE__, __FILE__);
    }

}*/

int main (int argc, char *argv[]){
    
    if(argc!=3){
        fprintf(stderr, "Uso : %s <num_thread_scrittori> <num_thread_lettori>\n", argv[0]);
        exit(1);
    }

    //numero di thread lettori e scrittori che devono partire oltre ai capi 
    int w = atoi(argv[1]);
    //int r = atoi(argv[2]);
    
    //buffer per gli scrittori
    int index=0;
    char **buffsc = malloc(PC_buffer_len * sizeof(char));
    if(buffsc==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    //semafori per gli scrittori
    sem_t sem_free_slot_sc;
    sem_t sem_data_items_sc;
    xsem_init(&sem_free_slot_sc, 0, PC_buffer_len, __LINE__, __FILE__);
    xsem_init(&sem_data_items_sc, 0, 0, __LINE__, __FILE__);

    //creo il thread capo scrittore 
    pthread_t capo_scrittore;
    datiCapoScrittore cs;
    cs.numero_scrittori = &w;
    cs.buffsc = buffsc;
    cs.index = &index;
    cs.sem_free_slots = &sem_free_slot_sc;
    cs.sem_data_items = &sem_data_items_sc;
    pthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs);

    pthread_join(capo_scrittore, NULL);



    //creo il capo lettore
    //pthread_t capo_lettore;

    free(buffsc);

    return 0;
}