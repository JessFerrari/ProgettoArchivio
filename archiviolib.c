#include "rw.h"


int main (int argc, char *argv[]){
    
    if(argc<3){
        fprintf(stderr, "Uso : %s <num_thread_scrittori> <num_thread_lettori> <L o S>\n", argv[0]);
        exit(1);
    }

    int w = atoi(argv[1]);
    int r = atoi(argv[2]);
    char *SL = argv[3];

    if(strcmp(SL, "l")){
        //buffer per gli scrittori
        int indexSC = 0;
        char **buffsc = malloc(PC_buffer_len * sizeof(char));
        if(buffsc==NULL){
            xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
        }
        //semafori per gli scrittori
        sem_t sem_data_items_sc;
        sem_t sem_free_slots_sc;
        xsem_init(&sem_data_items_sc, 0, 0, __LINE__, __FILE__ );
        xsem_init(&sem_free_slots_sc, 0, PC_buffer_len, __LINE__, __FILE__);

        
        //capo scrittore
        pthread_t capo_scrittore;
        datiCapoScrittore cs;
        cs.numero_scrittori = &w;
        cs.buffsc = buffsc;
        cs.index = &indexSC;
        cs.sem_free_slots = &sem_free_slots_sc;
        cs.sem_data_items = &sem_data_items_sc;
        
        //creo il thread capo scrittore
        xpthread_create(&capo_scrittore, NULL, &capo_scrittore_body, &cs, __LINE__, __FILE__);
        //aspetto il capo scrittore
        pthread_join(capo_scrittore, NULL);

        free(buffsc);
    }

    if(strcmp(SL,"s")){
        //buffer per i lettori
        int indexLET = 0;
        char **bufflet = malloc(PC_buffer_len * sizeof(char));
        if(bufflet == NULL){
            xtermina("[MALLOC] Errore allocazione memoria", __LINE__, __FILE__);
        }

        //semafori per i lettori
        sem_t sem_data_items_let;
        sem_t sem_free_slots_let;
        xsem_init(&sem_data_items_let, 0, 0, __LINE__, __FILE__ );
        xsem_init(&sem_free_slots_let, 0, PC_buffer_len, __LINE__, __FILE__);
    

        //capo lettore
        pthread_t capo_lettore;
        datiCapoLettore cl;
        cl.numero_lettori = &r;
        cl.bufflet = bufflet;
        cl.index = &indexLET;
        cl.sem_free_slots = &sem_free_slots_let;
        cl.sem_data_items = &sem_data_items_let;
        //creo il thread capo lettore
        xpthread_create(&capo_lettore, NULL, &capo_lettore_body, &cl, __LINE__, __FILE__);
        //aspetto il capo lettore
        pthread_join(capo_lettore, NULL);

        free(bufflet);
    }


    return 0;
}