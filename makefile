# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread



# definizione degli eseguibili
EXECS=main.out main_linked.out archivio.out provaScrittura.out

# se si scrive solo make di default compila main.c
all: $(EXECS)

# regola per la creazioni degli eseguibili utilizzando xerrori.o
%.out: %.o xerrori.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# regola per la creazione di file oggetto che dipendono da xerrori.h
%.o: %.c xerrori.h
	$(CC) $(CFLAGS) -c $<
 
# esempio di target che non corrisponde a una compilazione
# ma esegue la cancellazione dei file oggetto e degli eseguibili
clean: 
	rm -f *.out $(EXECS)



