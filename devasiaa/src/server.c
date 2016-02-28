#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/tcp.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#include "../include/global.h"
#include "../include/server.h"
#include "../include/comnd.h"

void server(char *mode, char *port){
	printf("server\n");

	struct sockaddr_in server, client;
	static struct slist *lists = NULL;
	struct slist *top = lists;
	char buff1[MAX_LINE], *buff;
	int len;
	fd_set master,read_fds;
	int fdmax,ret_fds;
	struct timeval tv;
	max_clients = 5;
	slistlen=0;
	//build address struct
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(port));
	//setup passive open
	if((ssockl = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("Cannot create socket\n");
		exit(1);
	}
	printf("Socket created\n");
	int yes = 1;

	/*
	if (setsockopt(ssockl,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	*/
	if (setsockopt(ssockl,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	

	if((bind(ssockl,(struct sockaddr *)&server, sizeof(server)))<0){
		perror("Cannot bind\n");
		close(ssockl);
		exit(1);
	}
	printf("Bind done\n");

	if(listen(ssockl,MAX_PENDING)<0){
		perror("Listen failed\n");
		close(ssockl);
		exit(1);
	}
	printf("wait for connection to arrive\n");
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(STDIN,&master);
	FD_SET(ssockl,&master);
	fdmax=ssockl;
	tv.tv_sec = 5; tv.tv_usec = 0;
	
	while(1){
		do{
			printf("Select\n");
			read_fds=master;
			ret_fds = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
		} while (ret_fds == -1 && errno == EINTR);
		//When an EINTR error is thrown the sets and  timeout become undefined, hence the above
		printf("In eternal while\n");
		if(FD_ISSET(ssockl,&read_fds)){
			printf("Listening Socket--shits come\n");
			bzero((char *)&client, sizeof(client));
			len=sizeof(client);
			struct slist *temp =  (struct slist *)malloc(sizeof(struct slist));
			if(!temp){
				perror("No memory\n");
				exit(1);
			}
			
			if((temp->ssockc = accept(ssockl,(struct sockaddr *)&client, &len))<0){
				perror("Accept failed\n");
				close(temp->ssockc);
				exit(1);
			}
			/*int flag = 1;
			if((setsockopt(temp->ssockc,IPPROTO_TCP,TCP_NODELAY, (char *)&flag, sizeof(int)))<0){
				perror("login:Socket optons cant be set\n");
				exit(1);
			}
			*/
			
			FD_SET(temp->ssockc,&master);
			fdmax = (temp->ssockc >fdmax)?temp->ssockc:fdmax;
			temp->sport = ntohs(client.sin_port);
			//printf("SPORT %d",temp->sport);
			buff = recvs(temp->ssockc,&master);
			
			if(strlen(buff)>=1){
				printf("--%sListening Port number\n", buff);
				strcpy(temp->port_num,buff);
			}
			else{
				temp->status = 0;
			}
			free(buff);
			struct hostent *he;
			he = gethostbyaddr(&client,sizeof(client),AF_INET);
			//printf("%s",he->h_name);
			strcpy(temp->hostname,he->h_name);
			//printf("%s\n",temp->hostname);
			/*if(!getnameinfo((struct sockaddr *)&client, sizeof(client),temp->hostname, sizeof(temp->hostname), service, sizeof(service), 0)){
				printf("getnameinfo failed\n");
				exit(1);
			}
			*/
			inet_ntop(AF_INET,&(client.sin_addr), temp->ip_addr, INET_ADDRSTRLEN);
			temp->sent =0; temp->recvd = 0;
			temp->status = 1;
			temp->msglen=0;
			temp->blocked =NULL; temp->blockedby = NULL;
			temp->blocklen =0; temp->blockbylen = 0;
			temp->bmax =10; temp->bbmax =10;
			if(lists==NULL){
				temp->next =NULL; temp->prev = NULL;
				lists=temp;
				slistlen+=1;
			}
			else{
				int fflag=0;
				top=lists;
				while(top!=NULL){
					if((strcmp(top->ip_addr,temp->ip_addr)==0)&&(strcmp(top->port_num,temp->port_num)==0)){
						printf("Setting flag to 1\n");
						fflag = 1;
						top->sport=temp->sport;
						free(temp);
						temp = top;
						break;
					}
					top=top->next;
				}
				if(fflag == 0){
					top = lists;
					printf("More than 1 CLient\n");
					while(top!=NULL){
						if((atoi(top->port_num))>(atoi(temp->port_num))){
							printf("Found place\n");
							if(top->prev!=NULL){
								top->prev->next = temp;
							}
							else{
								lists = temp;
							}
							temp->prev = top->prev;
							temp->next = top;
							top->prev = temp;
							break;
						}
						if(top->next == NULL){
							top->next = temp;
							temp->prev = top;
							temp->next = NULL;
							break;
						}
						top=top->next;
					}
					slistlen+=1;			
				}
			}
			sprintf(buff1,"%d",slistlen);
			if(!sends(buff1,temp->ssockc)){
				perror("Sending listlen failed\n");
				exit(1);
			}
			top = lists;
			int ctr =1;
			while(top!=NULL){
				sprintf(buff1,"%d %s %s %s",ctr,top->hostname,top->ip_addr,top->port_num);
				printf("List item %d = %s \n",ctr,buff1);
				sends(buff1,temp->ssockc);
				ctr++;
				top=top->next;
			}
			sprintf(buff1,"%d",temp->msglen);
			if(!sends(buff1,temp->ssockc)){
				perror("Sending msglen failed\n");
				exit(1);
			}
			for(int i = 0;i< temp->msglen;i++){
				if(!sends(temp->msg[i],temp->ssockc)){
					printf("Sending message %d failed\n",i);
					exit(1);
				}
			}
					
			printf("Client Socket Port %d\n",ntohs(client.sin_port));
			FD_CLR(ssockl,&read_fds);
		}
	
		
		if(FD_ISSET(STDIN,&read_fds)){
			printf("In STDIN\n");			
			if(fgets(c_str,MAX_LINE,stdin)!=NULL){
				if(c_str[strlen(c_str)-1]=='\n'){
					c_str[strlen(c_str)-1] = '\0';
				}
				if(strlen(c_str)>1){
					char *token = strtok(c_str, " ");
					strcpy(cmd_str, token);
					if(strcmp("STATISTICS", cmd_str)==0){
						stats(cmd_str, lists);
					}
					else if(strcmp("BLOCKED", cmd_str)==0){
						token = strtok(NULL,"");
						if(token!=NULL){
							strcpy(ip_str,token);
							if((blockedf(cmd_str,lists, ip_str))<0)
								errr(cmd_str);
						}
						else{
							errr(cmd_str);
						}
					}
					else if(strcmp("PORT", cmd_str)==0){
						dport(cmd_str,port);	
					}
					else if(strcmp("LIST", cmd_str)==0){
						lister(cmd_str, NULL, lists, NULL);	
					}
					else if(strcmp("IP", cmd_str)==0){
						dip(cmd_str,NULL);
					}
					else if(strcmp("AUTHOR",cmd_str)==0){
						author(cmd_str);
					}
				}
			}
		FD_CLR(STDIN,&read_fds);
		}	
		
		top=lists;
		while(top!=NULL){
			if(FD_ISSET(top->ssockc,&read_fds)){
				printf("CLIENT SOCKS\n");
				struct slist *top1 = lists;
				buff=recvs(top->ssockc,&master);
				printf("%s",buff);
				if(buff!=NULL){
					char *token = strtok(buff, " ");
					if(strcmp("SEND",token)==0){
						token = strtok(NULL, " ");
						if(token!=NULL)
							strcpy(ip_str, token);
						token = strtok(NULL, "");
						if(token!=NULL)
							strcpy(msg, token);
						top1=lists;
						int bflag=0;
						for(int i=0;i< top->blockbylen ;i++){
							if(strcmp(top->blockedby[i]->ip_addr,ip_str)==0){
									bflag=1;
									break;
							}
						}
						while((!bflag)&&(top1!=NULL)){
							if(!((strcmp(top1->ip_addr,ip_str)==0)&&(strcmp(top->port_num,top1->port_num)==0))){
								sprintf(buff1,"%s %s",top->ip_addr,msg);
								if(top1->status==1){
									if(!sends(buff1,top1->ssockc)){
										perror("SENDing from server failed\n");
										exit(1);
									}
								}
								else{
									strcpy(top1->msg[top1->msglen],msg);
								}
								event_msgrld(top->ip_addr, top1->ip_addr, msg);
								top->sent+=1;
								top1->recvd+=1;
								break;
							}
							top1=top1->next;
						}
					}
					else if(strcmp("BROADCAST",token)==0){
						token = strtok(NULL, "");
						if(token!=NULL)
							strcpy(msg, token);
						top1=lists;
						while(top1!=NULL){
							int bflag=0;
							sprintf(buff1,"%s %s",top->ip_addr,msg);
							for(int i=0;i<top1->blocklen;i++){
								if(strcmp(top1->blocked[i]->ip_addr,top->ip_addr)==0){
										bflag=1;
										break;
								}
							}
							if(!bflag){
								if(!((strcmp(top->ip_addr,top1->ip_addr)==0)&&(strcmp(top->port_num,top1->port_num)==0))){
									if(top1->status==1){	
										if(!sends(buff1,top1->ssockc)){
											perror("BROADCASTing from server failed\n");
											exit(1);
										}
									}
									else{
										strcpy(top1->msg[top1->msglen],msg);
									}
									top1->recvd+=1;
								}
							}
							top1=top1->next;
						}
						event_msgrld(top->ip_addr,"255.255.255.255",msg);
						top->sent+=1;						
					}
					else if(strcmp("REFRESH",token)==0){
						token = strtok(NULL, "");
						if(token!=NULL){
							if(slistlen==atoi(token)){
								if(!sends(token,top->ssockc)){
									perror("REFRESHing from server failed\n");
									exit(1);
								}
							}
							else{
								sprintf(buff1,"%d", slistlen);
								if(!sends(buff1,top->ssockc)){
									perror("REFRESHing list from server failed\n");
									exit(1);
								}
								top1=lists;
								int ctr =1;
								while(top1!=NULL){
									sprintf(buff1,"%d %s %s %s",ctr,top1->hostname,top1->ip_addr,top1->port_num);
									if(!sends(buff1,top->ssockc)){
										printf("Refresh message %d failed", ctr);
									}
									ctr++;
									top1=top1->next;
								}
							}
						}
					}
					else if(strcmp("BLOCK",token)==0){
						token = strtok(NULL, "");
						if(token!=NULL){
							strcpy(ip_str,token);
						}
						if(top->blocked == NULL){
							top->blocked = (struct slist **)malloc(top->bmax*sizeof(struct slist*));
						}
						else if(top->blocklen==top->bmax){
							top->bmax+=10;
							top->blocked = realloc(top->blocked,top->bmax*sizeof(struct slist*));
						}
						top1=lists;
						while(top1!=NULL){
							if(!((strcmp(top1->ip_addr,ip_str)==0)&&(strcmp(top->port_num,top1->port_num)==0))){
								top->blocked[top->blocklen]=top1;
								top->blocklen+=1;
								if(top1->blockedby == NULL){
									top1->blockedby = (struct slist **)malloc(top1->bbmax*sizeof(struct slist*));
								}
								else if(top1->blockbylen == top1->bbmax){
									top1->bbmax+=10;
									top1->blockedby = realloc(top1->blockedby,top1->bbmax*sizeof(struct slist*));
								}
								top1->blockedby[top1->blockbylen]=top;
								top1->blockbylen+=1;
								break;
							}
						top1= top1->next;
						}
					}
					else if(strcmp("UNBLOCK",token)==0){
						token = strtok(NULL, "");
						if(token!=NULL){
							strcpy(ip_str,token);
						}
						int bflag=0;
						for(int i =0;i<(top->blocklen)-1;i++){
							if(bflag||(strcmp(top->blocked[top->blocklen]->ip_addr, ip_str)==0)){
								bflag=1;
								top->blocked[top->blocklen]=top->blocked[(top->blocklen)+1];
							}
						}
						top->blocklen-=1;
						bflag=0;
						while(top1!=NULL){
							if(!((strcmp(top1->ip_addr,ip_str)==0)&&(strcmp(top->port_num,top1->port_num)==0))){
								for(int i =0;i<(top1->blockbylen)-1;i++){
									if(bflag||(strcmp(top1->blockedby[top1->blockbylen]->ip_addr, ip_str)==0)){
										bflag=1;
										top1->blockedby[top1->blockbylen]=top1->blockedby[(top1->blockbylen)+1];
									}
								}
								top->blockbylen-=1;
								break;
							}
						top1= top1->next;
						}
					}
				}
				else{
					top->status=0;
				}
				free(buff);
			}
			FD_CLR(top->ssockc,&read_fds);
			top=top->next;
		}
		
	}
	
	
}
