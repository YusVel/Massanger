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
#include <math.h>
#include <time.h>
#include <string.h>
#define MSGSIZE 1024
#define ADDRLEN 128
#define PORTLEN 24
#define ERRORLEN 256
#define MAXCLIENTS 10


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
		sprintf_s(address, ADDRLEN, "::ffff:127.0.0.1");
#else
		fprintf(stderr, "MASSAGE: %s\n", strerror(GETSOCKETERRNO()));
		sprintf(address, "::ffff:127.0.0.1");
#endif
		//sprintf(address,"::ffff:127.0.0.1");
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
		printf("***** Local address: %s\n", address);
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

void show_error(int num)
{
#if defined (_WIN32)
		char error_msg[ERRORLEN] = { 0 };
		strerror_s(error_msg, ERRORLEN, num);
		fprintf(stderr, "\nMASSAGE: %s\n", error_msg);
#else
		fprintf(stderr, "\nMASSAGE: %s", strerror(num));
#endif
}

void del_client_from_arr(SOCKET* arr, int size, SOCKET client)
{
	int i = 0;
	int no_matched = 1;
	for (; i < size; i++)
	{
		if (arr[i] == client)
		{
			arr[i] = 0;
			no_matched = 0;
			break;
		}
	}
	if (i != size)
	{
		memmove(&arr[i], &arr[i + 1], size * sizeof(SOCKET) - (i * sizeof(SOCKET)) - sizeof(SOCKET));
		arr[size - 1] = 0;
		printf("***** Client(socket=%d) deleted \n", client);
	}
	if (no_matched)
	{
		printf("***** Client(%d) is absent in the arr\n", client);
	}

}

int add_client_to_arr(SOCKET* arr, int size, SOCKET client)
{
	int i = 0;
	int filled_arr = 1;
	for (; i < size; i++)
	{
		if (arr[i] == 0)
		{
			arr[i] = client;
			printf("***** NEW client(socket=%d) added \n", client);
			filled_arr = 0;
			break;
		}
	}
	if (filled_arr)
	{
		printf("***** Clients storage is filled!!! Adding new client is impossible *****\n");
		return 1;
	}
	return 0;
}

void show_arr(SOCKET* arr, int size)
{
	for (int i = 0; i < size; i++)
	{
		printf("%d, ", arr[i]);
	}
	printf("\n");
}

int is_valid_double(char* msg,int recv_bytes)
{

	printf("------%.*s------\n",recv_bytes,msg);
	int points = 0;
	int sign = 0;
	for (int i = 0; i<recv_bytes-1; i++)
	{
		if (i > 0 && points == 0 && (msg[i] == ',' || msg[i] == '.'))
		{
			if (msg[i] == ',')
			{
				msg[i] == '.';
			}
			points++;
			continue;
		}
		if (msg[i] == '-' && points == 0)
		{
			continue;
		}
		if (msg[i] > 57 || msg[i] < 48)
		{
			return 0;
		}
	}
	return 1;
}

int is_valid_action(char* msg, int recv_bytes)
{
	if (recv_bytes == 1)
	{
		if (msg[0] == 42 || msg[0] == 43 || msg[0] == 45 || msg[0] == 47)
		{
			//правильное математическое действие
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


double calculate(double x, double y, char msg)
{
	switch (msg)
	{
	case '/':return x / y; break;
	case '*':return x * y; break;
	case '-':return x - y; break;
	case '+':return x + y; break;
	default:return 999999.9999; break;
	}
}

void send_to_clients(SOCKET *arr,char *massage, int fd)
{
	for (int j = 0; j < MAXCLIENTS; j++)
	{
		if(arr[j]==fd){continue;};
		if (arr[j] != 0)
		{
			char adr[ADDRLEN] = { 0 };
			char client_name[ADDRLEN] = { 0 };	
			struct sockaddr_storage post_client;
			memset(&post_client,0,sizeof(post_client));
			socklen_t post_client_len = sizeof(post_client);
	
			if(getpeername(arr[j],(struct sockaddr*)&post_client,&post_client_len)!=0)
			{
				fprintf(stderr,"##### getperrname().send_to_clients() faild(%d) #####",GETSOCKETERRNO());
				show_error(GETSOCKETERRNO());
			}
			if(getnameinfo((struct sockaddr*)&post_client,post_client_len,adr,sizeof(adr),0,0,NI_NUMERICHOST)!=0)
			{
				fprintf(stderr,"##### getnameinfo() send_to_clients(). faild(%d) #####",GETSOCKETERRNO());
				show_error(GETSOCKETERRNO());
			}
			if (getnameinfo((struct sockaddr*)&post_client, post_client_len, client_name, sizeof(client_name), 0, 0, 1) != 0)
			{
				fprintf(stderr, "##### getnameinfo() send_to_clients() faild(%d) #####", GETSOCKETERRNO());
				show_error(GETSOCKETERRNO());
			}		
			
			int send_bytes = send(arr[j], massage,strlen(massage), 0);
			if(send_bytes <1)
			{
				fprintf(stderr, "##### send() to socket(%d) faild(%d) #####", (int)arr[j], GETSOCKETERRNO());
				show_error(GETSOCKETERRNO());
			}
			printf("%d) SEND(%d bytes) to (socket=%d) %s (%s): %.*s",j+1, send_bytes, (int)arr[j], client_name, adr, send_bytes, massage);
		}
	}
}