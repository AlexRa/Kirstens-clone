#include "voicenet.h"
#include <windows.h>

bool ve_create_socket_tcp(VESocketHandle& hSocket)
{
    hSocket = (VESocketHandle)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hSocket == INVALID_SOCKET) hSocket = VE_INVALID_SOCKET;
    return (hSocket != VE_INVALID_SOCKET);
}

void ve_destroy_socket_tcp(VESocketHandle& hSocket)
{
    if (hSocket == VE_INVALID_SOCKET)
        return;

    closesocket((SOCKET)hSocket);
    hSocket = VE_INVALID_SOCKET;
}

bool ve_connect_socket_tcp(VESocketHandle hSocket, const char* host, int port)
{
    sockaddr_in saddr; 
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(host);
    saddr.sin_port = htons(port);
    return (connect((SOCKET)hSocket, (SOCKADDR*)&saddr, sizeof(saddr)) == 0);
}

int ve_send_packet_tcp(VESocketHandle hSocket, const char* sendBuffer, int size)
{
    return send((SOCKET)hSocket, sendBuffer, size, 0);
}

int ve_receive_packet_tcp(VESocketHandle hSocket, char* receiveBuffer, int size)
{
    return recv((SOCKET)hSocket, receiveBuffer, size, 0);
}
