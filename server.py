#! /usr/bin/env python3
import socket, sys, argparse, subprocess, time, signal, os, errno, struct, logging
from concurrent.futures import ThreadPoolExecutor

#inizializzo la socket del server 
server_socket = None
#variabile che conterrà il processo archivio
archivio_subprocess = None

#gestione dei due tipi di connessioni
def handle_client_connection(client_socket, client_address, caposc, capolet):
    """
    Funzione che gestisce le connessioni dei client.
    Se è un client di tipo 1 instauro una connessione per ogni riga e la invio al processo archivio sulla pipe capolet.
    Se è un client di tipo 2 instauro una connessione per ogni file e lo invio al processo archivio sulla pipe caposc.
        client_socket : socket del client
        client_address : indirizzo del client
        capolet : pipe capolet
        caposc : pipe caposc

    """
    # devo vedere se mi arriva un client di tipo 1 o di tipo 2
    client_type = client_socket.recv(1).decode('utf-8')
    #print(f"[SERVER] {client_address} : client di tipo {client_type}\n")
    # se la connessione è di tipo 1 devo instaurare una connessione per ogni linea letta dal file
    if client_type == "1":
        print(f"[SERVER] {client_address} : client di tipo 1 - connessione di tipo A\n")
        
        # mantengo il numero di byte che il server deve mandare nella pipe capolet
        num_bytes_to_send = 0
        # variabile per la lunghezza della stringa
        len_seq = 0
        
        while True:
            print(f"[SERVER] ricevuta connessione di tipo A: {client_address}\n")
            
            # Ricevo la lunghezza della stringa in bytes
            len_seq_in_bytes = recv_all(client_socket, 4)
            #se la stringa è vuota esco dal while
            if len_seq_in_bytes == 0:
                break
            len_seq = struct.unpack('!i', len_seq_in_bytes[:4])[0]
            print(f"[SERVER] {client_address} lunghezza della stringa da leggere: {len_seq}\n")
            if not len_seq:
                break
            len_seq_in_bytes = struct.pack('<i', len_seq)
            
            # Ricevo la stringa in bytes
            seq_in_bytes = recv_all(client_socket, len_seq)
            print(f"{client_address} ricevuta la stringa in bytes: {seq_in_bytes}")
            if not seq_in_bytes:
                break
            # decodifico la stringa
            seq = seq_in_bytes.decode('utf-8')
            print(f"[SERVER] {client_address} decodificata la stringa: {seq}")
            seq_in_bytes = struct.pack('<i', len(seq))

            print(f"[SERVER] len_seq -> {len_seq}, seq -> {seq}")
            print(f"[SERVER] len_seq_in_bytes -> {len_seq_in_bytes}, seq_in_bytes -> {seq_in_bytes}\n")
            
            num_bytes_to_send += (len(len_seq_in_bytes) + len(seq_in_bytes))

            # invio la stringa (prima la sua lunghezza in byte e poi la stinga in byte) al processo archivio sulla pipe capolet
            os.write(capolet, len_seq_in_bytes)
            os.write(capolet, seq_in_bytes)
           
            print(f"[SERVER] Sulla pipe capolet sono stati inviati {num_bytes_to_send} bytes in totale\n")
            
        #salvare su server log il numero di byte mandati
        logging.info("connessione di tipo A: scritti %d bytes in capolet", num_bytes_to_send)
        #chiudo la connessione 
        client_socket.close()
    #client di tipo 2
    elif client_type == "2":
        # instaura una connessione per ogni file che riceve dalla linea di comando
        print(f"[SERVER] {client_address} : client di tipo 2 - connessione di tipo B")
        # mantengo il numero totale di byte che il server deve mandare nella pipe caposc
        num_bytes_to_send = 0
        # variabile per il  numero sìdi sequenze ricevute
        tot_seq = 0
        while True:
            print(f"ricevuta connessione di tipo B: {client_address}")
            # Ricevo la lunghezza della stringa
            len_seq_in_bytes = recv_all(client_socket, 4)
            print(f"{client_address} ricevuta la lunghezza della stringa : {len_seq_in_bytes}")
            # Se ricevo 4 bytes di 0 allora ho finito di mandare sequenze
            # mando il numero sequenze ricevute
            if len_seq_in_bytes == b'\x00\x00\x00\x00':
               client_socket.sendall(struct.pack('!i', tot_seq))
               break
            len_seq = struct.unpack('!i', len_seq_in_bytes[:4])[0]
            print(f"{client_address} unpacked lunghezza della stringa {len_seq}")
            if not len_seq:
               break
       
            # Ricevo la stringa in bytes
            seq_in_bytes = recv_all(client_socket, len_seq)
            print(f"{client_address} ricevuta la stringa {seq_in_bytes}")
            if not seq_in_bytes:
               break

            # decodifico la stringa
            seq = seq_in_bytes.decode('utf-8')

            print(f"{client_address} decodificata la stringa {seq}")

            print(f"[SERVER] len -> {len_seq_in_bytes}, string -> {seq}")
            # invio la stringa (prima la sua lunghezza in byte e poi la stinga in byte) al processo archivio sulla pipe capolet
            os.write(caposc, len_seq_in_bytes)
            os.write(caposc, seq_in_bytes)
            tot_seq += 1
            num_bytes_to_send += len(len_seq_in_bytes) + len(seq_in_bytes)
            print("qUI CI ARRVIVO")
        print("qui non ci arrivo")
        #stampo il numero totale di byte mandati
        print(f"\n [SERVER] connessione di tipo 2 : num_bytes_to_send -> {num_bytes_to_send}")
        #salvare su server log
        logging.info("connessione di tipo B: scritti %d bytes in caposc", num_bytes_to_send)
        # resetto il numero di byte mandati a caposc
        num_bytes_to_send = 0
        # chiudo la connessione
        client_socket.close()


