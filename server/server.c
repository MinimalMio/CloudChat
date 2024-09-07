// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
} Client;

Client *clients[MAX_CLIENTS];
int client_count = 0;
time_t start_time;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket != sender_socket) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void handle_client(Client *client) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    sprintf(buffer, "(%s) Joined chat.\n", client->username);
    broadcast(buffer, client->socket);

    while ((bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, "/quit") == 0) {
            sprintf(buffer, "(%s) Exited chat.\n", client->username);
            broadcast(buffer, client->socket);
            break;
        } else if (strcmp(buffer, "/list") == 0) {
            // Send list of users
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                send(client->socket, clients[i]->username, strlen(clients[i]->username), 0);
                send(client->socket, "\n", 1, 0);
            }
            pthread_mutex_unlock(&clients_mutex);
        } else {
            char message[BUFFER_SIZE];
            sprintf(message, "<%s> %s", client->username, buffer);
            broadcast(message, client->socket);
        }
    }

    close(client->socket);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket == client->socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    free(client);
}

int main(int argc, char *argv[]) {
    int port = 4000;
    int max_clients = 20;
    int opt;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int server_socket, client_socket;
    pthread_t thread;

    while ((opt = getopt(argc, argv, "p:m:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                max_clients = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-m max_clients]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    start_time = time(NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, max_clients) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d with max clients %d\n", port, max_clients);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0) {
        if (client_count >= max_clients) {
            close(client_socket);
            continue;
        }

        Client *client = (Client *)malloc(sizeof(Client));
        client->socket = client_socket;
        recv(client_socket, client->username, sizeof(client->username) - 1, 0);
        client->username[strcspn(client->username, "\n")] = '\0';

        pthread_mutex_lock(&clients_mutex);
        clients[client_count++] = client;
        pthread_mutex_unlock(&clients_mutex);

        pthread_create(&thread, NULL, (void *(*)(void *))handle_client, client);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
