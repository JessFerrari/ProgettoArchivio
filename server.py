#! /usr/bin/env python3
import Log
import socket,  argparse, os, struct, logging, signal, threading, subprocess
from concurrent.futures import ThreadPoolExecutor
#inizializzo la socket del server 
server_socket = None
archivio_subprocess = None
#lock per scrivere nella pipe
pipe_lock = threading.Lock()

#Si deve mettere in attesa sull'indirizzo 127.0.0.1 e sulla porta 55531
#Ad ogni client che riceve deve assegnare un thread dedicato
#I client che può ricevere sono di due tipo: 
    #Tipo A che invia al server una singola sequenza di byte e ilserver la deve scrivere nalla pipe capolet
    #Tipo B che invia al server un numero imprecisato di sequenze di byte e il server le deve scrivere nalla pipe caposc. Infine il server invia al client il numero di sequenze ricevute
#deve gestire un file di log di nome server.log e scriverci il tipo di connessione e il numero di bytes scritti nella rispettiva pipe 
#Gestisce gli argomenti usando argparse
#fa partire il processo archivio, sia normalmente che con valgrind
#Se viene inviato il segnale SIGINT il aserver termina l'esecuzione chiudendo il socket, cancellandoo le fifo e inviando il segnale sigterm al programma archivio

def recv_all(conn,n):
  """Funzione mostrata a lezione, riceve esattamente n byte dal socket conn e li restituisce
    Il tipo restituto è "bytes": una sequenza immutabile di valori 0-255
    Questa funzione è analoga alla readn che abbiamo visto nel C
    conn : socket
    n : numero di byte"""
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 2048))
    if len(chunk) == 0:
      return 0
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks  

def write_in_pipe(pipe, size, data):
    """Funzione per scrivere in una pipe.
    pipe : pipe
    size : numero di byte della sequenza
    data : sequenza
    """
    with pipe_lock:
        try:
            if isinstance(size, bytes):
                os.write(pipe, size)
                Log.print_server(f"Ho scritto la lunghezza nella pipe {pipe}")
            else:
                Log.print_server(f"Errore: size non è di tipo bytes",3)
        except (IOError, OSError) as e:
            Log.print_server(f"Errore durante la scrittura di size su {pipe}: {e}", 3)
        try:
            if isinstance(data, bytes):
                os.write(pipe, data)
                Log.print_server(f"Ho scritto la sequenza di byte nella pipe {pipe}")
            else:
                Log.print_server(f"Errore: data non è di tipo bytes",3)
        except (IOError, OSError) as e:
            Log.print_server(f"Errore durante la scrittura di size su {pipe}: {e}", 3)

            

def gest_connessione(conn, addr, capolet, caposc):
    '''Funzione per gestire i client.
    Se ricevo un client di tipo 1 allora verrà instaurata una connessione di tipo A.
    Altrimenti, se ricevo un client di tipo 2 allora verranno instaurate una connessione di tipo B.
    conn : socket
    addr : address
    capolet : pipe capolet
    caposc : pipe caposc
    '''
    #Il client mi ha mandato una richiesta
    Log.print_server(f"mi ha contattato il client{addr}")
    #Ricevo un byte che mi indica se il client è di tipo 1 o 2
    client_type = recv_all(conn, 1).decode("utf-8")
    Log.print_server(f"Il client{addr} ha invuiato la richiesta di connessione di tipo {client_type}")
    #Il client è di tipo 1
    if client_type == "1":
        #Instauro una connessione di tipo A
        Log.print_server(f"Instaurata una connessione di tipo A con Client {addr} di tipo 1")
        bytes_written_capolet = 0
    
        #Ricevo la lunghezza della sequenza di byte che sto per ricevere (4 bytes)
        length_in_bytes = recv_all(conn, 4)
        Log.print_server(f"Ricevuta la lunghezza della sequenza di byte {length_in_bytes}")
        #Trasformo la sequenza di byte in un intero
        length = struct.unpack('!i', length_in_bytes[:4])[0]
        Log.print_server(f"La lunghezza della sequenza di byte {length}")
        #Ricevo la sequenza di byte
        data = recv_all(conn, length)
        Log.print_server(f"Ricevuta la sequenza di byte {data.decode('utf-8')}")
        #Invio la lunghezza della sequenza di byte
        write_in_pipe(capolet, length_in_bytes, data)
            
        bytes_written_capolet = bytes_written_capolet + length + 4
        #Registro le informazioni dei bytes scritti nella pipe capolet su file di log
        logging.info(f"Connessione di tipo A - byte scritti su 'capolet' -> {bytes_written_capolet}")
        #chiudo la connessione
        conn.close()

        #il client è di tipo 2
    elif client_type == "2":
        #Instauro una connessione di tipo B
        Log.print_server(f"Instaurata una connessione di tipo B con Client {addr} di tipo 2")
        bytes_written_caposc = 0
        tot_recived_seq = 0
        while True:
            #Ricevo la lunghezza della sequenza di byte che sto per ricevere (4 bytes)
            length_in_bytes = recv_all(conn, 4)
            #se è 0 allora il client ha finito la connessione
            if length_in_bytes == b'\x00\x00\x00\x00':
                Log.print_server(f"Ho ricevuto una sequenza di 0 bytes. Il client{addr} ha finito la connessione")
                break
            Log.print_server(f"Ricevuta lunghezza {length_in_bytes}")
            #Trasformo la sequenza di byte in un intero
            length = struct.unpack('!i', length_in_bytes[:4])[0]
            Log.print_server(f"La lunghezza ricevuta è {length} bytes")
            
            #Ricevo la sequenza di byte
            data = recv_all(conn, length)
            Log.print_server(f"Ricevuta la sequenza di byte {data}")

            #Scrivo nella pipe caposc la dimensione e la sequenza di byte
            write_in_pipe(caposc, length_in_bytes, data)

            tot_recived_seq = tot_recived_seq + 1
            bytes_written_caposc = bytes_written_caposc + length + 4

        #Registro le informazioni dei bytes scrittiInThe pipe capolet su file di log
        logging.info(f"Connessione di tipo B - byte scritti su 'caposc' -> {bytes_written_caposc}")
        #HA SENSO RESETTARE bytes_written_caposc???? NO...
        #invio al client il numero di sequenze ricevute
        conn.sendall(struct.pack('i', tot_recived_seq))
        #chiudo la connessione
        conn.close()

