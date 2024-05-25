#include "Multiplatformheader.h" //универсальный заголовок для WIN/Linux







int main()
{
#if defined(_WIN32) //Инициализируем структуру WSADATA, где содержится вся информация о версии сокетов под WIN
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "\n##### FAILD to INITIASLIZE!!! #####\n");
		printf("%d", WSAGetLastError());
		return 1;
	}
#endif



////////////////////////////////////////////////////////////
printf("***** Configuring local server address...\n");
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
	fprintf(stderr, "##### Error getnameinfo() (%d) #####", GETSOCKETERRNO());
	show_error(GETSOCKETERRNO());
	return 1;
}
printf("***** SERVER ADDRESS %s:%s\n", serveraddress, serverport);

printf("***** Creating socet... \n");
SOCKET server_sock;
server_sock = socket(bind_address->ai_family,bind_address->ai_socktype,bind_address->ai_protocol); //создаем серверный сокет, через который сервер будет общатся с клиентами
 if(!ISVALIDSOCKET(server_sock))
 {
	 fprintf(stderr,"##### Creating server socket FAILED! (%d) #####\n",GETSOCKETERRNO());
	 show_error(GETSOCKETERRNO());
	 return 1;
 }
 else
 {
	 printf("***** Server socket is created!\n");
 }
 

 int option =0;
#if defined(_WIN32)
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&option, sizeof(option))) //функция изеняющая настрой ки сокета, для приема клиентов с адресами как IPV4  так и IPV^6
 {
	 fprintf(stderr, "##### setsockopt() faild!!! (%d) ##### ", GETSOCKETERRNO());
	 show_error(GETSOCKETERRNO());
	 return 1;
 }
