
#include "xerrori.h"
#include <search.h>

#define QUI __LINE__,__FILE__

#define FIFO_CAPOSC "caposc"
#define FIFO_CAPOLET "capolet"
#define PC_buffer_len 10
#define Num_elem 1000000
#define Max_sequence_length 2048


// Testa della lista per la tabella hash 
ENTRY *testa_lista_entry = NULL;


//funzioni sotto al main per i thread
void *capo_scrittore_body(void *arg);
void *capo_lettore_body(void *arg);
void *scrittore_body(void *arg);
void *lettore_body(void *arg);

//funzioni sotto al main per la tabella hash
ENTRY *crea_entry(char *s, int n);
void distruggi_entry(ENTRY *e);

//-----Struct per i dati dei thread-----//

//Struct capo scrittore
typedef struct{
    int *numero_scrittori;
    char **buffsc;
    int *index;
    int *np;
    sem_t *sem_free_slots;
    sem_t *sem_data_items; 
} datiCapoScrittore;

//Struct scrittore
typedef struct{
    int id;
    char **buffsc;
    int *index;
    int *np;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiScrittori;

//Struct capo lettore
typedef struct{
  int *numero_lettori;
  char **bufflet;
  int *index;
  int *np;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoLettore;

//Struct lettore
typedef struct{
    int id;
    char **bufflet;
    int *index;
    int *np;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiLettori;

//-----Struct per la tabella hash-----//

//Struct per la lista di entry
typedef struct {
  int valore;    // numero di occorrenze della stringa 
  ENTRY *next;  
} coppia;


//main
int main (int argc, char *argv[]){
    
    if(argc<3){
        fprintf(stderr, "Uso : %s <num_thread_scrittori> <num_thread_lettori> \n", argv[0]);
        exit(1);
    }

    //leggo dalla line adi comando il numero di lettori e scrittori che si dovranno far partire
    int w = atoi(argv[1]);
    int r = atoi(argv[2]);

    //creo i buffer per il capo lettore e per il capo scrittore con i rispettivi indici
    char **buffsc = malloc(PC_buffer_len * sizeof(char *));
    if(buffsc==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    char **bufflet = malloc(PC_buffer_len * sizeof(char *));
    if(bufflet == NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    int indexSC = 0;
    int indexLET = 0;
    int npsc = 0;
    int nplet = 0;

    //creo i semafori che utilizzano i capi con i relatvi thread per leggere e scrivere nei rispettivi buffer
    //semafori per gli scrittori
    sem_t sem_data_items_sc;
    sem_t sem_free_slots_sc;
    //semafori per i lettori
    sem_t sem_data_items_let;
    sem_t sem_free_slots_let;

    xsem_init(&sem_data_items_sc, 0, 0, __LINE__, __FILE__ );
    xsem_init(&sem_free_slots_sc, 0, PC_buffer_len, __LINE__, __FILE__);
    xsem_init(&sem_data_items_let, 0, 0, __LINE__, __FILE__ );
    xsem_init(&sem_free_slots_let, 0, PC_buffer_len, __LINE__, __FILE__);


    //capo scrittore
    pthread_t capo_scrittore;
    datiCapoScrittore cs;
    //inizializzo il capo scrittore
    cs.numero_scrittori = &w;
    cs.buffsc = buffsc; 
    cs.index = &indexSC;
    cs.sem_free_slots = &sem_free_slots_sc; 
    cs.sem_data_items = &sem_data_items_sc;
    cs.np = &npsc;

    //capo lettore
    pthread_t capo_lettore;
    datiCapoLettore cl;
    //inizializzo il capo lettore
    cl.numero_lettori = &r;
    cl.bufflet = bufflet; 
    cl.index = &indexLET; 
    cl.sem_free_slots = &sem_free_slots_let;
    cl.sem_data_items = &sem_data_items_let;
    cl.np = &nplet;

    //creo i thread
    xpthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs, __LINE__, __FILE__);
    xpthread_create(&capo_lettore, NULL, &capo_lettore_body, &cl, __LINE__, __FILE__);
    
    //attendo la terminazione dei thread
    pthread_join(capo_scrittore, NULL);
    pthread_join(capo_lettore, NULL);

    for(int i= 0; i<npsc; i++){
        free(buffsc[i]);
    }
    for(int i = 0; i<nplet; i++){
        free(bufflet[i]);
    }

    free(buffsc);
    free(bufflet);
    return 0;
}


//Funzione capo scrittore
void *capo_scrittore_body(void *arg){
    
    //recupero i dati
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;
    fprintf(stdout, "CAPO SCRITTORE PARTITO\n");
    
    //apro la pipe CAPOSC da cui leggerò le sequenze di byte
    int fd = open(FIFO_CAPOSC, O_RDONLY);
    if(fd==-1){
        xtermina("[PIPE] Errore apertura caposc.\n", __LINE__, __FILE__);
    }

    //inizializzo i dati per i thread scrittori
    int indexS = 0;
    int np = 0;
    pthread_mutex_t mutexS = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tS[*(cs->numero_scrittori)];
    datiScrittori ds[*(cs->numero_scrittori)];
    for(int i=0; i<*(cs->numero_scrittori); i++){ 
        ds[i].id = i;
        ds[i].buffsc = cs->buffsc;
        ds[i].index = &indexS;
        ds[i].np = &np;
        ds[i].mutex = &mutexS;
        ds[i].sem_free_slots = cs->sem_free_slots;
        ds[i].sem_data_items = cs->sem_data_items;
        xpthread_create(&tS[i], NULL, &scrittore_body, ds+i, __LINE__, __FILE__);
    }


    //dati per leggere dalla pipe
    int size = 2 ;

    char *input_buffer = malloc(size);
    if(input_buffer==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
   
    while(true){

        //leggo la dimensione della sequenza di bytes
        ssize_t bytes_letti = read(fd, &size, sizeof(int));

        if(bytes_letti == 0){
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
        //input_buffer[bytes_letti] = 0x00; 
        input_buffer[bytes_letti] = '\0';

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
                fprintf(stdout, "BUFFER[%d] : %s\n", *(cs->index)%PC_buffer_len, cs->buffsc[*(cs->index)%PC_buffer_len]);
                *(cs->index) += 1;
            }
            *(cs->np) += 1;
            //faccio la post sul sem dei dati in quanto ne ho aggiunto uno
            xsem_post(cs->sem_data_items, __LINE__, __FILE__);
            
            token = strtok(NULL, ".,:; \n\r\t");
        }
        input_buffer = realloc(input_buffer, 2);
    }

    fprintf(stdout, "CAPO SCRITTORE HA SCRITTO %d PAROLE\n", *(cs->np));
    
    //faccio terminare i thread scrittori aggiungendo [null] nel buffer
    for(int i=0; i<*(cs->numero_scrittori); i++){
        xsem_wait(cs->sem_free_slots, __LINE__, __FILE__);
        cs->buffsc[*(cs->index) % PC_buffer_len] = NULL;
        //fprintf(stdout, "BUFFER[%d] : %s\n", *(cs->index)%PC_buffer_len, cs->buffsc[*(cs->index)%PC_buffer_len]);
        *(cs->index) += 1;
        xsem_post(cs->sem_data_items, __LINE__, __FILE__);
    }

    //attendo la terminazione dei thread scrittori
    for(int i=0; i<*(cs->numero_scrittori); i++){
        xpthread_join(tS[i], NULL, __LINE__, __FILE__);
    }

    pthread_mutex_destroy(&mutexS);
    free(input_buffer);
    close(fd);
    pthread_exit(NULL);

}

//Funzione capo lettore
void *capo_lettore_body(void *arg){

    //recupero i dati
    datiCapoLettore *cl = (datiCapoLettore *) arg;
    fprintf(stdout, "CAPO LETTORE PARTITO\n");


    //apro la pipe CAPOLET in lettura
    int fd = open(FIFO_CAPOLET, O_RDONLY);
    if(fd==-1){
        xtermina("[PIPE] Errore apertura capolet.\n", __LINE__, __FILE__);
    } 

    //dati per leggere dalla pipe
    int size = 2;
    char *input_buffer = malloc(size);
    if(input_buffer==NULL){
        xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
    }
    size_t bytes_letti;

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
        //input_buffer[bytes_letti] = 0x00; 
        input_buffer[bytes_letti] = '\0';

        //tokenizzo la stringa
        char *copia;
        char *token = strtok(input_buffer, ".,:; \n\r\t");
        while(token != NULL){
            copia = strdup(token);
            //aggiungo copia al buffer
            //faccio la wait sugli slot liberi, devo aggiungere, quindi se il buffer è pieno aspetto
            xsem_wait(cl->sem_free_slots, __LINE__, __FILE__);
            if(copia != NULL){
                cl->bufflet[*(cl->index) % PC_buffer_len] = copia;
                fprintf(stdout, "BUFFER[%d] : %s\n", *(cl->index)%PC_buffer_len, cl->bufflet[*(cl->index)%PC_buffer_len]);
                *(cl->index) += 1;
            }
            *(cl->np) += 1;
            //faccio la post sul sem dei dati in quanto ne ho aggiunto uno
            xsem_post(cl->sem_data_items, __LINE__, __FILE__);
           
            token = strtok(NULL, ".,:; \n\r\t");
        }
        
        input_buffer = realloc(input_buffer, 2);
    }
   
    fprintf(stdout, "CAPO LETTORE HA SCRITTO %d PAROLE\n", *(cl->np));
    
    free(input_buffer);
   
    close(fd);
    pthread_exit(NULL);

}

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
        //fprintf(stdout, "SCRITTORE %d, INDEX %d, PAROLA %s\n", ds->id, *(ds->index), parola);
        *(ds->index) += 1;
        //rilascio la mutex
        xpthread_mutex_unlock(ds->mutex, QUI);
        //ho liberato un posto nel buffer e quindi faccio la post
        xsem_post(ds->sem_free_slots, __LINE__, __FILE__);
        np++; //incremento il numero di parole lette

      
    }while(parola != NULL);

    fprintf(stdout, "SCRITTORE %d HA LETTO %d PAROLE\n", ds->id, np);
    pthread_exit(NULL);
}