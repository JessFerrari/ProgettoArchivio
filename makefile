# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC = gcc
CFLAGS = -std=c11 -Wall -g -O -pthread
LDLIBS = -lm -lrt -pthread


# definizione degli eseguibili
EXECS = archivio client1

# se si scrive solo make di default compila main.c
all: $(EXECS)

archivio: archivio.c xerrori.c xerrori.c hashtable.c
	$(CC) $(CFLAGS) -o archivio archivio.c xerrori.c hashtable.c

client1: client1.c xerrori.c
	$(CC) $(CFLAGS) -o client1 client1.c xerrori.c

# esegu la cancellazione dei file oggetto e degli eseguibili
clean: 
	rm -f *.out $(EXECS)