#else
 if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option)))
 {
	 fprintf(stderr, "##### setsockopt() faild!!! (%d) #####", GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
#endif
 if(bind(server_sock,bind_address->ai_addr,bind_address->ai_addrlen)) // настраиваем сокет, присваиваем ему адрес
 {
	 fprintf(stderr,"##### bind() FAILED! (%d) #####\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
  else
 {
	 printf("***** Server's address is bound!\n");
 }
 freeaddrinfo(bind_address); // высвобождаем память занятую структурой адреса 
 printf("***** Listening....\n");
 if(listen(server_sock,MAXCLIENTS)<0)
 {
	 fprintf(stderr,"##### linten() FAILED! (%d) #####\n",GETSOCKETERRNO());
	 perror("ERROR");
	 return 1;
 }
 printf("***** Waiting for connection.... \n");
 
 //////////////////////////
 
 fd_set sockets_set;     //создаем набор сокетов
 FD_ZERO(&sockets_set);		// зануляем его
 FD_SET(server_sock,&sockets_set); //добавляем слушающий сокет
 SOCKET max_socket = server_sock; // создаем меременную max_socket для хранения максимального номера файлового дескриптора


 SOCKET *arr = (SOCKET*)malloc(sizeof(SOCKET) * MAXCLIENTS); //массив сокетов клиентов создается для отправки сообщения всем клиентам
 for (int i = 0; i < MAXCLIENTS; i++)						//инициализируем массив нулями. 0(stdin отслеживается селектом и _kbhit())
 {
	 arr[i] = 0;
 }
 
 //SOCKET arr[MAXCLIENTS] = { 0 };


 double x = NAN, y = NAN , result = NAN;  //переменные для калькуляции
 int no_x = 0;
 int no_y = 0;
 //int good_result = 0;

 char ch = (char)64;   //математическое действие.

 while(1)
 {
	 fd_set read_sockets; //создаем набор сокетов, которые будут проверятся на входящие данные. И инициализируем его нашим набором.
	 read_sockets = sockets_set;
	 
	 
	 if(select(max_socket+1,&read_sockets, 0,0,0)<0) // с помощью функции select проверяем все сокеты на предмет воходящих данных. 
	 {
		 fprintf(stderr,"\n##### select() faild(%d) #####\n",GETSOCKETERRNO());
		 show_error(GETSOCKETERRNO());
		 return 1;
	 }

	 for(SOCKET i = 1; i<=max_socket;++i) // итеррируемся по нашему набору 
	 {
		 if(FD_ISSET(i,&read_sockets)) // если select отловил сокет с входящими данными
		 {
			 char adr[ADDRLEN] = { 0 };
			 char client_name[ADDRLEN] = { 0 };

			 if(i==server_sock) // если это слушающий сокет
			 {
				 struct sockaddr_storage client_address; //новый адрес входящего клиента.
				 socklen_t client_len = sizeof(client_address);
				 SOCKET new_client = accept(server_sock, (struct sockaddr*)&client_address, &client_len); //принимаем нового клиента
				 if(!ISVALIDSOCKET(new_client))
				 {
					  fprintf(stderr,"\n##### accept() new client faild(%d) #####",GETSOCKETERRNO());
					  show_error(GETSOCKETERRNO());
					  return 1;
				 }
				 else
				 {
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,adr,ADDRLEN,0,0,NI_NUMERICHOST)!=0) //получаем адрес нового клиента
					 {
						 fprintf(stderr,"\n###### getnameinfo() faild(%d) #####",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 
					 if(getnameinfo((struct sockaddr*)&client_address,client_len,client_name,ADDRLEN,0,0,NI_NOFQDN)!=0) //получаем имя ПК нового клиента
					 {
						 fprintf(stderr,"\n###### getnameinfo() faild(%d) #####",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					 }
					 printf("\n***** Accept new(%d) client: %s (%s)\n", (int)new_client,client_name,adr);
					 FD_SET(new_client,&sockets_set);
					 if(max_socket<new_client)
					 {
						 max_socket = new_client;
					 }

					 add_client_to_arr(arr, MAXCLIENTS, new_client); //добавляем сокет в массив, для отправки всем сообщений от севера
					 
					///////////Отправка сообщений в зависимости от того, на каком этапе введены данные/////////
					if(!no_x)
					{
						char massage[] = "ENTER X: ";
						int send_bytes = send(new_client, massage,strlen(massage), 0);
						if(send_bytes <1)
						{
							fprintf(stderr, "\n##### send() to socket(%d) faild(%d) #####", (int)i, GETSOCKETERRNO());
							show_error(GETSOCKETERRNO());
						}			
					}
					else if(!no_y)
					{
						char massage[] = "ENTER Y: ";
						int send_bytes = send(new_client, massage,strlen(massage), 0);
						if(send_bytes <1)
						{
							fprintf(stderr, "\n##### send() to socket(%d) faild(%d) #####", (int)i, GETSOCKETERRNO());
							show_error(GETSOCKETERRNO());
						}							
					}
					else if(ch==(char)64)
					{
						char massage[] = "ENTER ACTION (*,/,-,+): ";
						int send_bytes = send(new_client, massage,strlen(massage), 0);
						if(send_bytes <1)
						{
							fprintf(stderr, "\n##### send() to socket(%d) faild(%d) #####", (int)i, GETSOCKETERRNO());
							show_error(GETSOCKETERRNO());
						}							
					}
					
					//////////////////////////////////////////////////////////////////////////////////////////

					 
					 
				 } 
			 }
			 else
			 {
				char MSG[MSGSIZE]= {0};
				int recv_bytes = recv(i,MSG,MSGSIZE,0);
				//printf("Recieve %d bytes: %.*s\n",recv_bytes,recv_bytes,MSG);
				if(recv_bytes<1)
				{
#if !defined(_WIN32)
					struct sockaddr_storage lost_client;
					memset(&lost_client,0,sizeof(lost_client));
					socklen_t lost_client_len = sizeof(struct sockaddr_in);
					if(getpeername(i,(struct sockaddr*)&lost_client,&lost_client_len)!=0)
					{
						fprintf(stderr,"#####getpeername().lost_client adr. faild(%d)#####",GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}
					if(getnameinfo((struct sockaddr*)&lost_client,lost_client_len,adr,sizeof(adr),0,0,NI_NUMERICHOST)!=0)
					{
						 fprintf(stderr,"#####getnameinfo() loose client faild(%d)#####",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					}
					if (getnameinfo((struct sockaddr*)&lost_client, lost_client_len, client_name, sizeof(client_name), 0, 0, NI_NOFQDN) != 0)
					{
						fprintf(stderr, "##### getnameinfo() loose client. faild(%d) #####", GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}
#endif
					FD_CLR(i,&sockets_set);
					CLOSESOCKET(i);
					printf("\n***** WE lost client(socket=%d): %s (%s)\n",(int)i, client_name,adr);
					del_client_from_arr(arr, MAXCLIENTS, i); //удаляем сокет из массива, клиент выбыл и сообщения от сервера ему более не доступны.
					continue;

				}
				else
				{
					struct sockaddr_storage post_client;
					memset(&post_client,0,sizeof(post_client));
					socklen_t post_client_len = sizeof(post_client);

					if(getpeername(i,(struct sockaddr*)&post_client,&post_client_len)!=0)
					{
						fprintf(stderr,"##### getperrname().post_client adr. faild(%d) #####",GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}
					if(getnameinfo((struct sockaddr*)&post_client,post_client_len,adr,sizeof(adr),0,0,NI_NUMERICHOST)!=0)
					{
						 fprintf(stderr,"##### getnameinfo() post_client adr. faild(%d) #####",GETSOCKETERRNO());
						 show_error(GETSOCKETERRNO());
						 return 1;
					}
					if (getnameinfo((struct sockaddr*)&post_client, post_client_len, client_name, sizeof(client_name), 0, 0, NI_NOFQDN) != 0)
					{
						fprintf(stderr, "##### getnameinfo() post_client adr. faild(%d) #####", GETSOCKETERRNO());
						show_error(GETSOCKETERRNO());
						return 1;
					}



					printf("\nRecieve from(socket=%d) %s (%s): %.*s", (int)i, client_name,adr,recv_bytes,MSG);

					//обработка ошибок ввода
					
					if(!no_x)
					{
						if(is_valid_double(MSG,recv_bytes))
						{
							x=atof(MSG);
							no_x = 1;
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"Client %s(%s) send X = %5.2lf\n",client_name, adr,x);
#else
							sprintf(MSG,"Client %s(%s) send X = %5.2lf\n",client_name, adr,x);
#endif
							send_to_clients(arr,MSG,i);

							memset(MSG, '\0', MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG, MSGSIZE, "ENTER Y: ");
#else
							sprintf(MSG, "ENTER Y: ");
#endif
							send_to_clients(arr, MSG, 0);
						}
						else
						{
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"INPUT ERROR! Client %s(%s) send NOT A NUMBER! Repeat !!! \n",client_name, adr);
#else
							sprintf(MSG,"INPUT ERROR! Client %s(%s) send NOT A NUMBER! Repeat !!! \n",client_name, adr);
#endif
							send_to_clients(arr,MSG,0);							
						}
					}
					else if(!no_y)
					{
						if(is_valid_double(MSG,recv_bytes))
						{
							y=atof(MSG);
							no_y = 1;
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"Client %s(%s) send Y = %5.2lf\n",client_name, adr,y);
#else
							sprintf(MSG,"Client %s(%s) send Y = %5.2lf\n",client_name, adr,y);
#endif
							send_to_clients(arr,MSG,i);

							memset(MSG, '\0', MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG, MSGSIZE, "ENTER ACTION (*,/,-,+): ");
#else
							sprintf(MSG, "ENTER ACTION (*,/,-,+): ");
#endif
							send_to_clients(arr, MSG, 0);
						}
						else
						{
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"INPUT ERROR! Client %s(%s) send NOT A NUMBER! Repeat !!! \n",client_name, adr);
#else
							sprintf(MSG,"INPUT ERROR! Client %s(%s) send NOT A NUMBER! Repeat !!! \n",client_name, adr);
#endif
							send_to_clients(arr,MSG,0);							
						}						
					}
					else if(ch==(char)64)
					{
						if(is_valid_action(MSG,strlen(MSG)))
						{
							ch=MSG[0];
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"Client %s(%s) send ACTION = %c",client_name, adr,ch);
#else
							sprintf(MSG,"Client %s(%s) send ACTION = %c",client_name, adr,ch);
#endif
							send_to_clients(arr,MSG,i);

							result = calculate(x,y,ch);
							
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"(%5.2lf %c %5.2lf = %5.2lf)\n",x,ch,y,result);
#else
							sprintf(MSG,"(%5.2lf %c %5.2lf = %5.2lf)\n",x,ch,y,result);
#endif
							send_to_clients(arr,MSG,0);	

							memset(MSG, '\0', MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG, MSGSIZE, "Lets play again! ENTER X: ");
#else
							sprintf(MSG, "Lets play again! ENTER X: ");
#endif
							send_to_clients(arr, MSG, 0);

							no_x = 0;
							no_y = 0;
							ch = (char)64;
						}
						else
						{
							memset(MSG,'\0',MSGSIZE);
#if defined(_WIN32)
							sprintf_s(MSG,MSGSIZE,"INPUT ERROR! Client %s(%s) sent NOT AN ACTION! Repeat !!! \n",client_name, adr);
#else
							sprintf(MSG,"INPUT ERROR! Client %s(%s) sent NOT AN ACTION! Repeat !!! \n",client_name, adr);
#endif
							send_to_clients(arr,MSG,0);							
						}						
					}
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
  
 free(arr);
 CLOSESOCKET(server_sock);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("\n****FINISH*****\n");
}
