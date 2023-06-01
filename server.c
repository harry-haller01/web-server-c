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