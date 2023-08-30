#include "xerrori.h"


void termina(const char *messaggio){
    if(errno!=0) perror(messaggio);
    else fprintf(stderr,"%s\n", messaggio);
    exit(1);
}

int main(int argc, char *argv[]) {
    if(argc!=2) {
        fprintf(stderr, "Uso : %s <nome_file>\n", argv[0]);
        exit(1);
    }

    //Apro il file
    FILE *f = fopen(argv[1], "r");
    if(f==NULL) {
        termina("Errore apertura file");
    }

    //Per la lettura dal file
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    //Apro la named pipe in modalità di scrittura
    int fd = open("caposc", O_WRONLY);
    if (fd == -1) {
        perror("Errore nell'apertura della named pipe");
        return 1;
    }



    puts("\nLeggo dal file:\n");
    while((read = getline(&line, &len, f)) != -1) {
        printf("%s\n", line);
        ssize_t writen = write(fd, line, read);
        if (writen == -1) {
            perror("Errore nella scrittura nella named pipe");
            close(fd);
            return 1;
        }
    }


    free(line);
    //Chiudo il file
    fclose(f);
    //Chiudo la named pipe
    close(fd);


    return 0;
}