#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/tcp.h>
#include<netinet/in.h>
#include<netdb.h>

#include "../include/global.h"
#include "../include/client.h"
#include "../include/comnd.h"

void client(char *mode, char *port){

	login = 0;
	blocked = NULL;
	max_clients = 5;
	csocks=-1;

	struct sockaddr_in server, client;
	static struct list *listc = NULL;
	
	 int len,read_size;

	fd_set master,read_fds;
	int fdmax,ret_fds;
	struct timeval tv;
	
	//build address struct client
	bzero((char *)&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = htons(atoi(port));
	
	if((csockl = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("listen:Cannot create socket\n");
		exit(1);
	}
	printf("listen:Socket created\n");

	int yes=1;
	/*	
	if (setsockopt(csockl,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	*/
	if (setsockopt(csockl,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	if((bind(csockl,(struct sockaddr *)&client, sizeof(client)))<0){
		perror("Cannot bind\n");
		close(csockl);
		exit(1);
	}
	printf("Bind done\n");

	if(listen(csockl,MAX_PENDING)<0){
		perror("Listen failed\n");
		close(csockl);
		exit(1);
	}
	printf("wait for connection to arrive\n");
	
	//initializing fd_set	
	FD_ZERO(&master);
	FD_SET(STDIN,&master);
	FD_SET(csockl,&master);
	fdmax=csockl;
	tv.tv_sec = 15; tv.tv_usec = 0;
	
	//build address structure
	//bzero((char *)&server,sizeof(server));
	//server.sin_family = AF_INET;
	//inet_pton(AF_INET,host, &(server.sin_addr));
	//printf("%s",inet_ntop(AF_INET,&(server.sin_addr), host, INET_ADDRSTRLEN));
	//server.sin_port = htons(atoi(hport));
	
	while(1){
		do{
			FD_ZERO(&read_fds);
			read_fds=master;
			ret_fds = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
		} while (ret_fds == -1 && errno == EINTR);
		//When an EINTR error is thrown the sets and  timeout become undefined, hence the above
		if(FD_ISSET(STDIN,&read_fds)){
			printf("STDIN\n");
			if(fgets(c_str,MAX_LINE,stdin)!=NULL){
				if(c_str[strlen(c_str)-1]=='\n'){
					c_str[strlen(c_str)-1] = '\0';
				}
				if(strlen(c_str)>1){
					//c_str[285]='\0';
					//printf("%s", c_str);
					char *token = strtok(c_str, " ");
					//printf("%s",token);
					strcpy(cmd_str, token);
					if(strcmp("AUTHOR",cmd_str)==0){
						author(cmd_str);
					}
					else if(strcmp("EXIT",cmd_str)==0){
						if(login == 1){
							logout("LOGOUT",csocks,&listc,&master);
						}
						exitf(cmd_str);
					}
					else if(strcmp("LOGIN",cmd_str)==0){
						token = strtok(NULL, " ");
						if(token!=NULL){
							strcpy(ip_str, token);
							//printf("%s",ip_str);
						}
						token = strtok(NULL, " ");
						if(token!=NULL){
							strcpy(port_str, token);
							//printf("%s",port_str);
						}
						if(token == NULL){
							//printf("token Here\n");
							errr(cmd_str);
						}
						else{
							//build address structure
							bzero((char *)&server,sizeof(server));
							server.sin_family = AF_INET;
							if(!(inet_pton(AF_INET,ip_str, &(server.sin_addr))))
								errr(cmd_str);
							//printf("%s",inet_ntop(AF_INET,&(server.sin_addr), ip_str, INET_ADDRSTRLEN));
							if(!((atoi(port_str)>1024)&&(atoi(port_str)<=65535)))
								errr(cmd_str);
							else{
								server.sin_port = htons(atoi(port_str));
								//printf("HERE\n");
								csocks = logon(cmd_str,&server,port, &listc, &listlen);
								if(csocks<0)
									errr(cmd_str);
								else{
									login =1;
									FD_SET(csocks,&master);
									printf("fdmax %d csocks %d\n",fdmax,csocks);
									fdmax = (fdmax>csocks)?fdmax:csocks;
									printf("fdmax %d\n",fdmax);
								}
							}
						}
					}	
					else if(login ==1){
						if(strcmp("REFRESH",cmd_str)==0){
							if((refresh(cmd_str, csocks, &listc, &listlen, &master))<0)
								errr(cmd_str);
						}
						else if(strcmp("SEND", cmd_str)==0){
							token = strtok(NULL, " ");
							if(token!=NULL)
								strcpy(ip_str, token);
							token = strtok(NULL, "");
							if(token!=NULL)
								strcpy(msg, token);
							if(token == NULL){
								errr(cmd_str);
							}
							else{
								if((servsend(cmd_str,csocks,ip_str, msg, listc, listlen))<0)
									errr(cmd_str);
							}
						}
						else if(strcmp("BROADCAST", cmd_str)==0){
							token = strtok(NULL, "");
							if(token!=NULL){
								strcpy(msg,token);
								if((broadcasts(cmd_str,csocks,msg))<0)
									errr(cmd_str);
							}
							else
								errr(cmd_str);
						}
						else if(strcmp("BLOCK", cmd_str)==0){
							token = strtok(NULL, "");
							if(token!=NULL){
								strcpy(ip_str,token);
								if((block(cmd_str,csocks,&blocked,ip_str,listc,listlen))<0)
									errr(cmd_str);
								printf("%s",blocked);
							}
							else
								errr(cmd_str);
						}
						else if(strcmp("UNBLOCK", cmd_str)==0){
							token = strtok(NULL, "");
							if(token!=NULL){
								strcpy(ip_str,token);
								if((unblock(cmd_str,csocks,&blocked,ip_str))<0)
									errr(cmd_str);
								printf("%s",blocked);
							}
							else
								errr(cmd_str);
						}
						else if(strcmp("LOGOUT", cmd_str)==0){
							if((logout(cmd_str,csocks,&listc,&master))<0)
								errr(cmd_str);
							else
								login=0;
							
						}
						else if(strcmp("PORT", cmd_str)==0){
							dport(cmd_str,port);	
						}
						else if(strcmp("LIST", cmd_str)==0){
							printf("List : %d",listlen);
							if(listc==NULL){
								printf("ITs NULL WTF!!!\n");
							}
							lister(cmd_str, listc, NULL, listlen);	
						}
						else if(strcmp("IP", cmd_str)==0){
							if(!dip(cmd_str)){
								errr(cmd_str);
							}
						}
					}		
				}				
			}
			FD_CLR(STDIN,&read_fds);
		}
		if(FD_ISSET(csocks,&read_fds)){
			printf("LISTENING PORT from server \n");
			char *buff = recvs(csocks, &master);
			if((buff!=NULL)&&(strlen(buff)>7)){
				printf("After recieving %s\n",buff);
				event_msgrcvd(buff);
				free(buff);
			}
			FD_CLR(csocks,&read_fds);
		}
	}
}
