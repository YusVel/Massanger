#include "Multiplatformheader.h" //универсальный заголовок дл€ WIN/Linux







int main()
{
#if defined(_WIN32) //»нициализируем структуру WSADATA, где содержитс€ вс€ информаци€ о версии сокетов под WIN
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
struct addrinfo tips;  //Ќастройки адреса сервера
memset(&tips,0,sizeof(tips));
tips.ai_family = AF_INET6;
tips.ai_socktype = SOCK_STREAM;
tips.ai_flags = AI_PASSIVE; 
struct addrinfo *bind_address;
char serveraddress[ADDRLEN] = "::ffff:127.0.0.1";
char serverport[PORTLEN] = "5000";


get_yourIP(serveraddress); // ѕолучение текужего адреса ѕ 


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
server_sock = socket(bind_address->ai_family,bind_address->ai_socktype,bind_address->ai_protocol); //создаем серверный сокет, через который сервер будет общатс€ с клиентами
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
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&option, sizeof(option))) //функци€ изен€юща€ настрой ки сокета, дл€ приема клиентов с адресами как IPV4  так и IPV^6
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
 freeaddrinfo(bind_address); // высвобождаем пам€ть зан€тую структурой адреса 
 printf("Listening....\n");
 if(listen(server_sock,10)<0)
 {
	 fprintf(stderr,"linten() FAILED! (%d)\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 printf("Waiting for connection....\n");
 
 //////////////////////////
 
 fd_set sockets_set;     //создаем набор сокетов
 FD_ZERO(&sockets_set);		// занул€ем его
 FD_SET(server_sock,&sockets_set); //добавл€ем слушающий сокет
 SOCKET max_socket = server_sock; // создаем меременную max_socket дл€ хранени€ максимального номера файлового дескриптора
 
 
 
 while(1)
 {
	 fd_set read_sockets; //создаем набор сокетов, которые будут провер€тс€ на вход€щие данные. » инициализируем его нашим набором.
	 read_sockets = sockets_set;
	 
	 
	 if(select(max_socket+1,&read_sockets, 0,0,0)<0) // с помощью функции select провер€ем все сокеты на предмет воход€щих данных. 
	 {
		 fprintf(stderr,"select() faild(%d)/n",GETSOCKETERRNO());
		 show_error(GETSOCKETERRNO());
		 return 1;
	 }

	 for(SOCKET i = 1; i<=max_socket;++i) // итеррируемс€ по нашему набору 
	 {
		 if(FD_ISSET(i,&read_sockets)) // если select отловил сокет с вход€щими данными
		 {
			 if(i==server_sock) // если это слушающий сокет
			 {
				 
				 char adr[ADDRLEN] = {0};
				 char client_name[ADDRLEN] = {0};
				 
				 struct sockaddr_storage client_address; //новый адрес вход€щего клиента.
				 socklen_t client_len = sizeof(client_address);
				 SOCKET new_client = accept(server_sock, (struct sockaddr*)&client_address, &client_len); //принимаем нового клиента
				 if(!ISVALIDSOCKET(new_client))
				 {
					  fprintf(stderr,"accept() new client faild(%d)",GETSOCKETERRNO());
					  show_error(GETSOCKETERRNO());
					  return 1;
				 }
				 else
				 {
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,adr,ADDRLEN,0,0,NI_NUMERICHOST)!=0) //получаем адрес нового клиента
					 {
						 fprintf(stderr,"getnameinfo() faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,client_name,ADDRLEN,0,0,1)!=0) //получаем им€ ѕ  нового клиента
					 {
						 fprintf(stderr,"getnameinfo() faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 printf("Accept new(%d) client: %s (%s)\n",new_client,client_name,adr);
					 FD_SET(new_client,&sockets_set);
					 if(max_socket<new_client)
					 {
						 max_socket = new_client;
					 }
				 } 
			 }
			 else
			 {
				 
				char adr[ADDRLEN] = "123456789";
				char MSG[MSGSIZE]= {0};
				int recv_bytes = recv(i,MSG,MSGSIZE,0);
				//printf("Recieve %d bytes: %.*s\n",recv_bytes,recv_bytes,MSG);
				if(recv_bytes<1)
				{
					struct sockaddr_storage lost_client;
					memset(&lost_client,0,sizeof(lost_client));
					socklen_t lost_client_len = sizeof(struct sockaddr_in);

					
					if(getpeername(i,(struct sockaddr*)&lost_client,&lost_client_len)!=0)
					{
						fprintf(stderr,"getnameinfo().lost_client adr. faild(%d)",GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}
					
					if(getnameinfo((struct sockaddr*)&lost_client,lost_client_len,adr,sizeof(adr),0,0,NI_NUMERICHOST)!=0)
					{
						 fprintf(stderr,"getnameinfo() loose client. faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					}
					
					FD_CLR(i,&sockets_set);
					CLOSESOCKET(i);
					printf("WE lose client(socket=%d):(%s)\n",i,adr);
					continue;
				}
				else
				{
					struct sockaddr_storage post_client;
					memset(&post_client,0,sizeof(post_client));
					socklen_t post_client_len = sizeof(post_client);

					if(getpeername(i,(struct sockaddr*)&post_client,&post_client_len)!=0)
					{
						fprintf(stderr,"getnameinfo().post_client adr. faild(%d)",GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}
					if(getnameinfo((struct sockaddr*)&post_client,post_client_len,adr,sizeof(adr),0,0,NI_NUMERICHOST)!=0)
					{
						 fprintf(stderr,"getnameinfo() loose client. faild(%d)",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					}

					printf("Recieve from(socket=%d) (%s): %.*s\n",i,adr,recv_bytes,MSG);
				}
			 }
		 }
	 }
	 
 }
 
 
 //////////////////////////
 /*
 struct sockaddr_storage client_address;
 socklen_t client_len = sizeof(client_address); 

 SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_address, &client_len);

//////////////////////////////

char address[ADDRLEN] = "123456789";
struct sockaddr_in6 my_addr;
memset(&my_addr, 0, sizeof(my_addr));
socklen_t my_addrlen = sizeof(my_addr);
int err = getsockname(client_sock, (struct sockaddr*)&my_addr, &my_addrlen);

getnameinfo((struct sockaddr*)&my_addr,my_addrlen,address,sizeof(address),0,0,NI_NUMERICHOST);


printf("Client(socket=%d) conected from  %s\n",client_sock,address);

 //////////////////////////
 
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
  CLOSESOCKET(client_sock);
  */
  
  
  CLOSESOCKET(server_sock);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("\n****FINISH*****\n");
}
