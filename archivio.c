#include "hashtable.h"
#include "xerrori.h"
#include <arpa/inet.h>

#define QUI __LINE__,__FILE__

#define FIFO_CAPOSC "caposc"
#define FIFO_CAPOLET "capolet"
#define PC_buffer_len 10
#define Num_elem 1000000
#define Max_sequence_length 2048

//-----Struct per scrivere e leggere nella tabella hash-----//
rwHT struct_rwHT;

//funzioni sotto al main per i thread
void *capo_scrittore_body(void *arg);
void *capo_lettore_body(void *arg);
void *scrittore_body(void *arg);
void *lettore_body(void *arg);
void *gestore_segnali(void *arg);

//-----Struct per il thread gestore dei segnali-----//
typedef struct { //Passo al gestore i thread capi in modo che ne possa fare la join
  pthread_t *threadCapoLet;
  pthread_t *threadCapoSc;
} structSegnali;

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
  FILE *filelog;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoLettore;

//Struct lettore
typedef struct{
    int id;
    char **bufflet;
    int *index;
    int *np;
    FILE *filelog;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiLettori;


//main
int main (int argc, char *argv[]){
    
    if(argc<3){
        fprintf(stderr, "[ARCHIVIO] Uso : %s <num_thread_scrittori> <num_thread_lettori> \n", argv[0]);
        exit(1);
    }

    //creo la tabella hash
    int ht = hcreate(Num_elem);
    if(ht == 0){
        xtermina("[ARCHIVIO] Errore allocazione hashtable\n", __LINE__, __FILE__);
    }

    //assegno i valori per la struct rwHT
    pthread_mutex_t mutexHT = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condHT = PTHREAD_COND_INITIALIZER;
    struct_rwHT.mutexHT = mutexHT;
    struct_rwHT.condHT = condHT;
    struct_rwHT.writersHT = false;
    struct_rwHT.readersHT = 0;
    

    //leggo dalla linea di comando il numero di lettori e scrittori ricevuti dal server (minimo 3)
    int w = atoi(argv[1]);
    int r = atoi(argv[2]);
    assert(w>=3 && r>=3);


    //apro il file di log dove tengo le occorrenze lette
    FILE *lettoriLog = xfopen("lettori.log", "w", QUI);

    //creo i buffer per il capo  lettore e per il capo scrittore con i rispettivi indici
    char **buffsc = calloc(PC_buffer_len, sizeof(char *));
    if(buffsc==NULL){
        xtermina("[ARCHIVO] Errore allocazione memoria\n", __LINE__, __FILE__);
    }
    char **bufflet = calloc(PC_buffer_len, sizeof(char *));
    if(bufflet == NULL){
        xtermina("[ARCHIVIO] Errore allocazione memoria\n", __LINE__, __FILE__);
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
    //inizializzo i semafori
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
    cs.np = &npsc;
    cs.sem_free_slots = &sem_free_slots_sc; 
    cs.sem_data_items = &sem_data_items_sc;


    //capo lettore
    pthread_t capo_lettore;
    datiCapoLettore cl;
    //inizializzo il capo lettore
    cl.numero_lettori = &r;
    cl.bufflet = bufflet; 
    cl.index = &indexLET; 
    cl.np = &nplet;
    cl.sem_free_slots = &sem_free_slots_let;
    cl.sem_data_items = &sem_data_items_let;
    cl.filelog = lettoriLog;

    
    //thread gestore dei segnali
    pthread_t gestore_sg;
    structSegnali sg;
    //inizializzo il thread per la gestione dei segnali
    sg.threadCapoLet = &capo_lettore;
    sg.threadCapoSc = &capo_scrittore;
    //inizializzo la maschera dei segnali del main
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    //creo il thread
    xpthread_create(&gestore_sg, NULL, gestore_segnali, &sg, __LINE__, __FILE__);
    printf("[ARCHIVIO] Gestore segnali partito\n");



    //creo i thread
    xpthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs, __LINE__, __FILE__);
    xpthread_create(&capo_lettore, NULL, &capo_lettore_body, &cl, __LINE__, __FILE__);
    
   
    //La terminazione dei capi è gestita dal thread dei segnali
    if(xpthread_join(gestore_sg, NULL, QUI) !=0){
        xtermina("[ARCHIVIO] Errore nella join del thread gestore segnali\n", QUI);
    }
    printf("[ARCHIVIO] Gestore segnali terminato\n");

    distruggi_hash();

    fclose(lettoriLog);

    xpthread_mutex_destroy(&mutexHT, QUI);
    xpthread_cond_destroy(&condHT, QUI);

    xsem_destroy(&sem_data_items_sc, QUI);
    xsem_destroy(&sem_free_slots_sc, QUI);
    xsem_destroy(&sem_data_items_let, QUI);
    xsem_destroy(&sem_free_slots_let, QUI);
    
    free(buffsc);
    free(bufflet);
    
    return 0;
}


//Funzione scrittore
void *scrittore_body(void *arg){

    //recupero i dati
    datiScrittori *ds = (datiScrittori *) arg;

    rwHT *rw = &struct_rwHT;
    char *parola;
    int np = 0;

    do{
        //faccio la sem wait sul semaforo dei dati (sto per togliere un dato quindi se è 0 aspetterò)
        xsem_wait(ds->sem_data_items, __LINE__, __FILE__);

        //per leggere una parola dal buffer devo acquisire la mutex
        xpthread_mutex_lock(ds->mutex, QUI);
        parola = ds->buffsc[*(ds->index) % PC_buffer_len];
        *(ds->index) += 1;

        //rilascio la mutex
        xpthread_mutex_unlock(ds->mutex, QUI);
        //incremento il numero di parole lette
        np++;

        //aggiungo la parola nella tabella hash
        if(parola != NULL){
            write_lock(rw);
            printf("[ARCHIVIO] Thread scrittore %d aggiunge %s\n", ds->id, parola);
            aggiungi(parola);
            free(parola);
            stampa_lista_entry();
            write_unlock(rw);
        }

        //ho liberato un posto nel buffer e quindi faccio la post
        xsem_post(ds->sem_free_slots, __LINE__, __FILE__);

      
    }while(parola != NULL);
    pthread_exit(NULL);
}

//Funzione capo scrittore
void *capo_scrittore_body(void *arg){
    
    //recupero i dati
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;
    fprintf(stdout, "[ARCHIVIO] CAPO SCRITTORE PARTITO\n");
    

    //inizializzo i dati per gli scrittori
    int indexS = 0;
  
    pthread_mutex_t mutexS = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tS[*(cs->numero_scrittori)];
    datiScrittori ds [*(cs->numero_scrittori)];
    
    
    //creo i thread scrittori
    for(int i=0; i<*(cs->numero_scrittori); i++){
        ds[i].buffsc = cs->buffsc;
        ds[i].index = &indexS;
        ds[i].sem_free_slots = cs->sem_free_slots;
        ds[i].sem_data_items = cs->sem_data_items;
        ds[i].mutex = &mutexS;
        ds[i].id = i;  
        ds[i].np = 0;      
        xpthread_create(&tS[i], NULL, scrittore_body, ds+i, __LINE__, __FILE__);
    } 

    //apro la pipe CAPOSC da cui leggerò le sequenze di byte
    int fd = open(FIFO_CAPOSC, O_RDONLY);
    if(fd==-1){
        xtermina("[ARCHIVIO] Errore apertura FIFO caposc.\n", __LINE__, __FILE__);
    }
    printf("[ARCHIVIO] FIFO caposc aperta in lettura\n");

    //dati per leggere dalla pipe
    int size = 0;
    char *input_buffer = malloc(sizeof(char));
    if(input_buffer==NULL){
        xtermina("[ARCHIVIO] Errore allocazione memoria\n", __LINE__, __FILE__);
    }
   
    while(true){

        //leggo la dimensione della sequenza di bytes
        ssize_t bytes_letti = read(fd, &size, sizeof(int));
        size = ntohl(size);

        if(bytes_letti==-1){
            xtermina("[ARCHIVIO] Errore nella lettura della dimensione della sequenza di byte\n", __LINE__, __FILE__);
        }

        if(bytes_letti == 0){
            printf("[ARCHIVIO] FIFO caposc chiusa in lettura\n");
            break;
        }

        if(bytes_letti != sizeof(int)){
            perror("[ARCHIVIO] Errore nella lettura della lunghezza della sequenza di byte\n");
            break;
        }

        printf("[ARCHIVIO] Dimensione della sequenza di byte %d\n", size);

        //realloco il buffer con la dimensione giusta
        input_buffer = realloc(input_buffer, (size+1) * sizeof(char));
        if(input_buffer==NULL){
            xtermina("[ARCHIVIO] Errore allocazione memoria\n", __LINE__, __FILE__);
        }

        //leggo la sequenza di n byte
        bytes_letti = read(fd, input_buffer, size * sizeof(char));
        if(bytes_letti==0){
            printf("[ARCHIVIO] FIFO caposc chiusa in scrittura\n");
            break;
        }
        
        if(bytes_letti != size ){
            perror("[ARCHIVIO] Errore nella lettura della sequenza di byte\n");
        }

        //aggiungo 0 alla fine della stringa 
        input_buffer[bytes_letti] = '\0' ;
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
                *(cs->index) += 1;
            }
            *(cs->np) += 1;

            //faccio la post sul sem dei dati in quanto ne ho aggiunto uno
            xsem_post(cs->sem_data_items, __LINE__, __FILE__);
            token = strtok(NULL, ".,:; \n\r\t");
        }
    }

    //termino gli scrittori aggiungendo null nel buffer    
    for(int i=0; i<*(cs->numero_scrittori); i++){
        xsem_wait(cs->sem_free_slots, __LINE__, __FILE__);
        cs->buffsc[*(cs->index) % PC_buffer_len] = NULL;
        *(cs->index) += 1;
        xsem_post(cs->sem_data_items, __LINE__, __FILE__);
    }

    //aspetto i thread scrittori
    for (int i=0; i<*(cs->numero_scrittori); i++){
        pthread_join(tS[i], NULL);
    }
    xpthread_mutex_destroy(&mutexS, QUI);

    free(input_buffer);
    xclose(fd, QUI);
    pthread_exit(NULL);

}


