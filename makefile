# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread


# definizione degli eseguibili
EXECS=main main_linked  provaScrittura  archivio

# se si scrive solo make di default compila main.c
all: $(EXECS)

provaScrittura: provaScrittura.c xerrori.c
	$(CC) $(CFLAGS) -o provaScrittura provaScrittura.c xerrori.c

archivio: archivio.c xerrori.c
	$(CC) $(CFLAGS) -o archivio archivio.c xerrori.c


# esegu la cancellazione dei file oggetto e degli eseguibili
clean: 
	rm -f *.out $(EXECS)

