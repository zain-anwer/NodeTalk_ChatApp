#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "data_structs.h"
#include "socket_utils.h"

struct acceptedClientSocketFDs* head;
struct messageQueue* receivedMessages;

sem_t queue_mutex;
sem_t empty;
sem_t full;

void program_variables_init();
void* handle_client (void* arg);
void* send_messages (void* arg);

int main()
{
	// some elementary initializations

	program_variables_init();

	// this function creates a socket file descriptor
	
	int socket_fd = createTCPIpv4Socket();

	// this function creates a socket address that is a struct defining the port and the ip address

	struct sockaddr* address = createTCPIpv4SocketAddress("",2000);

	// this function assigns the IP address and port to the socket

	if (bind(socket_fd,address,sizeof(*address)) == 0)
		printf("Socket Bound Successfully\n");

	// the socket listens through this function for any incoming connections
	// the first parameter is the socket file descriptor
	// the second parameter is the backlog (amount of clients queued to be serviced)
	
	listen (socket_fd,10);

	// this is the loop that will accept multiple clients

	struct acceptedClientSocketFDs* tail = head;

	pthread_t sender_thread;
	pthread_create(&sender_thread,NULL,send_messages,NULL);

	while (true)
	{
		struct sockaddr clientAddress;
		socklen_t clientAddressSize = sizeof(clientAddress);
		
		int client_socket_fd = accept(socket_fd,&clientAddress,&clientAddressSize);

		tail->value = client_socket_fd;
		tail->next = malloc (sizeof (struct acceptedClientSocketFDs));
		tail = tail->next;
		tail->value = 0;
		tail->next = NULL;

		pthread_t tid;

		int* fd = (int*) malloc (sizeof(int));
		*fd = client_socket_fd;
		pthread_create(&tid,NULL,handle_client,(void*)fd);
	}
	
	shutdown(socket_fd,SHUT_RDWR);
	sem_destroy(&queue_mutex);
	sem_destroy(&empty);
	sem_destroy(&full);
	
	return 0;
}

void program_variables_init()
{
	head = malloc(sizeof(struct acceptedClientSocketFDs));
	receivedMessages = malloc(sizeof(struct messageQueue));

	head->value = 0;
	head->next = NULL;
	
	messageQueue_init(receivedMessages);

	sem_init(&queue_mutex,0,1);
	sem_init(&empty,0,QUEUE_SIZE);
	sem_init(&full,0,0);
}


void* handle_client (void* arg)
{
	int client_socket_fd = *((int*)arg);
	free(arg);

	while (true)
	{
		char* buffer = (char*) malloc (BUFFER_SIZE * sizeof(char));
		memset(buffer,0,BUFFER_SIZE);

	
		ssize_t bytes_read = read(client_socket_fd,buffer,BUFFER_SIZE);

		if (bytes_read <= 0)
			break;

		buffer[bytes_read] = '\0';
	
		sem_wait(&empty);
		sem_wait(&queue_mutex);

		enqueue(receivedMessages,buffer,client_socket_fd);

		sem_post(&queue_mutex);
		sem_post(&full);
	}	 

	close(client_socket_fd);

	pthread_exit(0);
}

void* send_messages (void* arg)
{
	while (true)
	{
		char* buffer = (char*) malloc (BUFFER_SIZE * sizeof(char));
		memset(buffer,0,BUFFER_SIZE);
		
		sem_wait(&full);

		struct acceptedClientSocketFDs* temp = head;

		int* fd = (int*) malloc (sizeof(int));

		sem_wait(&queue_mutex);
		
		strcpy(buffer,dequeue(receivedMessages,fd));

		sem_post(&queue_mutex);

		sem_post(&empty);

		while (temp != NULL)
		{
			if (temp->value != *fd)
				write(temp->value,buffer,strlen(buffer));
			temp = temp->next;
		} 		

		free(fd);
	} 	
}
