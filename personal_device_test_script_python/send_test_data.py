import socket
import struct
TCP_IP = '192.168.0.40' 
TCP_PORT = 80      
BUFFER_SIZE = 1024
MESSAGE = b'\x05\x00\x00\x00'
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
s.send(MESSAGE)
data = s.recv(BUFFER_SIZE)
s.close()
print( "received data:", data)