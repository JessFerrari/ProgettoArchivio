#! /usr/bin/env python3
import Log
import socket, sys, struct, threading

#Accetta sulla linea di comando il nome di uno o più file di testo
#Per ogni file deve essere creato un thread che si collega al server 
#Ivia una alla volte le linee del file come sequenze di byte, inviando prima la rispettiva lunghezza
#Viene usata la funzione recv_all mostrata a lezione per ricevere alla fine il numero di sequenze che il server ha ricevuto
#Mi devo connettere al

def recv_all(conn,n):
    """Funzione mostrata a lezione.
    Riceve esattamente n byte dal socket conn e li restituisce.
    Il tipo restituto è "bytes": una sequenza immutabile di valori 0-255.
    Funzione analoga alla readn in C"""
    chunks = b''
    bytes_recd = 0
    while bytes_recd < n:
        chunk = conn.recv(min(n - bytes_recd, 2048))
        if len(chunk) == 0:
            return 0
        chunks += chunk
        bytes_recd = bytes_recd + len(chunk)
    return chunks 

def sendfile(file):
    try:
        Log.print_client(f"Arrivo nella funzione per gestire il {file}")
        with open(file, 'r') as f:
            #Creo la connessione
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #Specifico l'idirizzo IP e la porta del server
            server_address = ('127.0.0.1', 55531)
            #Connetto al server
            client_socket.connect(server_address)
            #Invio al server l'identificativo per la connessione di tipo B, quindi invio 2
            client_socket.sendall(b'2')
            #variabile per la lunghezza della linea
            lenght = 0
            #leggo tutte le linee del file
            lines = f.readlines()
            for line in lines :
                Log.print_client(f"Linea da leggere: {line}", 2)
                line = line.strip()
                lenght = len(line)
                Log.print_client(f"Linea da inviare: {line}, di lunghezza {lenght}")
                if(lenght > 2048 ):
                    Log.print_client(f"Linea troppo lunga: {line}")
                    continue
                if(lenght == 0):
                    #Log.print_client(f"Linea vuota: {line}")
                    continue
                else:
                    #Invio la lunghezza della linea
                    client_socket.sendall(struct.pack('!i', lenght))
                    Log.print_client(f"Invio la lunghezza della linea")
                    #Invio la linea
                    client_socket.sendall(line.encode())
                    Log.print_client(f"Invio la linea")
                
            #segnalo che non ci sono altre sequenze inviando una sequenza lunga 0
            client_socket.sendall(b'\x00\x00\x00\x00')
            #ricevo del server il numero di sequenze ricevute
            nseq_bytes = recv_all(client_socket, 4)
            nseq = struct.unpack('i', nseq_bytes[:4])[0]
            Log.print_client(f"Il server ha ricevuto {nseq} sequenze")
            #chiudo la connessione
            client_socket.close()
    except ConnectionError as e:
        print(f"Errore di connessione: {e}")
    except FileNotFoundError as e:
        print(f"File non trovato: {e}")
    except Exception as e:
        print(f"Errore non gestito: {e}")

if __name__ == '__main__':
    #Controllo che sono stati passati i file (almeno 1)
    if len(sys.argv) < 2:
        Log.print_client(f"USO: {sys.argv[0]} <file1> [<file2> ...]")
        sys.exit(1)
    
    #Ottengo i nomi dei file dalla linea di comando
    files = sys.argv[1:]

    #Creo un thread per ogni file ricevuto da linea di comando
    threads = []
    for file in files:
        thread = threading.Thread(target=sendfile, args=(file,))
        Log.print_client(f"Avvio di un thread di connessione per il file {file}")
        threads.append(thread)
        thread.start()
    
    #Attendo il completamento di tutti i thread
    for thread in threads:
        thread.join()
    
    #FINE
    Log.normal_print(f"TERMINATO")