//Funzione lettore
void *lettore_body(void *arg){
    //recupero i dati
    datiLettori *dl = (datiLettori *) arg;
    int np = 0;
    rwHT *rw = &struct_rwHT;
    char *parola;
    //int conto = 0;

    do{
        //faccio la sem wait sul semaforo dei dati (sto per togliere un dato quindi se è 0 aspetterò)
        xsem_wait(dl->sem_data_items, __LINE__, __FILE__);
        //per leggere una parola dal buffer devo acquisire la mutex
        xpthread_mutex_lock(dl->mutex, QUI);
        parola = dl->bufflet[*(dl->index) % PC_buffer_len];
        *(dl->index) += 1;

        //rilascio la mutex
        xpthread_mutex_unlock(dl->mutex, QUI);
        np ++;
        
        //devo poi contare le occorenze della parola nella hash table
        if(parola!=NULL){
            read_lock(rw);
            int occorenze = conta(parola);
            fprintf(dl->filelog, "%s %d\n", parola, occorenze);
            free(parola);
            read_unlock(rw);
        }

        //ho liberato un posto nel buffer e quindi faccio la post
        xsem_post(dl->sem_free_slots, __LINE__, __FILE__);


    }while(parola != NULL);
    pthread_exit(NULL);
}

//Funzione capo lettore
void *capo_lettore_body(void *arg){

    //recupero i dati
    datiCapoLettore *cl = (datiCapoLettore *) arg;
    fprintf(stdout, "[ARCHIVIO] CAPO LETTORE PARTITO\n");

    //apro la pipe CAPOLET in lettura
    int fd = open(FIFO_CAPOLET, O_RDONLY);
    printf("[ARCHIVIO] FIFO capolet aperto in lettura\n");
    if(fd==-1){
        xtermina("[ARCHIVIO] Errore apertura capolet.\n", __LINE__, __FILE__);
    } 
    printf("[ARCHIVIO] FIFO capolet aperto in lettura\n");

    //inizializzo i dati per i lettori
    pthread_mutex_t mutexL = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tL[*(cl->numero_lettori)];
    datiLettori dl [*(cl->numero_lettori)];
    int indexL = 0;
    //creo i thread lettori
    for(int i=0; i<*(cl->numero_lettori); i++){
        dl[i].bufflet = cl->bufflet;
        dl[i].index = &indexL;
        dl[i].sem_free_slots = cl->sem_free_slots;
        dl[i].sem_data_items = cl->sem_data_items;
        dl[i].mutex = &mutexL;
        dl[i].id = i;
        dl[i].np = 0;
        dl[i].filelog = cl->filelog;
        xpthread_create(&tL[i], NULL, lettore_body, dl+i, __LINE__, __FILE__);
    }


    //dati per leggere dalla pipe
    int size = 2;
    char *input_buffer = malloc(size);
    if(input_buffer==NULL){
        xtermina("[ARCHIVIO] Errore allocazione memoria\n", __LINE__, __FILE__);
    }
    size_t bytes_letti;

    while(true){
        //leggo la dimensione della sequenza di bytes
        bytes_letti = read(fd, &size, sizeof(int));
        size = ntohl(size);
        printf("[ARCHIVIO] ho letto la dimensione della sequenza di byte dalla FIFO capolet : %d\n", size);
        if(bytes_letti==0){
            printf("[ARCHIVIO] FIFO capolet chiusa in lettura\n");
            break;
        }
        if(bytes_letti != sizeof(int)){
            printf("[ARCHIVIO ERRORE 1] Errore nella lettura della lunghezza della sequenza di byte: %ld \n", bytes_letti);
            break;
        }

        //realloco il buffer con la dimensione giusta
        input_buffer = realloc(input_buffer, (size+1) * sizeof(char));
        if(input_buffer==NULL){
            xtermina("[ARCHIVIO] Errore allocazione memoria\n", __LINE__, __FILE__);
        }

        //leggo la sequenza di n byte
        bytes_letti = read(fd, input_buffer, size);   
    
        if(bytes_letti==0){
            printf("[ARCHIVIO] FIFO capolet chiusa in scrittura\n");
            break;
        }
        if(bytes_letti != size){
            printf("[ARCHIVIO] nella lettura della sequenza di byte: ho letto %ld byte al posto di %d\n", bytes_letti, size);
        }

        //aggiungo 0 alla fine della stringa
        input_buffer[bytes_letti] = '\0' ;
        printf("[ARCHIVIO] ho letto la sequenza di byte dalla FIFO capolet : %s\n", input_buffer);

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
                *(cl->index) += 1;
            }
            *(cl->np) += 1;
            //faccio la post sul sem dei dati in quanto ne ho aggiunto uno
            xsem_post(cl->sem_data_items, __LINE__, __FILE__);
            token = strtok(NULL, ".,:; \n\r\t");
        }
    }
   
    fprintf(stdout, "[ARCHIVIO] CAPO LETTORE HA SCRITTO %d PAROLE\n", *(cl->np));
    //termino i lettori aggiungendo null nel buffer
    for(int i=0; i<*(cl->numero_lettori); i++){
        xsem_wait(cl->sem_free_slots, __LINE__, __FILE__);
        cl->bufflet[*(cl->index) % PC_buffer_len] = NULL;
        *(cl->index) += 1;
        xsem_post(cl->sem_data_items, __LINE__, __FILE__);
    }

    //aspetto i thread lettori
    for (int i=0; i<*(cl->numero_lettori); i++){
        pthread_join(tL[i], NULL);
    }
    xpthread_mutex_destroy(&mutexL, QUI);
    free(input_buffer);
    xclose(fd, QUI);
    pthread_exit(NULL);

}

