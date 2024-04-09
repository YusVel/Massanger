#if defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

#define ISVALIDSOCKET(s) ((s)!=INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

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
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>


int main()
{
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "FAILD to INITIASLIZE!!!\n");
		printf("%s", WSAGetLastError());
		return 1;
	}
#endif
	//printf("Ready to use socket API!!!\n");


	time_t current_time;
	time(&current_time);
#if defined(_WIN32)
	char buffer[64] = { '\0' };
	ctime_s(buffer, 64, &current_time);
	printf("Today is: %s\n",buffer );

#else
	printf("Today is: %s\n", ctime(&current_time));
#endif
////////////////////////////////////////////////////////////
printf("Configuring local server adress...\n");
struct addrinfo tips;
memset(&tips,0,sizeof(tips));
tips.ai_family = AF_INET;
tips.ai_socktype = SOCK_STREAM;
tips.ai_flags = AI_PASSIVE; 
struct addrinfo *bind_address;
getaddrinfo(0,"5000",&tips,&bind_address);

printf("Creating socet...\n");
SOCKET server_sock;
server_sock = socket(bind_address->ai_family,bind_address->ai_socktype,bind_address->ai_protocol);
 if(!ISVALIDSOCKET(server_sock))
 {
	 fprintf(stderr,"Creating server socket FAILED! (%d)\n",GETSOCKETERRNO());
	 return 1;
 }
 else
 {
	 printf("Server socket created!\n");
 }
 
 if(bind(server_sock,bind_address->ai_addr,bind_address->ai_addrlen))
 {
	 fprintf(stderr,"bind() FAILED! (%d)\n",GETSOCKETERRNO());
	 return 1;
 }
  else
 {
	 printf("Server's adress is bound!\n");
 }
 freeaddrinfo(bind_address);
 printf("Listening....\n");
 if(listen(server_sock,10)<0)
 {
	 fprintf(stderr,"linten() FAILED! (%d)\n",GETSOCKETERRNO());
	 return 1;
 }
 printf("Waiting for connection....\n");
 
 struct sockaddr_storage client_address;
 socklen_t client_len = sizeof(client_address); 
 
 SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_address,client_len);
 
 if(!ISVALIDSOCKET(client_sock))
 {
	 fprintf(stderr,"Client accept() FAILED! (%d)\n",GETSOCKETERRNO());
	 return 1;
 }
 
  printf("Listening....\n");
  char addres_buffer[128];
  getnameinfo((struct sockaddr*)&client_address,client_len,addres_buffer,sizeof(addres_buffer),0,0,NI_NUMERICHOST);
  printf("%s",addres_buffer);
 
///////////////////////////////////////////////////////////
#if defined(_WIN32)
	WSACleanup();
#endif
}