#include "xerrori.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

// Costanti
#define QUI __LINE__, __FILE__
#define HOST "127.0.0.1"
#define PORT 55531 
#define Max_sequence_length 2048

/*Accetta sulla linea di comando il nome di un file di testo
Crea per ogni lineea una connessione di tipo A, si presenta mandando l'identificativo 1
Invia sulla connessione la linea letta dal file*/

ssize_t writen(int fd, void *ptr, size_t n) {    
/*Funzione mostrata a lezione.
 *Permette di scrivere i dati in modo che non devo controllare ogni volta che tutti i dati siano stati scritti.
 *Si chiede di scrivere tot byte e va avanti finché non riesce a scriverli tutti.
 *funzione analoga per la lettura*/
    size_t nleft;
    ssize_t nwritten;

    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n)
            return -1; 
        else
            break; 
        } else if (nwritten == 0)
        break;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n - nleft); /* return >= 0 */
}

int main(int argc, char *argv[]) {
    //controllo che venga passato il nome del file
    if(argc != 2){
        fprintf(stderr, "[CLIENT1] Uso : %s <nome_file>\n", argv[0]);
        exit(1);
    }

    //Apro il file in lettura
    FILE *f = xfopen(argv[1], "r", QUI);
    if(f == NULL){
        xtermina("[CLIENT1] Errore apertura file", __LINE__, __FILE__);
    }

    //Variabili per la lettura del file e la comunicazione con il server
    char *line = NULL;
    size_t length = 0;
    ssize_t bytes_read;
    size_t e;
    int tmp;

    while((bytes_read = getline(&line, &length, f)) != -1){
        printf("[CLIENT1] ho letto la linea %s di dimensione %ld\n", line, bytes_read);
        //se la linea è vuota la salto
        if (strlen(line) == 1 && line[0] == '\n') {
            printf("[CLIENT1] Linea vuota, la salto\n");
            continue;
        }
        //se la dimensione è > 2048 la salto
        if (strlen(line) > Max_sequence_length) {
            printf("[CLIENT1] Linea troppo lunga, la salto\n");
            continue;
        }
        //La linea rispetta i giusti parametri e instauro una connessione

        //creo il socket
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(client_socket == -1){
            xtermina("[CLIENT1] Errore nella creazione del socket\n", __LINE__, __FILE__);
        }

        //Connetto il client al server
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(HOST);
        server_addr.sin_port = htons(PORT);
        
        if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            printf("[CLIENT1] Errore connessione client1 sulla porta %d\n", htons(PORT));
            fclose(f);
            xclose(client_socket, __LINE__, __FILE__);
            exit(1);
        }

        //mi identifico mandando 1
        e = writen(client_socket, "1", 1);
        if(e == -1){
            xtermina("[CLIENT1] Errore nella scrittura del tipo di connessione\n", __LINE__, __FILE__);
        }else {
            printf("[CLIENT1] Connessione effettuata\n");
        }
        
        //mando la lunghezza della sequenza
        tmp = htonl(strlen(line));
        e = writen(client_socket, &tmp, sizeof(tmp));
        if(e == -1){
            xtermina("[CLIENT1] Errore nella scrittura della lunghezza della linea\n", __LINE__, __FILE__);
        } else {
            printf("[CLIENT1] Lunghezza della sequenza %s mandata\n", line);
        }

        //mando la sequenza
        e = writen(client_socket, line, strlen(line));
        if(e == -1){
            xtermina("[CLIENT1] Errore nella scrittura della linea\n", __LINE__, __FILE__);
        } else {
            printf("[CLIENT1] Sequenza %s mandata \n", line);
        }

        //chiudo la connessione
        xclose(client_socket, __LINE__, __FILE__);
    }

    free(line);
    fclose(f);
    return 0;
}