void *gestore_segnali(void *arg){
    //recupero i dati
    structSegnali *sg = (structSegnali *) arg;
    //inizializzo la maschera dei segnali 
    sigset_t mask;
    //aggiungo i segnali che devo gestire
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);

    int segnale;

    while(true){
        int e = sigwait(&mask, &segnale);
        if(e != 0){
            xtermina("[ARCHIVIO] Errore nella sigwait\n", __LINE__, __FILE__);
        }
        if(segnale == SIGTERM){
            fprintf(stdout, "[ARCHIVIO] RICEVUTO SIGTERM\n");

            //faccio la join dei thread
            if (xpthread_join(*(sg -> threadCapoLet), NULL, QUI) != 0){
                xtermina("[SIGTERM] Errore nell'attesa del thread capolettore\n", QUI);
            }
            if (xpthread_join(*(sg -> threadCapoSc), NULL, QUI) != 0){
                xtermina("[SIGTERM] Errore nell'attesa del thread caposcrittore\n", QUI);
            }
            int num_stringhe = numero_stringhe();
            //stampo su stdout il numero di stringhe presenti nella hash table
            fprintf(stdout, "[SIGTERM] Numero di stringhe distinte nella ht -> %d\n", num_stringhe);
            stampa_lista_entry();
            pthread_exit(NULL);

        }
        if(segnale == SIGINT){
            fprintf(stdout, "[ARCHIVIO] RICEVUTO SIGINT\n");
            int num_stringhe = numero_stringhe();
            fprintf(stderr, "[SIGINT] Numero di stringhe distinte nella ht -> %d\n", num_stringhe);
            continue;
        }
        if(segnale == SIGUSR1){
            fprintf(stdout, "[ARCHIVIO] RICEVUTO SIGUSR1\n");
            //Ripristino la hash table
            clear_hash();
            int ht = hcreate(Num_elem);
            if(ht == 0)
                xtermina("[ARCHIVIO] Errore allocazione hashtable\n", __LINE__, __FILE__);
            printf("[SIGUSR1] Hashtable ripristinata\n");
            continue;
        }
    }
    return NULL;
}