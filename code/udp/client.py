import socket
 
size = 8192
 
 

try:
 
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  for i in range(51):
    msg = raw_input()
    msg= str(i) +"_"+msg
    sock.sendto(msg, ('localhost', 9876))
    print sock.recv(size)
  sock.close()
 
except:
  print "cannot reach the server"
