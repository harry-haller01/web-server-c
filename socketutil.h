#ifndef SOCKETUTIL_SOCKETUTIL_H
#define SOCKETUTIL_SOCKETUTIL_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

struct sockaddr_in *createIPv4Address(char *ip, int port);

int createTCPipv4Sockets();

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccesfully;
};

struct AcceptedSocket *acceptIncomingConection(int serversocketFD);

void startAcceptingConecion(int serversocketFD);

void recibeAndPrintIncomingData(int socketFD);

#endif // SOCKETUTIL_SOCKETUTIL_H