def shutdown_server(signum, frame):
    """
Funzione per terminare il server con il comando CTRL+C.
    signum : rappresenta il numero dell segnale.
    frame : rappresenta il frame corrente dello stack.
"""
    Log.normal_print("[...TERMINAZIONE SERVER...]") 
     
    # Chiude il socket del server
    if server_socket is not None:
        server_socket.shutdown(socket.SHUT_RDWR)
        server_socket.close()
    #else:
    #    Log.normal_print("<[SERVER NON INIZIALIZZATO]>")
    
    
    # Chiudo le pipe caposc e capolet
    if os.path.exists("caposc"):
        os.unlink("caposc")
    if os.path.exists("capolet"):
        os.unlink("capolet")
    
    # Invia il segnale SIGTERM al processo archivio
    archivio_subprocess.send_signal(signal.SIGTERM)

    Log.normal_print("<[SERVER TERMINATO]>")
    exit()



def archivio_normale(readers, writers):
    global server_socket, archivio_subprocess
    # Esegue il programma C
    archivio_subprocess = subprocess.Popen(['./archivio', str(readers), str(writers)])
    Log.print_server(f"Lancio il processo archivio {archivio_subprocess.pid} con {readers} lettori e {writers} scrittori")


# Funzione che lancia archivio con valgrind, passando il numero di lettori e scrittori e i parametri che il professore ha utilizzato in manager.py
def archivio_valgrind(readers, writers):
    global server_socket, archivio_subprocess
    # Esegue il programma C passando anche valgrind
    archivio_subprocess = subprocess.Popen(["valgrind","--leak-check=full", "--show-leak-kinds=all",  "--log-file=valgrind-%p.log", "./archivio", str(readers), str(writers)])
    Log.print_server(f"Ho lanciato il processo archivio {archivio_subprocess.pid} con valgrind")


def mainServer(numMaxThreads, writers, readers, valgrind):
    '''
    Funzione principale del server
    numMaxThreads : numero massimo di threads
    writers : numero di writers
    readers : numero di readers
    valgrind : booleano che indica se si vuole usare valgrind'''

    #configuro il server
    host = 'localhost'
    port = 55531

    #creo il socket del server
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if server_socket is None:
        Log.print_server("Errore durante la creazione del socket del server", 3)
        exit(1)  # Esce dal programma in caso di errore

    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_socket.bind((host, port))

    server_socket.listen(numMaxThreads)

    Log.print_server(f"Server in ascolto su {host}:{port}")

    #creo il threadpool per gestire i client
    executor = ThreadPoolExecutor(max_workers=numMaxThreads)
    Log.print_server(f"threadpool in creato con {numMaxThreads} thread")

    #inizializzo le pipes
    if not os.path.exists("capolet"):
        os.mkfifo("capolet", 0o0666)
        Log.print_server(f"pipe capolet creato")
    if not os.path.exists("caposc"):
        os.mkfifo("caposc", 0o0666)
        Log.print_server(f"pipe caposc creato")

    #faccio partire archivio
    if valgrind:
        archivio_valgrind(readers, writers)
    else:
        archivio_normale(readers, writers)

    #apro le pipes
    try:
        capolet = os.open("capolet", os.O_WRONLY)
        Log.print_server(f"pipe capolet aperta in scrittura")
        caposc = os.open("caposc", os.O_WRONLY)
        Log.print_server(f"pipe caposc aperta in scrittura")
    except Exception as e:
        Log.print_server(f"Errore durante l'apertura delle pipes: {e}")

    #configuro il file di log
    logging.basicConfig(filename="server.log", level=logging.INFO, format='%(asctime)s - %(message)s')

    #gestisco i segnali
    signal.signal(signal.SIGINT, shutdown_server)

    #accetto le connessioni
    while True:
        conn, addr = server_socket.accept()
        #assegno un thread ad ogni connessione
        executor.submit(gest_connessione, conn, addr, capolet, caposc)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="USE: ./server.py numMaxThreads -r n.readers -w n.writers -v")
    parser.add_argument("numMaxThreads", type=int, help="numero massimo di threads")
    parser.add_argument("-r", "--readers", type=int, default=3, help="Numero di lettori")
    parser.add_argument("-w", "--writers", type=int, default=3, help="Numero di scrittori")
    parser.add_argument("-v", "--valgrind", action="store_true", help="valgrind")
    args = parser.parse_args()
    mainServer(args.numMaxThreads, args.readers, args.writers, args.valgrind)
