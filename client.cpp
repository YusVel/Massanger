#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#endif
#include <winSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

#define ISVALIDSOCKET(s) ((s)!=INVALID_SOCKET)
#define GETSOCKETERRNO() (WSAGetLastError())
#define CLOSESOCKET(s) closesocket(s)

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET int
#define ISVALIDSOCKET(s) ((s)>=0)
#define GETSOCKETERRNO() (errno)
#define CLOSESOCKET(s) close(s)
#endif

#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif

#include <stdio.h>
#include <string.h>
#define MSGSIZE 1024

int main()
{
#if defined(_WIN32)
	WSADATA d;
	if(WSAStartup(MAKEWORD(2,2),&d))
	{
		fprintf(stderr,"Faild to initialize adapters!!!(%d)\n",GETSOCKETERRNO());
		return 1;
	}
#endif
printf("Configuring client ...");
SOCKET client_sock = socket(AF_INET,SOCK_STREAM,0);
if(!ISVALIDSOCKET(client_sock))
{
	fprintf(stderr,"Faild  socket()!!!(%d)\n",GETSOCKETERRNO());
	return 1;
}
struct sockaddr_in serveraddr;
memset(&serveraddr,0,sizeof(serveraddr));
serveraddr.sin_family = AF_INET;
serveraddr.sin_port = htons(8080);

if (inet_pton(AF_INET, "127.0.0.1", (sockaddr*)&serveraddr) < 1)
{
	fprintf(stderr, "Faild  inet_pton()!!!(%d)\n", GETSOCKETERRNO());
	return 1;
}


if(connect(client_sock,(sockaddr*)&serveraddr,sizeof(serveraddr))==-1)
{
	fprintf(stderr,"Faild  connect()!!!(%d)\n",GETSOCKETERRNO());
	return 1;
}
printf("Connected!\n");
char massage[MSGSIZE] = {'\0'};

#if defined(_WIN32)
sprintf_s(massage, "Hello world!");
#else
sprintf(massage, "Hello world!");
#endif

int send_bytes = send(client_sock,massage,strlen(massage),0);
printf("Send: %s (%d bytes)\n",massage,send_bytes);
memset(massage,0,strlen(massage));
int recv_bytes  = recv(client_sock,massage,MSGSIZE,0);
printf("Recive: %s (%d bytes)\n",massage,recv_bytes);

memset(massage,0,strlen(massage));
recv_bytes  = recv(client_sock,massage,MSGSIZE,0);
printf("Recive: %s (%d bytes)\n",massage,recv_bytes);

	return 0;
}
