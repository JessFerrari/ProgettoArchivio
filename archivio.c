#include "xerrori.h"
#define QUI __LINE__,__FILE__


#define Num_elem 1000000
#define PC_buffer_len 10

void termina(const char *messaggio){
  if(errno!=0) perror(messaggio);
	else fprintf(stderr,"%s\n", messaggio);
  exit(1);
}

//struttura thread capo scrittore
typedef struct{
  int *numero_scrittori;
  char **buffsc;
  int *index;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} datiCapoScrittore;
 
//struttura thread capo lettore
typedef struct{
  int numero_lettori;
  char **bufflet;
} datiCapoLettore;

//funzione capo scrittore
/*
    - apre la FIFO capolet in lettura
    - legge una seq di byte
    - aggiunge 0 in fondo alla seq
    - tokenizza con  ".,:; \n\r\t"
    - mette il token nel buffer buffsc
*/

void *capo_scrittore_body(void *arg){
    //prendo i dati allegati al thread
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;

    //creo i thread scrittori
    for(int i=0; i<*(cs->numero_scrittori); i++){
    
    }


    //apro la pipe caposc in lettura
    int fd = open("caposc", O_RDONLY);
    if(fd==-1){
        termina("Errore apertura caposc.\n");
    } 
    printf ("Aperto la pipe caposc\n");
    
    int dimensione = 0;
    char *input_buffer = malloc(dimensione * sizeof(char));
    if(input_buffer==NULL){
        termina("[MALLOC] Errore allocazione memoria");
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
        printf("dimensione %d\n", dimensione);

        //realloco il buffer con la dimensione giusta
        input_buffer = realloc(input_buffer, dimensione * sizeof(char));
        if(input_buffer==NULL){
            termina("[REALLOC] Errore allocazione memoria");
        }

        //leggo la sequenza di n byte
        bytes_letti = read(fd, input_buffer, dimensione); 
        printf("lettura %zu bytes : %s\n", bytes_letti, input_buffer);
        
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
        printf("aggiungo 0 alla fine della stringa : %s\n", input_buffer);
        printf("La dimensione della seq ora è : %ld\n", bytes_letti+1);

        //tokenizzo la stringa
    
        char *copia;
        char *token = strtok(input_buffer, ".,:; \n\r\t");
        while(token != NULL){
            printf("Token %d: %s\n",*(cs->index), token);
            //duplico il token
            copia = strdup(token);
            //aggiungo la copia del token al buffer
            printf("aggiungo %s al buffer\n", copia);

            sem_wait(cs->sem_free_slots);
            cs->buffsc[*(cs->index)%PC_buffer_len] = copia;
            printf("%s\n", cs->buffsc[*(cs->index)]);
            sem_post(cs->sem_data_items); 

            token = strtok(NULL, ".,:; \n\r\t");;
            *(cs->index) = *(cs->index) + 1;
        }
        
        //stampo il buffer
        puts("\n\nbuffer :");
        for(int j=0; j<=*(cs->index); j++){
            printf("%s\n", cs->buffsc[j]);
        }
        
    }

    /*stampo il buffer finale
    puts("\n\nbuffer finale :");
    for(int j=0; j<=*(cs->index); j++){
        printf("%s\n", cs->buffsc[j]);
    }*/


    printf("Devo creare %d thread ausiliari\n", *(cs->numero_scrittori));
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
        termina("[MALLOC] Errore allocazione memoria");
    }
    //semafori per gli scrittori
    sem_t sem_free_slot_sc;
    sem_t sem_data_items_sc;
    sem_init(&sem_free_slot_sc, 0, PC_buffer_len);
    sem_init(&sem_data_items_sc, 0, 0);

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


    return 0;
}