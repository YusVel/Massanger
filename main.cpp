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
	printf("Ready to use socket API!!!\n");


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


///////////////////////////////////////////////////////////
#if defined(_WIN32)
	WSACleanup();
#endif
}