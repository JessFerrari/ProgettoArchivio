# ProgettoArchivio
Progetto finale del corso di Laboratorio 2

## Organizzazione file

- archivio.c
    In questo file è presente la logica del programma.
    Il programma archivio.c viene fatto partire dal server con due modalità; Per maggiori informazioni su queste ultime consultare la sezione server.

    Riceve in input il numero di thread scrittori e di thread lettori da creare e restituisce quanti lettori e quanti scrittori sono stati creati.

     ### Thread capo scrittore

     Questo thread legge delle sequenze di byte dalla named pipe caposc. Legge per prima la dimensione di ogni sequenza e poi la sequenza stessa.
     Dopo aver letto ogni sequenza la tokenizza con strtok, inserisce una copia del token nel buffer produttore consumatore gestito con i semafori.

- server.py
    Questo file è il server,

- client1

- client2




-> Per far partire il server usare il comando
$ ./server.py #numThread -r numReaders -w #numWriters -v

-> Per far partire i client usare i comandi
$ ./client2 <file> <file>      # scrive dati su archivio
$ ./client1 <file>             # interroga archivio

-> Per terminare il server correttamente usare il comando

$ pkill -INT -f server.py       # invia SIGINT a server.py
                                # che a sua volta termina archivio