# ProgettoArchivio
Progetto finale del corso di Laboratorio 2

## Organizzazione file

- archivio.c
    In questo file è presente il programma che viene fatto partire dal server. Riceve in input il numero di thread scrittori e di thread lettori da creare e restituisce quanti lettori e quanti scrittori sono stati creati.
     ###Thread capo scrittore
     Questo thread legge delle sequenze di byte dalla named pipe caposc. Legge per prima la dimensione di ogni sequenza e poi la sequenza stessa.
     Dopo aver letto ogni sequenza la tokenizza con strtok, inserisce una copia del token nel buffer produttore consumatore gestito con i semafori.
     