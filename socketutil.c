#include "socketutil.h"

int createTCPipv4Sockets()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in *createIPv4Address(char *ip, int port)
{
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;   // famila de protocolo ipv4
    address->sin_port = htons(port); // puerto del servidor que debe star escuchando en el otro lado
    if (strlen(ip) == 0)

        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

struct AcceptedSocket *acceptIncomingConection(int serversocketFD)
{
    // aceptar coneccion entrante
    struct sockaddr_in clientaddress;

    int clientaddressSize = sizeof(struct sockaddr_in);

    // descriptor del socket del cliente
    int clientSocketFD = accept(serversocketFD, (struct sockaddr *)&clientaddress, &clientaddressSize);

    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientaddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccesfully = clientSocketFD > 0;
    if (!acceptedSocket->acceptedSuccesfully)
    {
        acceptedSocket->error = clientSocketFD;
    }

    return acceptedSocket;
}
