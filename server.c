#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "html_utils.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int html_success = html_builder(argv[2]);
    if (html_success == 1) {
        perror("fhtml");
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket, valread, fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *html;
    FILE *fp;

    // Abrir el archivo HTML solicitado
    long size = open_file(&fp, &html);

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

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Crear una respuesta HTTP con el contenido del archivo HTML
        char response[1024];
        sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %ld\n\n%s", size, html);
        
        valread = read(new_socket, buffer, 1024);
        if (valread > 0) {
            char *button_name = strstr(buffer, "buttonName");
            if (button_name != NULL) {
                button_name += strlen("buttonName\":\"");
                char *end_quote = strchr(button_name, '\"');
                if (end_quote != NULL) {
                    *end_quote = '\0';
                    printf("Button name: %s\n", button_name);
                    // Aquí puedes almacenar el resultado en el servidor
                    // por ejemplo, escribiéndolo en un archivo
                    FILE *fp = fopen("result.txt", "a");
                    if (fp != NULL) {
                        fprintf(fp, "%s\n", button_name);
                        fclose(fp);
                    }
                }
            }
        }

        send(new_socket, response, strlen(response), 0);
        printf("HTML message sent\n");
        close(new_socket);
    }
    return 0;
}

/*int main(int argc, char *argv[]) {
    char *root_dir = argv[2];

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int max_clients = MAX_CLIENTS;
    int max_sd, activity, i, sd;
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int clients[MAX_CLIENTS];
    for (i = 0; i < MAX_CLIENTS; i++) clients[i] = 0;

    // create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = atoi(argv[1]);

    // bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Listening in port: %d\n", server_addr.sin_port);
    printf("Serving Directory: %s\n", root_dir);

    int html1 = html(root_dir);

    while (1) {
        // clear file descriptor set
        FD_ZERO(&readfds);

        // add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // add child sockets to set
        for (i = 0; i < max_clients; i++) {
            sd = clients[i];

            // add valid socket to set
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            // update max socket descriptor
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // wait for activity on sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select");
            exit(1);
        }

        // handle incoming connection request
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t client_len = sizeof(client_addr);
            if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                perror("accept");
                exit(1);
            }

            // add new client to list of clients
            for (i = 0; i < max_clients; i++) {
                if (clients[i] == 0) {
                    clients[i] = client_fd;
                    printf("New connection from %s:%d, socket fd: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
                    break;
                }
            }
        }

        // handle incoming data from client
        for (i = 0; i < max_clients; i++) {
            sd = clients[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // client disconnected
                    printf("Client disconnected, socket fd: %d\n", sd);
                    close(sd);
                    clients[i] = 0;
                } else {
                    // send message to all other clients
                    for (int j = 0; j < max_clients; j++) {
                        if (i != j && clients[j] > 0) {
                            if (write(clients[j], buffer, strlen(buffer)) < 0) {
                                perror("write");
                                exit(1);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}*/