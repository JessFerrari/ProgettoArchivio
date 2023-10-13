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

/*Funzione mostrata a lezione.
 *Permette di scrivere i dati in modo che non devo controllare ogni volta che tutti i dati siano stati scritti.
 *Si chiede di scrivere tot byte e va avanti finché non riesce a scriverli tutti.
 *funzione analoga per la lettura*/
ssize_t writen(int fd, void *ptr, size_t n) {
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
        fprintf(stderr, "Uso : %s <nome_file>\n", argv[0]);
        exit(1);
    }

    //variabili per la lettura e la comunicazione con il server
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    size_t e;
    int tmp;

    //apro il file in lettura
    FILE *f = xfopen(argv[1], "r", __LINE__, __FILE__);
    if(f == NULL){
        xtermina("[CLIENT1] Errore nella apertura del file\n", __LINE__, __FILE__);
    }

    //leggo una linea e la mando con una connessione
    while((read = getline(&line, &len, f)) != -1) {
        printf("[CLIENT1] ho letto dal file: %s\n", line );

        //se la linea è vuota viene saltata
        if (strlen(line) == 1 && line[0] == '\n') {
        printf("[CLIENT1] Linea vuota, la skippo\n");
        continue;
    }
        //se la dimensione è >2048 allora è troppo grande e viene saltata
        if(strlen(line) > 2048){
            fprintf(stderr, "[CLIENT1] Linea troppo lunga\n");
            continue;
        }

        //a questo punto la linea è correta e quindi instauro una connessione
        //creo il socket
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(client_socket == -1){
            xtermina("[CLIENT1] Errore nella creazione del socket\n", __LINE__, __FILE__);
        }
        
        //connetto il client al server
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
        printf("[CLIENT1] Connessione effettuata \n");

        //identifioco il tipo di connessione 
        char *connection_type = "1";
        //mando il tipo di connessione
        if(send(client_socket, connection_type, strlen(connection_type), 0) < 0){
            xtermina("[CLIENT1] Errore nella scrittura del tipo di connessione\n", __LINE__, __FILE__);
        }

        //invio la lunghezza della linea
        tmp = htonl(strlen(line));
        e = writen(client_socket, &tmp, sizeof(tmp));
        if(e != sizeof(int)){
            xtermina("[CLIENT1] Errore nella scrittura della lunghezza della linea\n", __LINE__, __FILE__);
        }
        printf("[CLIENT1] Lunghezza della linea: %ld, inviata al server\n", strlen(line));

        //invio la sequenza di caratteri
        if(send(client_socket, line, read, 0) < 0){
            xtermina("[CLIENT1] Errore nella scrittura della sequenza di caratteri\n", __LINE__, __FILE__);
        }
        printf("[CLIENT1] Sequenza di caratteri inviata al server\n");

        xclose(client_socket, __LINE__, __FILE__);

    }

    //chiudo il file
    fclose(f);
    //libero line
    free(line);
    printf("[CLIENT1] Completato\n");
    return 0;
}