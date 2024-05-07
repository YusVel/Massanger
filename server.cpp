#include "Multiplatformheader.h" //универсальный заголовок для WIN/Linux







int main()
{
#if defined(_WIN32) //Инициализируем структуру WSADATA, где содержится вся информация о версии сокетов под WIN
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "FAILD to INITIASLIZE!!!\n");
		printf("%d", WSAGetLastError());
		return 1;
	}
#endif



////////////////////////////////////////////////////////////
printf("Configuring local server address...\n");
struct addrinfo tips;  //Настройки адреса сервера
memset(&tips,0,sizeof(tips));
tips.ai_family = AF_INET6;
tips.ai_socktype = SOCK_STREAM;
tips.ai_flags = AI_PASSIVE; 
struct addrinfo *bind_address;
char serveraddress[ADDRLEN] = "::ffff:127.0.0.1";
char serverport[PORTLEN] = "5000";


get_yourIP(serveraddress); // Получение текужего адреса ПК


getaddrinfo(serveraddress, serverport, &tips, &bind_address); //заполнение структуры addrino bind_address с учетом внесенных настроек tips

if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, serveraddress, ADDRLEN, serverport, PORTLEN, NI_NUMERICHOST)) //получаем адрес из сокета
{
	fprintf(stderr, "Error getnameinfo() (%d)", GETSOCKETERRNO());
	show_error(GETSOCKETERRNO());
	return 1;
}
printf("SERVER ADDRESS %s:%s\n", serveraddress, serverport);

printf("Creating socet...\n");
SOCKET server_sock;
server_sock = socket(bind_address->ai_family,bind_address->ai_socktype,bind_address->ai_protocol); //создаем серверный сокет, через который сервер будет общатся с клиентами
 if(!ISVALIDSOCKET(server_sock))
 {
	 fprintf(stderr,"Creating server socket FAILED! (%d)\n",GETSOCKETERRNO());
	 show_error(GETSOCKETERRNO());
	 return 1;
 }
 else
 {
	 printf("Server socket is created!\n");
 }
 

 int option =0;
#if defined(_WIN32)
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&option, sizeof(option))) //функция изеняющая настрой ки сокета, для приема клиентов с адресами как IPV4  так и IPV^6
 {
	 fprintf(stderr, "setsockopt() faild!!! (%d)", GETSOCKETERRNO());
	 show_error(GETSOCKETERRNO());
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
 if(bind(server_sock,bind_address->ai_addr,bind_address->ai_addrlen)) // настраиваем сокет, присваиваем ему адрес
 {
	 fprintf(stderr,"bind() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
  else
 {
	 printf("Server's address is bound!\n");
 }
 freeaddrinfo(bind_address); // высвобождаем память занятую структурой адреса 
 printf("Listening....\n");
 if(listen(server_sock,10)<0)
 {
	 fprintf(stderr,"linten() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 printf("Waiting for connection....\n");
 
 //////////////////////////
 /*
 fd_set sockets_set;
 FD_ZERO(&sockets_set);
 FD_SET(server_sock,&sockets_set);
 SOCKET max_socket = server_sock;
 
 
 
 while(1)
 {
	 fd_set read_sockets;
	 read_sockets = sockets_set;
	 
	 char adr[ADDRLEN] = {0};
	 char client_name[ADDRLEN] = {0};
	 
	 if(select(max_socket+1,&read_sockets, 0,0,NULL))
	 {
		 fprintf(stderr,"select() faild(%d)",GETSOCKETERRNO());
		 show_error(GETSOCKETERRNO());
		 return 1;
	 }
	 SOCKET i;
	 for(i = 1; i<=max_socket;++i)
	 {
		 if(FD_ISSET(i,&read_sockets))
		 {
			 if(i==server_sock)
			 {
				 struct sockaddr_storage client_address;
				 socklen_t client_len = sizeof(client_address);
				 SOCKET new_client = accept(server_sock, (struct sockaddr*)&client_address, &client_len)
				 if(!ISVALIDSOCKET(new_client))
				 {
					  fprintf(stderr,"accept() faild(%d)",GETSOCKETERRNO());
					  show_error(GETSOCKETERRNO());
					  return 1;
				 }
				 else
				 {
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,adr,ADDRLEN,0,0,NI_NUMERICHOST)!=0)
					 {
						 fprintf(stderr,"getnameinfo() faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,client_name,ADDRLEN,0,0,1)!=0)
					 {
						 fprintf(stderr,"getnameinfo() faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 printf("Accept new client: s& (s%)\n",client_name,adr);
					 FD_SET(i,&sockets_set);
					 if(max_socket<new_client)
					 {
						 max_socket= new_client;
					 }
				 } 
			 }
			 else
			 {
				char MSG[MSGSIZE]= {0};
				int recv_bytes = recv(i,MSG,MSGSIZE,0);
				if(recv_bytes<1)
				{
					FD_CLR(i,&sockets_set);
					CLOSESOCKET(i);
					continue;
				}
				///////////////
			 }
		 }
	 }
	 
 }
 
 /*
 //////////////////////////
 
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
