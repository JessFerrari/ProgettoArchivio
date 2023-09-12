#! /usr/bin/env python3
import socket, sys, argparse, subprocess, time, signal, os, errno, struct, logging
from concurrent.futures import ThreadPoolExecutor

# inizializzo la socket del server 
server_socket = None

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
    # se la connessione è di tipo 1 devo instaurare una connessione per ogni linea letta dal file
    if client_type == "1":
        print(f"[SERVER] {client_address} : client di tipo 1")
        # mantengo il numero di byte che il server deve mandare nella pipe capolet
        num_bytes_to_send = 0
        # variabile per la lunghezza della stringa
        string_len = 0
        while True:
            # Ricevo la lunghezza della stringa
            bytes_string_len = recv_all(client_socket, 4)
            if bytes_string_len == 0:
                break
            string_len = struct.unpack('!i', bytes_string_len[:4])[0]
            if not string_len:
                break
            # Ricevo la stringa in bytes
            bytes_string = recv_all(client_socket, string_len)
            if not bytes_string:
                break
            # decodifico la stringa
            string = bytes_string.decode('utf-8')
            # mantengo la lunghezza in bytes
            #num_bytes_to_send = struct.pack('!i', string_len)
            print(f"[SERVER] len -> {string_len}, string -> {string}")
            # invio la stringa (prima la sua lunghezza in byte e poi la stinga in byte) al processo archivio sulla pipe capolet
            os.write(capolet, bytes_string_len)
            os.write(capolet, bytes_string)

        # uscita dal while devo salvare su server log
        #chiudo la connessione 
        client_socket.close()
    #client di tipo 2
    elif client_type == "2":
       # instaura una connessione per ogni file che riceve dalla linea di comando
       print(f"[SERVER] {client_address} : client di tipo 2")
       # mantengo il numero totale di byte che il server deve mandare nella pipe caposc
       num_bytes_to_send = 0
       # variabile per il  numero sìdi sequenze ricevute
       tot_seq = 0
       while True:
           # Ricevo la lunghezza della stringa
           bytes_string_len = recv_all(client_socket, 4)
           # Se ricevo 4 bytes di 0 allora ho finito di mandare sequenze
           # mando il numero sequenze ricevute
           if bytes_string_len == b'\x00\x00\x00\x00':
               client_socket.sendall(struct.pack('!i', tot_seq))
               break
           string_len = struct.unpack('!i', bytes_string_len[:4])[0]
           if not string_len:
               break
           # Ricevo la stringa in bytes
           bytes_string = recv_all(client_socket, string_len)
           if not bytes_string:
               break
           # decodifico la stringa
           string = bytes_string.decode('utf-8')
           
           print(f"[SERVER] len -> {bytes_string_len}, string -> {string}")
           # invio la stringa (prima la sua lunghezza in byte e poi la stinga in byte) al processo archivio sulla pipe capolet
           os.write(caposc, bytes_string_len)
           os.write(caposc, bytes_string)
           tot_seq += 1
           num_bytes_to_send += len(bytes_string_len) + len(bytes_string)

       # uscita dal while devo salvare su server log
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
    print(f"[SERVER] Eseguo il programma archivio con {readers} lettori e {writers} scrittori")