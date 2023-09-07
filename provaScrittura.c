#include "xerrori.h"
#define QUI __LINE__, __FILE__

int main(int argc, char *argv[]){
    if(argc<3){
        fprintf(stdout, "Uso : %s <nome file sc> <nome file lt>\n", argv[0]);
        exit(1);
    }
    //Per la lettura dal file
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    //apro il file in lettura
    FILE *f = xfopen(argv[1], "r", __LINE__, __FILE__);
    //Apro la pipe caposc in scrittura
    int fd = open("caposc", O_WRONLY);

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
    
    //leggo il secondo file
    char *line_l= NULL;
    size_t len_l = 0;
    ssize_t read_l;
    //apro il file in lettura
    FILE *fl = xfopen(argv[2], "r", __LINE__, __FILE__);
    //Apro la pipe caposc in scrittura
    int fd_l = open("capolet", O_WRONLY);

    puts("\nLeggo dal file:\n");
    while((read_l = getline(&line_l, &len_l, fl)) != -1) {
        printf("%s\n", line_l );

        int dimensione = read_l;
        printf ("Dimensione: %d\n\n", dimensione);
        //scrivo la dimensione della sequenza
        ssize_t writedim = write(fd_l, &dimensione, sizeof(int));
        if (writedim == -1) {
            perror("Errore nella scrittura nella named pipe\n");
            close(fd_l);
            return 1;
        }

        //scrivo la sequenza
        ssize_t writen = write(fd_l, line_l, read_l);
        if (writen == -1) {
            perror("Errore nella scrittura nella named pipe\n");
            close(fd_l);
            return 1;
        }
    }

    free(line_l);
    //Chiudo il file
    fclose(fl);
    //Chiudo la named pipe
    close(fd_l);
    


    return 0;
}