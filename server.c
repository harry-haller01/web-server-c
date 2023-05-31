#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int html_builder(const char *root_dir) {
    DIR *dir;
    struct dirent *entry;

    // abrir directorio
    dir = opendir(root_dir);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    // crear archivo HTML
    FILE *html_file = fopen("index.html", "w");
    if (!html_file) {
        perror("fopen");
        return 1;
    }

    char item1[2048] = "<script>\nfunction sortTable(n) {\nvar table, rows, switching, i, x, y, shouldSwitch, dir, switchcount = 0;\n";
    char item2[] = "table = document.getElementsByTagName(\"table\")[0];\nswitching = true;\ndir = \"asc\";\nwhile (switching) {\n";
    char item3[] = "switching = false;\nrows = table.rows;\nfor (i = 1; i < (rows.length - 1); i++) {\n";
    char item4[] = "shouldSwitch = false;\nx = rows[i].getElementsByTagName(\"td\")[n];\n";
    char item5[] = "y = rows[i + 1].getElementsByTagName(\"td\")[n];\nif (dir == \"asc\") {\n";
    char item6[] = "if (x.innerHTML.toLowerCase() > y.innerHTML.toLowerCase()) {\nshouldSwitch = true;\nbreak;\n}\n";
    char item7[] = "} else if (dir == \"desc\") {\nif (x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) {\n";
    char item8[] = "shouldSwitch = true;\nbreak;\n}\n}\n}\nif (shouldSwitch) {\nrows[i].parentNode.insertBefore(rows[i + 1], rows[i]);\n";
    char item9[] = "switching = true;\nswitchcount ++;\n} else {\nif (switchcount == 0 && dir == \"asc\") {\n";
    char item10[] = "dir = \"desc\";\nswitching = true;\n}\n}\n}\n}\n</script>\n";
    strcat(item1, item2);
    strcat(item1, item3);
    strcat(item1, item4);
    strcat(item1, item5);
    strcat(item1, item6);
    strcat(item1, item7);
    strcat(item1, item8);
    strcat(item1, item9);
    strcat(item1, item10);

    // escribir encabezado HTML
    fprintf(html_file, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(html_file, "<title>Contenido de %s</title>\n", root_dir);
    fprintf(html_file, "%s", item1);
    fprintf(html_file, "</head>\n<body>\n<h1>Contenido de %s</h1>\n<table>\n", root_dir);
    fprintf(html_file, "<thead>\n<tr>\n<th id=\"nombre\" onclick=\"sortTable(0)\">Name</th>\n");
    fprintf(html_file, "<th id=\"peso\" onclick=\"sortTable(1)\">Size</th>\n");
    fprintf(html_file, "<th id=\"fecha\" onclick=\"sortTable(2)\">Date</th>\n</tr>\n</thead>\n<tbody>\n");
    
    //fprintf(html_file, "<li><a href=\"%s\" download>../</a></li>\n", last_slash);

    // escribir lista de archivos y subdirectorios
    while ((entry = readdir(dir)) != NULL) {
        // omitir archivos y directorios ocultos
        if (entry->d_name[0] == '.') {
            continue;
        }

        // obtener ruta completa del archivo o directorio
        char entry_path[1024];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", root_dir, entry->d_name);

        // determinar si es un archivo o un directorio
        int is_dir = 0;
        DIR *subdir = opendir(entry_path);
        if (subdir) {
            is_dir = 1;
            closedir(subdir);
        }

        // escribir elemento de lista en el archivo HTML
        if (is_dir) {
            struct stat info;

            if (stat(entry_path, &info) == 0) {
                fprintf(html_file, "<tr>\n<td><a href=\"%s/\">%s/</a></td>\n", entry_path, entry->d_name);
                fprintf(html_file, "<td>%ld bytes</td>\n", info.st_size);
                fprintf(html_file, "<td>%s</td>\n</tr>\n", ctime(&info.st_mtime));
            }
        }
        else {
            struct stat info;

            if (stat(entry_path, &info) == 0) {
                fprintf(html_file, "<tr>\n<td><a href=\"%s\" download>%s</a></td>\n", entry_path, entry->d_name);
                fprintf(html_file, "<td>%ld bytes</td>\n", info.st_size);
                fprintf(html_file, "<td>%s</td>\n</tr>\n", ctime(&info.st_mtime));
            }
        }
    }

    // escribir cierre HTML
    fprintf(html_file, "</tbody>\n</table>\n</body>\n</html>");

    // cerrar archivos y directorios
    closedir(dir);
    fclose(html_file);

    printf("Archivo HTML generado exitosamente.\n");

    return 0;
}

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
    fp = fopen("index.html", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    html = malloc(size + 1);
    fread(html, 1, size, fp);
    fclose(fp);

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