CC = gcc
CFLAGS = -Wall -Wextra -g
TARGETS = server client
OBJ = common.h

all: $(TARGETS)

server: server.c $(OBJ)
	$(CC) $(CFLAGS) -o server server.c

client: client.c $(OBJ)
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f $(TARGETS) *.o

run_server: server
	./server

run_client: client
	./client 127.0.0.1
