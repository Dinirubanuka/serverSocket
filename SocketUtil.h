//
// Created by Diniru Banuka on 2023-09-05.
//

#ifndef SERVERSOCKET_SOCKETUTIL_H
#define SERVERSOCKET_SOCKETUTIL_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
struct sockaddr_in* CreateIPv4Address(char *ip, int port);

int CreateTCPIpv4Socket();


#endif //SERVERSOCKET_SOCKETUTIL_H
