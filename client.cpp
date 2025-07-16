// client program

#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "socket_utils.h"
#include "crypto_utils.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstring>
#include <string>

#define BUFFER_SIZE 200

using namespace std;

bool program_running = true;
vector<string> chat_messages;
pthread_mutex_t client_message_mutex;
pthread_mutex_t other_message_mutex;
string client_input = "";

void* consumer_function (void* arg);
void* chat_gui (void* arg);

int main(int argc, char** argv)
{
    pthread_mutex_init(&client_message_mutex,NULL);
    pthread_mutex_init(&other_message_mutex,NULL);
    
    char ip[20] = "127.0.0.1";
    int port_num = 2000;
    char name[20];

    if (argc > 3)
    {
        strcpy(ip,argv[1]);
        port_num = atoi(argv[2]);
        strcpy(name,argv[3]);
    }

    if (argc < 4)
    {
    	printf("Enter Client Name : ");
    	scanf("%s",name);
    }

    int socket_fd = createTCPIpv4Socket();
    struct sockaddr* address = createTCPIpv4SocketAddress(ip,port_num);

    if (connect(socket_fd,address,sizeof(*address)) == 0)
        printf("Connection Successful\n");

    int fd[2];
    if (pipe(fd) < 0) {
        perror("Pipe failed");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid > 0) // Parent
    {
        close(fd[0]);

        pthread_t consumer_thread, gui_thread;
        int* socket_fd_ptr = (int*) malloc(sizeof(int));
        *socket_fd_ptr = socket_fd;

        pthread_create(&consumer_thread, NULL, consumer_function, socket_fd_ptr);
        pthread_create(&gui_thread, NULL, chat_gui, NULL);

        while (true)
        {
            char* buffer = (char*) malloc(BUFFER_SIZE);
            memset(buffer, 0, BUFFER_SIZE);

            pthread_mutex_lock(&client_message_mutex);
            string temp_input = client_input;
            client_input.clear();
            pthread_mutex_unlock(&client_message_mutex);
            
            strncpy(buffer, temp_input.c_str(), BUFFER_SIZE - 1);

			if (strlen(buffer) == 0)
			{
				free(buffer);
				continue;
			}

			char final_message[BUFFER_SIZE];
			snprintf(final_message, BUFFER_SIZE, "%s - %s", name, buffer);
			strcpy(buffer,final_message);

            if (program_running == false) 
            {
                write(fd[1], "EXIT_PROGRAM", strlen("EXIT_PROGRAM"));
                free(buffer);
                break;
            }

			if (strlen(buffer) != 0)
            	write(fd[1], buffer, strlen(buffer));
            free(buffer);
        }

        wait(NULL);
        pthread_join(gui_thread, NULL);
        pthread_join(consumer_thread, NULL);
        close(fd[1]);
        close(socket_fd);
    }
    else // Child
    {
        close(fd[1]);
        while (true)
        {
            char* buffer = (char*) malloc(BUFFER_SIZE);
            memset(buffer, 0, BUFFER_SIZE);

            ssize_t bytes_read = read(fd[0], buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                free(buffer);
                break;
            }

            buffer[bytes_read] = '\0';

            if (strncmp(buffer, "EXIT_PROGRAM", strlen("EXIT_PROGRAM")) == 0) {
                free(buffer);
                shutdown(socket_fd, SHUT_RDWR);
                break;
            }

            encrypt(buffer, KEY_VALUE);
            write(socket_fd, buffer, strlen(buffer));
            free(buffer);
        }

        close(socket_fd);
        exit(0);
    }

    return 0;
}

void* consumer_function (void* arg)
{
    int socket_fd = *((int*)arg);
    free(arg);

    while (true)
    {
        char* buffer = (char*) malloc(BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes_read = read(socket_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            free(buffer);
            break;
        }

        buffer[bytes_read] = '\0';
        decrypt(buffer, KEY_VALUE);

        pthread_mutex_lock(&other_message_mutex);
        chat_messages.push_back(string(buffer));
        pthread_mutex_unlock(&other_message_mutex);

        free(buffer);
    }

    pthread_exit(0);
}

void* chat_gui (void* arg)
{
    sf::RenderWindow window(sf::VideoMode(600, 400), "Parlons");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("Lato-Regular.ttf")) {
        fprintf(stderr, "Failed to load font file\n");
        pthread_exit(0);
    }

    sf::Text input_text;
    input_text.setFont(font);
    input_text.setCharacterSize(18);
    input_text.setFillColor(sf::Color::White);
    input_text.setPosition(10, 360);

    sf::RectangleShape message_box(sf::Vector2f(580, 30));
    message_box.setPosition(10, 355);
    message_box.setFillColor(sf::Color(50, 50, 50));

    string current_input = "";
    int i = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
			{
				program_running = false;
                window.close();
            }

            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode == '\b' && !current_input.empty())
                    current_input.pop_back();
                else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!current_input.empty()) {
                        pthread_mutex_lock(&other_message_mutex);
                        chat_messages.push_back(current_input);
                        pthread_mutex_unlock(&other_message_mutex);

                        pthread_mutex_lock(&client_message_mutex);
                        client_input = current_input;
                        pthread_mutex_unlock(&client_message_mutex);

                        current_input.clear();
                    }
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\b')
                    current_input += static_cast<char>(event.text.unicode);
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(message_box);

        int y = 0;
        pthread_mutex_lock(&other_message_mutex);
        for (const string& msg : chat_messages)
        {
            sf::Text message;
            message.setFont(font);
            message.setString(msg);
            message.setCharacterSize(18);
            message.setFillColor(sf::Color(255, 192, 203));
            message.setPosition(10, y);
            y += 25;
            if (y > 360) break;
            window.draw(message);
        }
        pthread_mutex_unlock(&other_message_mutex);

        i++;
        if (i < 30)
            input_text.setString(current_input + "|");
        else if (i < 60)
            input_text.setString(current_input);
        else
            i = 0;

        window.draw(input_text);
        window.display();
    }

    pthread_exit(0);
}


