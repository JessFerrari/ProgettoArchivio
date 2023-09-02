#include "xerrori.h"


void termina(const char *messaggio){
    if(errno!=0) perror(messaggio);
    else fprintf(stderr,"%s\n", messaggio);
    exit(1);
}

int main(int argc, char *argv[]) {
    if(argc!=3) {
        fprintf(stderr, "Uso : %s <nome_file> <nome_pipe>\n", argv[0]);
        exit(1);
    }

    //Apro il file
    FILE *f = fopen(argv[1], "r");
    if(f==NULL) {
        termina("Errore apertura file\n");
    }

    //Per la lettura dal file
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    //Apro la named pipe in modalità di scrittura
    int fd = open(argv[2], O_WRONLY);
    if (fd == -1) {
        perror("Errore nell'apertura della named pipe\n");
        return 1;
    }



    puts("\nLeggo dal file:\n");
    while((read = getline(&line, &len, f)) != -1) {
        printf("%s\n", line );

        int dimensione = read;
        printf ("Dimensione: %d\n\n", dimensione);
        //scrivo la dimensione della sequenza
        ssize_t writedim = write(fd, &dimensione, sizeof(int));
        if (writedim == -1) {
            perror("Errore nella scrittura nella named pipe\n");
            close(fd);
            return 1;
        }

        //scrivo la sequenza
        ssize_t writen = write(fd, line, read);
        if (writen == -1) {
            perror("Errore nella scrittura nella named pipe\n");
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