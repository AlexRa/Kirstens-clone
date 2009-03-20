#ifndef VOICENET_H
#define VOICENET_H

#define VE_INVALID_SOCKET -1

typedef long VESocketHandle;

bool ve_create_socket_tcp(VESocketHandle& hSocket);
void ve_destroy_socket_tcp(VESocketHandle& hSocket);
bool ve_connect_socket_tcp(VESocketHandle hSocket, const char* host, int port);
int ve_send_packet_tcp(VESocketHandle hSocket, const char* sendBuffer, int size);
int ve_receive_packet_tcp(VESocketHandle hSocket, char* receiveBuffer, int size);

#endif	//	VOICENET_H
