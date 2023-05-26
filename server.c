#include "socketutil.c"
void reciveAndPrintIncomingDataSeparateThread(struct AcceptedSocket *clientSockets);
int main()
{
    // servidor
    // Crear el socket
    int serversocketFD = createTCPipv4Sockets();

    // crear la estructura del servidor para escuchar solicitudes
    struct sockaddr_in *serveraddress = createIPv4Address("", 2000);

    // vincular el servidor a la direccion
    int result = bind(serversocketFD, (struct sockaddr *)serveraddress, sizeof(*serveraddress));
    if (result < 0)
    {
        perror("Error al vincular los sockets del servidor");
        exit(1);
    }
    else
    {
        printf("Sockets del servidor vinculado correctamente \n");
    }

    // comenzar a escuchar lo sockets(conexiones) entarntes
    int listenresult = listen(serversocketFD, 10);
    if (listenresult < 0)
    {
        perror("Error al comenzar a escuchar los sockets entrantes");
        exit(1);
    }
    else
    {
        printf("Servidor esperando conexiones entrantes...\n");
    }

    // comenzar a aceptar conecciones(separados por subprocesos)
    startAcceptingConecion(serversocketFD);

    // cerrar los File Descriptor
    shutdown(serversocketFD, SHUT_RDWR);
    return 0;
}

void startAcceptingConecion(int serversocketFD)
{
    // para cada conexxion entrante acceptada crear un subproceso
    while (true)
    {
        // estructura para almacenar datos de la coneccion entrante
        struct AcceptedSocket *clientSockets = acceptIncomingConection(serversocketFD);
        if (clientSockets->acceptedSocketFD < 0)
        {
            perror("Error al aceptar la conexión entrante");
            exit(1);
        }
        else
        {
            printf("Conexión aceptada desde %s:%d\n", inet_ntoa(clientSockets->address.sin_addr), ntohs(clientSockets->address.sin_port));
        }

        reciveAndPrintIncomingDataSeparateThread(clientSockets);
    }
}

void reciveAndPrintIncomingDataSeparateThread(struct AcceptedSocket *clientSockets)
{
    // recibir respuesta del cliente en un subproceso separado
    pthread_t id;
    pthread_create(&id, NULL, (void *)recibeAndPrintIncomingData, clientSockets->acceptedSocketFD);
}

void recibeAndPrintIncomingData(int socketFD)
{
    char buffer[1024];
    while (true)
    {
        ssize_t ammountRecived = recv(socketFD, buffer, 1024, 0);
        if (ammountRecived > 0)
        {
            buffer[ammountRecived] = 0;
            printf("La respuesta es: %s", buffer);
        }
        if (ammountRecived == 0)
        {
            break;
        }
    }
    close(socketFD);
}
