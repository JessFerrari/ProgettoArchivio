#include "xerrori.h"

#define Num_elem 1000000

void termina(const char *messaggio){
  if(errno!=0) perror(messaggio);
	else fprintf(stderr,"%s\n", messaggio);
  exit(1);
}

//struttura thread capo scrittore
typedef struct{
  int numero_scrittori;
  //char *buffsc;
} datiCapoScrittore;

//struttura thread capo lettore
typedef struct{
  int numero_lettori;
  //char *bufflet;
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
    datiCapoScrittore *cs = (datiCapoScrittore *) arg;
    //apro la pipe caposc in lettura
    int fd = open("caposc", O_RDONLY);
    if(fd==-1){
        termina("Errore apertura caposc");
    } 
    printf ("Aperto la pipe caposc\n");
    
    //leggo una sequenza di byte finchè non finisco
    char input_buffer[2048];
    size_t bytes_letti;
    while(true){

        bytes_letti = read(fd, input_buffer, 2048); 
        if(bytes_letti==0){
            printf("FIFO chiusa in scrittura\n");
            break;
        }
        printf("lettura %zu bytes\n", bytes_letti);
        printf("lettura %s\n", input_buffer);
        //aggiungo 0 alla fine della stringa
        input_buffer[bytes_letti] = '0'; 
        input_buffer[bytes_letti+1] = '\0';
        printf("aggiungo 0 alla fine della stringa : %s\n", input_buffer);

        //tokenizzo la stringa
        int i=0;
        char *token = strtok(input_buffer, ".,:; \n\r\t");
        while(token != NULL){
            printf("Token %d: %s\n", i, token);
            token = strtok(NULL, ".,:; \n\r\t");
            i++;
        }
        
    }
    printf("Devo creare %d thread ausiliari\n", cs->numero_scrittori);
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
    

    //creo il thread capo scrittore 
    pthread_t capo_scrittore;
    datiCapoScrittore cs;
    cs.numero_scrittori = w;
    pthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs);

    pthread_join(capo_scrittore, NULL);



    //creo il capo lettore
    //pthread_t capo_lettore;


    return 0;
}