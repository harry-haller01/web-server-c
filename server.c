#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "html_utils.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

sem_t connection_semaphore;
char directory_path[BUFFER_SIZE];

void send_file(int client_socket, const char *path)
{
    int file = open(path, O_RDONLY);
    if (file == -1)
    {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    struct stat file_stat;
    fstat(file, &file_stat);

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Length: %ld\r\n\r\n",
             file_stat.st_size);

    send(client_socket, response, strlen(response), 0);
    sendfile(client_socket, file, NULL, file_stat.st_size);

    close(file);
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    sem_post(&connection_semaphore);

    char buffer[BUFFER_SIZE];
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    char request_path[BUFFER_SIZE];
    sscanf(buffer, "GET %s HTTP/1.1", request_path);

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s%s", directory_path, request_path);

    struct stat path_stat;
    stat(full_path, &path_stat);

    if (S_ISDIR(path_stat.st_mode))
    {
        html_builder(client_socket, full_path);
    }
    else if (S_ISREG(path_stat.st_mode))
    {
        send_file(client_socket, full_path);
    }
    else
    {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    strncpy(directory_path, argv[2], sizeof(directory_path));

    // Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Vincular el socket a una dirección IP y un puerto específicos en el servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar el socket para las solicitudes entrantes de los clientes
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    sem_init(&connection_semaphore, 0, MAX_CLIENTS);

    while (1)
    {
        sem_wait(&connection_semaphore);
        int client_socket = accept(server_fd, NULL, NULL);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &client_socket);
        pthread_detach(thread);
    }

    close(server_fd);
    sem_destroy(&connection_semaphore);
    return 0;
}