#if defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winSock2.h>
#include <ws2tcpip.h>
#include <conio.h>
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
#include <stdlib.h>

#define SOCKET int
#define ISVALIDSOCKET(s) ((s)>=0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#endif

#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif


#include <stdio.h>
#include <time.h>
#include <string.h>
#define MSGSIZE 1024
#define ADDRLEN 128
#define PORTLEN 24
#define ERRORLEN 256


extern int get_yourIP(char* address) 
{
	
	char routeraddress[ADDRLEN] = "8.8.8.8";
	int routerport = 53;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (!ISVALIDSOCKET(sock))
	{
		fprintf(stderr, "Creating socket FAILED! ERROR getting IP! (%d)\n", GETSOCKETERRNO());
#if defined (_WIN32)
		char error_msg[ERRORLEN] = { 0 };
		strerror_s(error_msg, ERRORLEN, GETSOCKETERRNO());
		fprintf(stderr, "MASSAGE: %s\n", error_msg);
#else
		fprintf(stderr, "MASSAGE: %s", strerror(GETSOCKETERRNO()));
#endif

		exit(1);
	}

	struct sockaddr_in routeraddr;
	memset(&routeraddr, 0, sizeof(routeraddr));
	routeraddr.sin_family = AF_INET;
	routeraddr.sin_port = htons(routerport);
	inet_pton(AF_INET, routeraddress, &routeraddr.sin_addr.s_addr);
	int err = connect(sock, (const struct sockaddr*)&routeraddr, sizeof(routeraddr));
	if (err < 0)
	{
		fprintf(stderr, "NETWORK diabled! (%d)\n", GETSOCKETERRNO());
#if defined (_WIN32)
		char error_msg[ERRORLEN] = { 0 };
		strerror_s(error_msg, ERRORLEN, GETSOCKETERRNO());
		fprintf(stderr, "MASSAGE: %s\n", error_msg);
#else
		fprintf(stderr, "MASSAGE: %s\n", strerror(GETSOCKETERRNO()));
#endif
		sprintf(address,"::ffff:127.0.0.1");
		return 0;
		//exit(1);
	}
	
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	socklen_t my_addrlen = sizeof(my_addr);
	err = getsockname(sock, (struct sockaddr*)&my_addr, &my_addrlen);
	if (inet_ntop(AF_INET, &my_addr.sin_addr, address, ADDRLEN) != NULL)
	{
		memmove(address + 7, address, strlen(address) + 1);
		memcpy(address, "::ffff:", strlen("::ffff:"));
		printf("Local address: %s\n", address);
	}
	else
	{
		fprintf(stderr, "inet_ntop() FAILED! ERROR getting IP! (%d)\n", GETSOCKETERRNO());
#if defined (_WIN32)
		char error_msg[ERRORLEN] = { 0 };
		strerror_s(error_msg, ERRORLEN, GETSOCKETERRNO());
		fprintf(stderr, "MASSAGE: %s\n", error_msg);
#else
		fprintf(stderr, "MASSAGE: %s", strerror(GETSOCKETERRNO()));
#endif
		exit(1);
	}
	CLOSESOCKET(sock);
	return 0;
}