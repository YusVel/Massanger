#include "Multiplatformheader.h"

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


////////////////////////////////////////////////////////////
printf("Configuring local server adress...\n");
struct addrinfo tips;
memset(&tips,0,sizeof(tips));
tips.ai_family = AF_INET6;
tips.ai_socktype = SOCK_STREAM;
tips.ai_flags = AI_PASSIVE; 
struct addrinfo *bind_address;
char serveraddress[64] = "::ffff:127.0.0.1";
char serverport[64] = "5000";
getaddrinfo(serveraddress, serverport, &tips, &bind_address);

if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, serveraddress, sizeof(serveraddress), serverport, sizeof(serverport), NI_NUMERICHOST))
{
	fprintf(stderr, "Error getnameinfo() (%d)", GETSOCKETERRNO());
	perror("ERROR");
	return 1;
}
printf("SERVER ADDRESS %s:%s\n", serveraddress, serverport);

printf("Creating socet...\n");
SOCKET server_sock;
server_sock = socket(bind_address->ai_family,bind_address->ai_socktype,bind_address->ai_protocol);
 if(!ISVALIDSOCKET(server_sock))
 {
	 fprintf(stderr,"Creating server socket FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 else
 {
	 printf("Server socket created!\n");
 }
 

 int option =0;
#if defined(_WIN32)
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&option, sizeof(option)))
 {
	 fprintf(stderr, "setsockopt() faild!!! (%d)", GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
#else
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option)))
 {
	 fprintf(stderr, "setsockopt() faild!!! (%d)", GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
#endif
 if(bind(server_sock,bind_address->ai_addr,bind_address->ai_addrlen))
 {
	 fprintf(stderr,"bind() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
  else
 {
	 printf("Server's address is bound!\n");
 }
 freeaddrinfo(bind_address);
 printf("Listening....\n");
 if(listen(server_sock,10)<0)
 {
	 fprintf(stderr,"linten() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 printf("Waiting for connection....\n");
 
 struct sockaddr_storage client_address;
 socklen_t client_len = sizeof(client_address); 

 SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_address, &client_len);

 
 
 if(!ISVALIDSOCKET(client_sock))
 {
	 fprintf(stderr,"Client accept() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 
  printf("Client conected....\n");
  char addres_buffer[128];
  getnameinfo((struct sockaddr*)&client_address,client_len,addres_buffer,sizeof(addres_buffer),0,0,NI_NUMERICHOST);

  printf("Massage from ");
  printf("%s\n ", addres_buffer);
  char MSG[MSGSIZE];
  memset(MSG, 0, MSGSIZE);
  int recv_bytes = recv(client_sock, MSG, MSGSIZE, 0);
  if (recv_bytes < 1)
  {
	  printf("\nError. Client disconnected!\n");
	  return 1;
  }
  printf("Recive %d bytes.",recv_bytes);


  time_t current_time;
  time(&current_time);
#if defined(_WIN32)

  ctime_s(MSG, MSGSIZE, &current_time);

#else
  sprintf(MSG,"Today is: %s\n", ctime(&current_time));
#endif

  
  const char* response =
	  "HTTP/1.1 200 OK\r\n"
	  "Connection: close\r\n"
	  "Content-Type: text/plain\r\n\r\n"
	  "Today is: ";
  int sent_bytes = send(client_sock, response, strlen(response), 0);
  printf("\nSending response....(%d bytes)\n",sent_bytes);

  sent_bytes = send(client_sock, MSG, strlen(MSG), 0);
  printf("\nSending massage (current time)....(%d bytes)\n", sent_bytes);
///////////////////////////////////////////////////////////


  CLOSESOCKET(client_sock);
  CLOSESOCKET(server_sock);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("\n****FINISH*****\n");
}
