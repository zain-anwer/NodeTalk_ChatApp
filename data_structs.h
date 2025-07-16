#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <stdio.h>
#include <string.h>

#define QUEUE_SIZE 50
#define BUFFER_SIZE 200

struct messageQueue
{
	char messages[QUEUE_SIZE][BUFFER_SIZE];
	int client_socket_fds[QUEUE_SIZE];
	int front;
	int rear;
	int numMessages;
};

void messageQueue_init(struct messageQueue* Q);
void enqueue(struct messageQueue* Q, char* message, int socket_fd);
char* dequeue(struct messageQueue* Q, int* socket_fd);

struct acceptedClientSocketFDs
{
	int value;
	struct acceptedClientSocketFDs* next;
};

void fd_list_init (struct acceptedClientSocketFDs* list);

#endif
