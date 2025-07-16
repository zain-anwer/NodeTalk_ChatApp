CC=gcc 

server: server.o socket_utils.o data_structs.o
	$(CC) server.o socket_utils.o data_structs.o -o server

client: client.o socket_utils.o crypto_utils.o 
	$(CC) client.o socket_utils.o crypto_utils.o -o client

clean:
	rm -f *.o
