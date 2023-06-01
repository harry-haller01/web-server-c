#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

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
    FILE *html_file = fopen(".index.html", "w");
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
    fprintf(html_file, "<style>\n%s\n</style>\n", read_file(".styles.css"));
    fprintf(html_file, "%s", item1);
    fprintf(html_file, "</head>\n<body>\n<h1>\n<center>\nContenido de %s\n</center>\n</h1>\n<table>\n", root_dir);
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

int open_file(FILE **fp, char **html)
{
    *fp = fopen(".index.html", "r");
    if (*fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fseek(*fp, 0, SEEK_END);
    long size = ftell(*fp);
    fseek(*fp, 0, SEEK_SET);
    *html = malloc(size + 1);
    fread(*html, 1, size, *fp);
    fclose(*fp);
    return size;
}