#include "socket_init.h"

int socket_init(uint16_t port)
{
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        return 0;
    }
    int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    return sock;
}