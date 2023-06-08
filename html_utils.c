#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

char mod_str[80];

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    // Obtener el tamaño del archivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Leer el archivo en bloques
    char *buffer = malloc(size + 1);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = 0;
    while (bytes_read < size) {
        size_t bytes_to_read = size - bytes_read;
        if (bytes_to_read > 1024) {
            bytes_to_read = 1024;
        }

        size_t bytes = fread(buffer + bytes_read, 1, bytes_to_read, file);
        if (bytes == 0) {
            break;
        }

        bytes_read += bytes;
    }

    buffer[bytes_read] = '\0';

    fclose(file);

    return buffer;
}

char *permissions(int id) {
    switch (id % 8) {
        case 0:
            return "No permission";
        case 1:
            return "Execute";
        case 2:
            return "Write";
        case 3:
            return "Write and execute";
        case 4:
            return "Read";
        case 5:
            return "Read and execute";
        case 6:
            return "Read and write";
        case 7:
            return "Read, write and execute";
        default:
            return "-";
    }
}

char *time_stat(struct stat info) {
    time_t mod_time = info.st_mtime;
    struct tm *mod_tm = localtime(&mod_time);
    strftime(mod_str, 80, "%Y-%m-%d %A %H:%M:%S", mod_tm);
    return mod_str;
}

void html_builder(int client_socket, const char *root_dir) {
    char response[8192];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<!DOCTYPE html>\n<html>\n<head>\n");
    
    DIR *dir;
    struct dirent *entry;

    // abrir directorio
    dir = opendir(root_dir);
    if (!dir) {
        snprintf(response, sizeof(response),
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<!DOCTYPE html>\n<html>\n<head>\n"
            "<title>Error 403: Acceso denegado</title>\n"
            "</head>\n<body>\n"
            "<h1>Error 403: Acceso denegado</h1>\n"
            "<p>No tienes permisos suficientes para acceder al directorio especificado.</p>\n"
            "</body>\n</html>");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    // escribir encabezado HTML
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<title>Contenido de %s</title>\n", root_dir);
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<style>\n%s\n</style>\n", read_file(".styles.css"));
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<script>\nfunction sortTable(n) {\nvar table, rows, switching, i, x, y, shouldSwitch, dir, switchcount = 0;\n"
        "table = document.getElementsByTagName(\"table\")[0];\nswitching = true;\ndir = \"asc\";\nwhile (switching) {\n"
        "switching = false;\nrows = table.rows;\nfor (i = 1; i < (rows.length - 1); i++) {\n"
        "shouldSwitch = false;\nx = rows[i].getElementsByTagName(\"td\")[n];\n"
        "y = rows[i + 1].getElementsByTagName(\"td\")[n];\nif (dir == \"asc\") {\n"
        "if (x.innerHTML.toLowerCase() > y.innerHTML.toLowerCase()) {\nshouldSwitch = true;\nbreak;\n}\n"
        "} else if (dir == \"desc\") {\nif (x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) {\n"
        "shouldSwitch = true;\nbreak;\n}\n}\n}\nif (shouldSwitch) {\nrows[i].parentNode.insertBefore(rows[i + 1], rows[i]);\n"
        "switching = true;\nswitchcount ++;\n} else {\nif (switchcount == 0 && dir == \"asc\") {\n"
        "dir = \"desc\";\nswitching = true;\n}\n}\n}\n}\n</script>\n");
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "</head>\n<body>\n<h1>\n<center>\nContenido de %s\n</center>\n</h1>\n<table>\n", root_dir);
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<thead>\n<tr>\n<th id=\"nombre\" onclick=\"sortTable(0)\">Name</th>\n");
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<th id=\"peso\" onclick=\"sortTable(1)\">Size</th>\n");
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<th id=\"fecha\" onclick=\"sortTable(2)\">Date</th>\n");
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "<th id=\"permisos\" onclick=\"sortTable(3)\">Permission</th>\n</tr>\n</thead>\n<tbody>\n");

    // escribir lista de archivos y subdirectorios
    while ((entry = readdir(dir)) != NULL) {
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
        struct stat info;
        if (stat(entry_path, &info) == 0) {
            if (is_dir)
                snprintf(response + strlen(response), sizeof(response) - strlen(response),
                    "<tr>\n<td><a href=\"%s/\" class=\"dir\">%s/</a></td>\n", entry->d_name, entry->d_name);
            else
                snprintf(response + strlen(response), sizeof(response) - strlen(response),
                    "<tr>\n<td><a href=\"%s\" class=\"file\">%s</a></td>\n", entry->d_name, entry->d_name);
            snprintf(response + strlen(response), sizeof(response) - strlen(response),
                "<td>%ld bytes</td>\n", info.st_size);
            snprintf(response + strlen(response), sizeof(response) - strlen(response),
                "<td>%s</td>\n", time_stat(info));
            snprintf(response + strlen(response), sizeof(response) - strlen(response),
                "<td>%s</td>\n</tr>\n", permissions(info.st_mode & 0777));
        }
    }
    closedir(dir);

    // escribir cierre HTML
    snprintf(response + strlen(response), sizeof(response) - strlen(response),
        "</tbody>\n</table>\n</body>\n</html>");

    printf("Archivo HTML generado exitosamente.\n");

    send(client_socket, response, strlen(response), 0);
}