def recv_all(conn,n):
  """
Funzione mostrata a lezione, riceve esattamente n byte dal socket conn e li restituisce
Il tipo restituto è "bytes": una sequenza immutabile di valori 0-255
Questa funzione è analoga alla readn che abbiamo visto nel C
  conn : socket
  n : numero di byte  
  """
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 2048))
    if len(chunk) == 0:
      return 0
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks  

def shutdown_server(signum, frame):
    """
Funzione per terminare il server con il comando CTRL+C.
    signum : rappresenta il numero dell segnale.
    frame : rappresenta il frame corrente dello stack.
"""
    global server_socket, archivio_subprocess
    print("[...TERMINAZIONE SERVER...]") 
     
    # Chiude il socket del server
    server_socket.shutdown(socket.SHUT_RDWR)
    server_socket.close()
    
    # Chiudo le pipe caposc e capolet
    if os.path.exists("caposc"):
        os.unlink("caposc")
    if os.path.exists("capolet"):
        os.unlink("capolet")
    
    # Invia il segnale SIGTERM al processo archivio
    archivio_subprocess.send_signal(signal.SIGTERM)

    print("<[SERVER TERMINATO]>")
    exit()

def archivio(readers, writers):
    """
    Funzione che lancia il programma archivio passando il numero di scrittori e lettori (minimo 3)
        readers : numero di lettori
        writers : numero di scrittori
    """
    global archivio_subprocess, server_socket
    # Eseguo il programma archivio.c
    archivio_subprocess = subprocess.Popen(["./archivio", str(readers), str(writers)])
    print(f"[SERVER] Eseguo il programma archivio {archivio_subprocess.pid} con {readers} lettori e {writers} scrittori\n")

def archivio_valgrind(readers, writers):
    global server_socket, archivio_subprocess
    # Esegue il programma C passando anche valgrind
    archivio_subprocess = subprocess.Popen(["valgrind","--leak-check=full", "--show-leak-kinds=all",  "--log-file=valgrind-%p.log", "./archivio.out", str(readers), str(writers)])
    print(f"[SERVER] Ho lanciato il processo archivio {archivio_subprocess.pid} con valgrind\n")


def mainServer(thread_count, readers, writers, valgrind):
    """
    Funzione che gestisce il server.
    """ 

    global server_socket, archivo_subprocess
    host = "127.0.0.1"
    port = 55531
    # inizializzo il socket del server
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #crea un socket
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) #permette di riutilizzare il socket
    server_socket.bind((host, port)) #associa l'indirizzo alla porta
    server_socket.listen(5) #attiva il listening con 5 connessioni
    print("[SERVER] in ascolto sulla porta %d\n" % port)

    #creazione di un threadpool
    executor = ThreadPoolExecutor(max_workers=thread_count)
    print("[SERVER] threadpool creato\n")

    #inzializzazione delle pipes
    # controllo se esistono di già se no le creo
    if not os.path.exists("capolet"):
        try:
            os.mkfifo("capolet", 0o0666) 
            print("[SERVER] capolet creato\n")
        except FileExistsError:
            print("[SERVER] capolet gia esistente")

    if not os.path.exists("caposc"):
        try:
            os.mkfifo("caposc", 0o0666)
            print("[SERVER] caposc creato\n")  
        except FileExistsError:
            print("[SERVER] caposc gia esistente")
    

    #inizializzazione del processo archivio. Se valgrind = 1 esegue il programma archivio con valgrind
    if valgrind:
        archivio_valgrind(readers, writers)
    else:
        archivio(readers, writers)
    
    #apro le pipes
    try:
        capolet = os.open("capolet", os.O_WRONLY)
        caposc = os.open("caposc", os.O_WRONLY)
        print("\n[SERVER] capolet e caposc aperti in SCRITTURA\n")
    except Exception as e:
        print(f"Error: {e}")


    #configurazione del file di log
    logging.basicConfig(filename='server.log', level=logging.INFO, format='%(asctime)s - %(message)s')

    #gestione del SIGINT
    signal.signal(signal.SIGINT, shutdown_server)

    while True:
        client_socket, client_address = server_socket.accept()
        executor.submit(handle_client_connection, client_socket, client_address, caposc, capolet)
        print("[SERVER] accettato nuovo client")

if __name__ == "__main__":
    # Creazione del parser e definizione dei parametri
    parser = argparse.ArgumentParser(description='Uso: ./server.py [thread_count] [readers] [writers]')
    parser.add_argument('thread_count', type=int, help='Numero massimo di thread')
    parser.add_argument('-r', '--readers', type=int, default=3, help='Numero di lettori, escluso il capo')
    parser.add_argument('-w','--writers', type=int, default=3, help='Numero di scrittori, escluso il capo')
    parser.add_argument('-v', '--valgrind', action='store_true', help='Esegue il programma archivio con valgrind')

    args = parser.parse_args()

    mainServer(args.thread_count, args.readers, args.writers, args.valgrind)