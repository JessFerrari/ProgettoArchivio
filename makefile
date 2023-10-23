# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread


# definizione degli eseguibili
EXECS=main main_linked  provaScrittura  archivio client1 archivioprova

# se si scrive solo make di default compila main.c
all: $(EXECS)

main : main.c 
	$(CC) $(CFLAGS) -o main main.c

main_linked : main_linked.c 
	$(CC) $(CFLAGS) -o main_linked main_linked.c

provaScrittura: provaScrittura.c xerrori.c
	$(CC) $(CFLAGS) -o provaScrittura provaScrittura.c xerrori.c

archivio: archivio.c xerrori.c xerrori.c hashtable.c
	$(CC) $(CFLAGS) -o archivio archivio.c xerrori.c hashtable.c

client1: client1.c xerrori.c
	$(CC) $(CFLAGS) -o client1 client1.c xerrori.c

archivioprova: archivioprova.c xerrori.c
	$(CC) $(CFLAGS) -o archivioprova archivioprova.c xerrori.c

# esegu la cancellazione dei file oggetto e degli eseguibili
clean: 
	rm -f *.out $(EXECS)

