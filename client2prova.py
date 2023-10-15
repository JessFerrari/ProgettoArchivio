import socket, struct, threading


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

# Funzione per inviare il file al server
def send_file_to_server(file_path):
    with open(file_path, 'rb') as file:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect(('localhost', 55531))
            client_socket.sendall(b'2')  # Invia "2" al server per indicare l'invio di un file
            #variabile per la lunghezza della stringa
            length = 0
            lines = file.readlines()
            for line in lines :
                #memorizzo la lunghezza della stringa
                length = len(line)
                print(f"[CLIENT2] : line = {line}, length = {length}\n")
                #invio la lunghezza della stringa al server
                client_socket.send(struct.pack('!i', length))
                print("[CLIENT2] : inviata la lunghezza della stringa al server\n")
                #invio la stringa al server
                client_socket.sendall(line)
                print("[CLIENT2] : inviata la stringa al server\n")

            # Invia una sequenza lunga 0 per indicare la fine del file
            client_socket.sendall(b'\x00\x00\x00\x00')

            # Riceve il numero di sequenze inviate dal server
            num_sequences = recv_all(client_socket, 4)
            print(f"[CLIENT2] : Sequenze inviate per {file_path}: {num_sequences}")

        # Chiude la connessione
        client_socket.close()

# Lista dei file da inviare al server
#file_list = ['file1.txt', 'file2.txt', 'file3.txt']

# Creazione di thread per inviare ciascun file
#threads = []
#for file in file_list:
#    thread = threading.Thread(target=send_file_to_server, args=(file,))
#   threads.append(thread)
#   thread.start()

# Attendi che tutti i thread abbiano completato
#for thread in threads:
#   thread.join()

#print("Tutti i file sono stati inviati al server.")

if __name__ == "__main__":
    import sys

    #si controlla che ci sia almeno il nome di un file come argomento
    if len(sys.argv) < 2:
        print("[CLIENT2] : Usage: python3 client2.py <file1> ... <fileN>")
        sys.exit(1)
    
    #si ottengono i nomi dei file dalla linea di comando e si fa partire un thread per ogni file
    threads = []
    for file in sys.argv[1:]:
        thread = threading.Thread(target=send_file_to_server, args=(file,))
        threads.append(thread)
        thread.start()
    
    #si attende che tutti i thread terminino
    for thread in threads:
        thread.join()
    
    print("[Client 2] : terminato")