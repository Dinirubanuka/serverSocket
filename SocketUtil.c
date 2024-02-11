#include "SocketUtil.h"

int CreateTCPIpv4Socket() { //create socket
    int socketFD = socket(AF_INET,SOCK_STREAM,0);
    if (socketFD < 0) {
        perror("Error creating socket");
        exit(1);
    }
    return socketFD;
}

struct sockaddr_in* CreateIPv4Address(char *ip, int port) {//connect to the server

    struct sockaddr_in *address = malloc(sizeof (struct sockaddr_in));//socket address
    address->sin_family = AF_INET;
    address->sin_port =  htons(port);

    if (strlen(ip) == 0){
        address->sin_addr.s_addr = INADDR_ANY;
    } else
        inet_pton(AF_INET,ip,&address->sin_addr.s_addr ); //convert internet addresss unsigned integer format

    return address;
}
