import socket

size = 8192
counter = 1
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

try:
  while True:
    data, address = sock.recvfrom(size)
    sock.sendto(str(counter)+"_"+data, address)
    counter += 1
finally:
  sock.close()
