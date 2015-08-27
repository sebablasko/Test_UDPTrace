all: all_Internet all_UNIX

clean: rm_Internet rm_UNIX


# InternetUDP
all_Internet: internetUDPServer internetUDPClient
rm_Internet: rm_internetUDPServer rm_internetUDPClient

internetUDPServer: internetUDPServer.o ../ssocket/ssocket.o
	gcc -o3 internetUDPServer.o ../ssocket/ssocket.o -o internetUDPServerTesis -lpthread

rm_internetUDPServer:
	rm internetUDPServerTesis internetUDPServer.o

internetUDPClient: internetUDPClient.o ../ssocket/ssocket.o
	gcc -o3 internetUDPClient.o ../ssocket/ssocket.o -o internetUDPClientTesis

rm_internetUDPClient:
	rm internetUDPClientTesis internetUDPClient.o


# UNIXUDP
all_UNIX: unixUDPServer unixUDPClient
rm_UNIX: rm_unixUDPServer rm_unixUDPClient

unixUDPServer: unixUDPServer.o ../ssocket/ssocket.o
	gcc -o3 unixUDPServer.o ../ssocket/ssocket.o -o unixUDPServerTesis -lpthread

rm_unixUDPServer:
	rm unixUDPServerTesis unixUDPServer.o

unixUDPClient: unixUDPClient.o ../ssocket/ssocket.o
	gcc -o3 unixUDPClient.o ../ssocket/ssocket.o -o unixUDPClientTesis

rm_unixUDPClient:
	rm unixUDPClientTesis unixUDPClient.o
