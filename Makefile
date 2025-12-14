CC = gcc
CFLAGS = -Wall -Wextra -g
TARGETS = server client
OBJ = common.h logger.h

all: $(TARGETS)

server: server.c logger.c $(OBJ)
	$(CC) $(CFLAGS) -o server server.c logger.c

client: client.c logger.c $(OBJ)
	$(CC) $(CFLAGS) -o client client.c logger.c

clean:
	rm -f $(TARGETS) *.o *.log

run_server: server
	./server

run_client: client
	./client 127.0.0.1

debug: CFLAGS += -DDEBUG -O0
debug: clean all

# Run with valgrind to check for memory leaks
valgrind_server: server
	valgrind --leak-check=full --show-leak-kinds=all ./server

valgrind_client: client
	valgrind --leak-check=full --show-leak-kinds=all ./client 127.0.0.1
