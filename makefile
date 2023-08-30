# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -O -g -pthread
LDLIBS=-lm

# definizione degli eseguibili
EXECS=main main_linked archivio provaScrittura

# se si scrive solo make di default compila main.c
all: $(EXECS)

# regola per la creazioni degli eseguibili utilizzando xerrori.o
%.out: %.o xerrori.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)


# regola per la creazione di file oggetto che dipendono da xerrori.h
%.o: %.c xerrori.h
	$(CC) $(CFLAGS) -c $<

	
clean: 
	rm -f *.o $(EXECS)
