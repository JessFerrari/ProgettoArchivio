#! /usr/bin/env python3
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

def client2(file_path) : 
    #viene aperto il file da cui si vuole leggere
    with open(file_path, 'rb') as file:
        #viene creato un socket TCP
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #viene assegnato un indirizzo ip e un numero di porta
        server_address = ('localhost', 55531)
        #connessione
        client_socket.connect(server_address)
        #viene identificato il client inviando 2
        client_socket.sendall(("2").encode())


        #vengono lette tutte le linee del file
        lines = file.readlines()
        #variabile per mantenere la lunghezza della sequenza
        length = 0

        #vengono inviate le linee lette al server
        for line in lines :
            #vengono rimossi gli spazi bianchi
            line = line.strip()
            if len(line) > 2048:
                print("[Client 2] linea troppo lunga")
                continue
            elif line : 
                client_socket.sendall(struct.pack('!i', len(line)) )
                client_socket.sendall(line.encode())
            else:
                print("[Client 2] linea vuota")
                continue

            #viene inviata una sequenza lunga 0 per indicare che non ci sono altre sequenze da inviare
            client_socket.sendall(b'\x00\x00\x00\x00')

            #viene ricevuto dal server il numero di sequenze inviate
            length_byte = client_socket.recv(4)
            num_seq = struct.unpack('!i', length_byte[:4])[0]
            print(f"[CLIENT2] Totale sequenze ricevute dal server -> {num_seq}")
            
            #si chiude la connessione
            client_socket.close()
        
if __name__ == "__main__":
    import sys

    #si controlla che ci sia almeno il nome di un file come argomento
    if len(sys.argv) < 2:
        print("Usage: python3 client2.py <file1> ... <fileN>")
        sys.exit(1)
    
    #si ottengono i nomi dei file dalla linea di comando e si fa partire un thread per ogni file
    threads = []
    for file in sys.argv[1:]:
        t = threading.Thread(target=client2, args=(file,))
        threads.append(t)
        t.start()
    
    #si attende che tutti i thread terminino
    for t in threads:
        t.join()
    
    print("[Client 2] terminato")