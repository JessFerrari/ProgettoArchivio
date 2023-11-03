# Progetto-Laboratorio-II
### Relazione del progetto finale del corso di Laboratorio II 22/23 - Ferrari Jessica 615531
--------------------------------------------------------------------------------------------
## Introduzione 
Il progetto svolto è la versione completa di un programma client-server che gestisce sequenze di bytes utilizzando files, named pipes e come struttura dati principali una tabella hash.
I client hanno il compito di leggere da dei file delle sequenza di bytes e inviarle al server il cui si occuoerà di inserirle nella named pipe adeguata e far partire un sottoprocesso che leggerà tali sequenze e, con ilparadigma produttore consumatore, andrà ad inserirle o a cercarle all'interno della tabella hash.


## Organizzazione file e scelte implementative

- `archivio.c`
    In questo file è presente la logica del programma.
    Il programma archivio.c viene fatto partire dal server ricevendo in input il numero di threads scrittori e di threads lettori da creare. All'interno del main vengono inizializzati 3 diversi threads : capo_scrittore, capo_lettore e gestore_sg.


  Al primo, ***capo_scrittore***, vengono passati tramite una struct apposita il numero di threads scrittori da creare, un buffer condiviso con i sudetti threads e due semafori usati per la scrittura nel buffer. Questo thread fa paretire gli n threads scrittori passandogli il buffer, i due semafori per la lettura dal buffer e una mutex, che utilizzano per la lettura concorrente all'interno del buffer condiviso.
Recupera dalla named pipe `caposc` le sequenze di bytes, precedute dalla loro lunghezza, aggiunge un byte `\0` in modo che venga riconosciuta come una stringa, la tockenizza e la copia del tocken viene inserita all'interno del buffer attraverso l'uso dei semafori.
Dopo aver finito la lettura dalla pipe inserisce nel buffer NULL per ogni thread scrittore per farlo terminare e li aspetta con la join. 
I threads scrittori accedono al buffer con la mutex, recuperano la parola, rilasciano la mutex e aggiungono la parola recuperata all'interno della hash table mediante la funzione `aggiungi(char parola)` presente nella libreria `hashtable.h`. Per aggiungerla usano il paradigma produttori consumatori visto nella lezione 40 e implementato nella medesima libreria mediante le funzioni `write_lock` e `write_unlock`.
Con un funzionamento simile, al thread ***capo_lettore***, vengono passati tramite una struct apposita il numero di threads lettori da creare, un buffer condiviso con i sudetti threads e due semafori usati per la scrittura nel buffer. Questo thread fa paretire gli n threads lettori passandogli il buffer, i due semafori per la lettura dal buffer, una mutex, che utilizzano per la lettura concorrente all'interno del buffer condiviso ed un file in cui stamperanno i loro risultati di ricerca all'interno della hash table.
Questa volta le sequenze di bytes, sempre precedute dalla loro lunghezza, vengono recuperate dalla named pipe `capolet`. Ad esse viene aggiunto un byte `\0`, vengono opportunamente tockenizzate e la copia del tocken viene inserita all'interno del buffer attraverso l'uso dei semafori.
Dopo aver finito la lettura dalla pipe inserisce nel buffer NULL per ogni thread lettore per farlo terminare e li aspetta con la join. 
I threads lettori accedono al buffer con la mutex, recuperano la parola, rilasciano la mutex e cercano la parola recuperata all'interno della hash table mediante la funzione `conta(char parola)` presente nella libreria `hashtable.h` che restituisce il numero di occorrenze di tale stringa. Per leggere nella tabella hash usano anche loro il paradigma produttori consumatori precedentemente citato e implementato nella medesima libreria mediante le funzioni `read_lock` e `read_unlock`.
Il thread ***gestore_sg*** gestisce i segnali `SIGTERM`, `SIGINT` e  `SIGURS1` mediante la funzione `sigwait`

- `server.py`
    Questo file contiene il server, il quale gestiscee le connessioni con i client, lancia il processo archivio, normalmente o con valgrind, e gestisce il segnale sigint terminando e facendo terminare a sua volta archivio.
  I client vengono diversificati in due tipi: quelli di tipi: 1 e 2. Dai primi il server riceve una singola sequenza di bytes preceduta dalla sua lunghezza e la inserisce nella named pipe `capolet`. Dai secondi invece riceve n sequenza e le inserisce nella named pipe `caposc`. Per capire quando queste sono terminate riceve dal client una sequenza di lunghezza 0 che indica che quel determinato client ha chiuso la connessione.
  Usa un file di logging in cui segna quanti bytes ha scritto su ogni pipe per ogni connessione.
  

- `client1`
    Il client di tipo 1, scritto in C, riceve sulla linea di comando un solo file e richiede una connessione per ogni linea che legge. In ogni connessione manda quindi la lunghezza della sequenza letta, convertita con la funzione `htonl`, e la sequenza stessa.
  

- `client2`
      Il client di tipo 2, scritto in python, riceve sulla linea di comando uno o più file e per ognuno crea un thread. Ogni thread, successivamente all'assegnamento di uno dei file, richiede la connessione con il server e manda le sequenze leggendole linea per linea evitando quelle vuote o quelle troppo lunghe. A fine file manda una sequenza lunga 0 per avvisare il server che non ci sono più sequenze e riceve dal server stesso il numero di sequenze che ha ricevuto. Dopo di che chiude la connessione. 


- `hashtable.h` & `hashtable.c`
    In questa libreria ho implementato il paradigma produttore-consumatore con l'uso di mutex e condition variables visto a lezione e le funzioni neccessarie per l'utilizzo della tabella hash.

- `xerrori.h` & `xerrori.c`
      Questa libreria è quella che ci è stata fornita dal professore durante le lezioni nella quale sono implementate delle funzioni per una corretta gestione degli errori.

## Compilazione ed esecuzione

Dopo aver clonato il repository usare il comando
`make`                       # per compilare i file .c

-> Per far partire il server usare il comando  
`./server.py #numThread -r numReaders -w #numWriters -v`

-> Per far partire i client usare i comandi    
`./client2 file file`        # scrive dati su archivio  
`./client1 file`             # interroga archivio  

-> Per terminare il server correttamente usare il comando  
`pkill -INT -f server.py`    # invia SIGINT a server.py che a sua volta termina archivio
