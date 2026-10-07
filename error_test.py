import socket

HOST = "127.0.0.1"
PORT = 9410

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

def send_command(command):
    print("Sent:", command)
    s.sendall((command + "\n").encode())
    response = s.recv(4096).decode()
    print("Agent response:", response.strip())

send_command("AUTH OPS-4117")
send_command("GET missing_file_117.txt")
send_command("HELLO")

s.close()
