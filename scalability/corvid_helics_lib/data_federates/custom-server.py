import socket
import sys

HOST = "127.0.0.1"
PORT = 23555

def main(argv):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()

        print(f"Connecting on {HOST}:{PORT}...")

        conn, addr = s.accept()
        with conn:
            print(f"Connected to {addr}")
            while True:
                data = conn.recv(4096)
                if not data:
                    break

                data_string = data.decode('utf-8')
                print(data_string)

if __name__ == "__main__":
    main(sys.argv)