// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
} Client;

Client client;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(client.socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char *ip = NULL;
    int port = 0;
    char *username = NULL;
    int opt;
    struct sockaddr_in server_addr;
    pthread_t thread;

    while ((opt = getopt(argc, argv, "i:p:n:")) != -1) {
        switch (opt) {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'n':
                username = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -i ip -p port -n username\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!ip || !port || !username) {
        fprintf(stderr, "Usage: %s -i ip -p port -n username\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    client.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client.socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(client.socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    strncpy(client.username, username, sizeof(client.username) - 1);
    send(client.socket, client.username, strlen(client.username), 0);

    pthread_create(&thread, NULL, receive_messages, NULL);
    pthread_detach(thread);

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (strcmp(buffer, "/quit\n") == 0) {
            close(client.socket);
            break;
        }
        send(client.socket, buffer, strlen(buffer), 0);
    }

    return 0;
}
