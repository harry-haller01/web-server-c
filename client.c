#include "socketutil.c"

int main()
{
    // cliente
    // Crear el socket
    int socketFD = createTCPipv4Sockets();

    // crear la estructura  del servidor y agregarle las proiedades localhost:127.0.0.1 - ip de gogle:142.250.188.46
    struct sockaddr_in *address = createIPv4Address("127.0.0.1 ", 2000);

    // para conectarce al servidor
    int result = connect(socketFD, (struct sockaddr *)address, sizeof(*address));
    if (result == 0)
    {
        printf("Conexión establecida \n");
    }
    else
    {
        perror("Error al establecer la conexión");
        exit(1);
    }

    printf("type and we will send(type exit)...\n");
    char *line = NULL;
    size_t linesize = 0;
    while (true)
    {
        ssize_t charcount = getline(&line, &linesize, stdin);
        if (charcount > 0)
        {
            if (strcmp(line, "exit\n") == 0)
                break;
            ssize_t connectWasSent = send(socketFD, line, charcount, 0);
        }
    }

    close(socketFD);

    return 0;
}
