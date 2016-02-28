#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/tcp.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"
#include "../include/comnd.h"

int sends(char *strs, int s){
	//char buff0[286];
	char ack[2];
	strs[strlen(strs)]='\0';
	int rsz;
	if((send(s,strs,strlen(strs),0))<0)
		{
			perror("send() error:");
			return 0;
		}
	if(rsz=recv(s,ack,1,MSG_WAITALL)){
		ack[1]='\0';
		printf("ACK %s",ack);
		if(strcmp("1",ack)==0)
			return 1;
		else
			return -1;
	}
	if(rsz == 0){
		printf("PEER Disconnected\n");
		close(s);
	}
	else{
		if(rsz == -1){
			perror("recv failed\n");
			exit(1);
		}
	}

	/*sprintf(buff0,"%s",strs);
	//printf("%s\n",buff0); 
	int lens = strlen(buff0) + 1;
	sprintf(buffl,"%d",lens);
	if(send(s,buffl,strlen(buffl),0))
	{	int rsz=recv(s,buffl,2,MSG_WAITALL);
		buffl[rsz]='\0';
	}
	printf("%s",buffl);
	if(strcmp("OK",buffl)==0){
		int total = 0;int bytesleft = lens;
		while(total < lens){
			int n = send(s,buff0+total,strlen(buff0+total),0);
			if(n == -1){
				perror("sends\n");
				return 0;
			}
			total += n;
			bytesleft -=n;
		}			
		return 1;
	}
	*/
}
char* recvs(int s, fd_set *fds){
	char *buff = (char *)malloc((MAX_LINE+1)*sizeof(char));
	int read_size;
	char buff0[MAX_LINE+1];
	int flag=0;
	read_size=recv(s,buff0,MAX_LINE,0);
	buff0[read_size]='\0';
	strcpy(buff,buff0);
	/*if(send(s,"OK",strlen("OK"),0)){
		if((read_size=recv(s,buff,atoi(buff0),MSG_WAITALL))>0)
			return buff;
	}
	else{printf("OK not sent\n");}
	*/
	if(read_size == 0){
		printf("Server Disconnected\n");
		close(s);
		if(fds != NULL)
			FD_CLR(s,fds);
		return NULL;
	}
	else{
		if(read_size == -1){
			perror("recv failed\n");
			exit(1);
		}
	}
	send(s,"1",1,0);
	return buff;
}
void success(char *cmnd_str){
	cse4589_print_and_log("[%s:SUCCESS]\n", cmnd_str);
}
void errors(char *cmnd_str){
	cse4589_print_and_log("[%s:ERROR]\n", cmnd_str);
}
void ends(char *cmnd_str){
	cse4589_print_and_log("[%s:END]\n", cmnd_str);
}
void author(char *cmnd_str){
	success(cmnd_str);
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit_name);
	ends(cmnd_str);
}
void exitf(char *cmnd_str){
	success(cmnd_str);
	ends(cmnd_str);
	exit(0);
}
void errr(char * cmnd_str){
	errors(cmnd_str);
	ends(cmnd_str);
}
void event_msgrcvd(char *buff){
	char ip[16], msg[MAX_LINE];
	sscanf(buff,"%s %s",ip,msg);
	success("RECEIVED");
	cse4589_print_and_log("msg from:%s\n[msg]:%s\n", ip, msg);
	ends("RECEIVED");
}
void event_msgrld(char *fromip, char *toip, char *msg){
	success("RELAYED");
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromip, toip, msg);
	ends("RELAYED");
}
int logon(char * cmnd_str, struct sockaddr_in *server, char *port, struct list **listc, int *listlen){
	int s,buff_msg;
	char *buff;
	if((s = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("listen:Cannot create socket\n");
		exit(1);
	}
	/*int flag = 1;
	if((setsockopt(s,IPPROTO_TCP,TCP_NODELAY, (char *)&flag, sizeof(int)))<0){
		perror("login:Socket optons cant be set\n");
		exit(1);
	}
	*/
	printf("login:Socket created\n");
	if(connect(s,(struct sockaddr *)server, sizeof(struct sockaddr_in))<0){
		if(errno==ECONNREFUSED){
			return -1;
		}
		else{
			perror("Connect failed\n");
			close(s);
			exit(1);
		}
	}
	//connection successful
	if(!sends(port,s)){
		printf("Send login 1 failed\n");
		return -1;
	}
	buff=recvs(s,NULL);
	if(strlen(buff)<1){
		printf("buff login num \n");
		return -1;
	}
	else{
		*listlen = atoi(buff);
		printf("buff loging num %s\n", buff);
	}
	free(buff);
	if(*listlen > max_clients)
		max_clients = *listlen;
	free(*listc);
	*listc = malloc((max_clients)*sizeof(struct list));
	for(int i=0;i<*listlen;i++){
		buff=recvs(s,NULL);
		if(strlen(buff) < 37){
			printf("List items %d error %s",i,buff); 
			return -1;
		}		
		else{
			printf("ITEM%d\n",i);
			printf("ITEM IN BUFF: %s\n",buff);
			sscanf(buff, "%d %s %s %d",&(*listc)[i].list_id,(*listc)[i].hostname,(*listc)[i].ip_addr,&(*listc)[i].port_num);
			printf("%d %s %s %d\n",(*listc)[i].list_id,(*listc)[i].hostname,(*listc)[i].ip_addr,(*listc)[i].port_num);
			free(buff);
		}
	}

	buff=recvs(s,NULL);	
	if(strlen(buff)<1){
		printf("msg size error\n");
		return -1;
	}
	else{
		buff_msg = atoi(buff);
		printf("message size%s\n", buff);
	}
	free(buff);
	

	for(int i=0;i<buff_msg;i++){
		buff=recvs(s,NULL);
		if(strlen(buff)<18){
			printf("message list %d error %s\n",i,buff);
			return -1;
		}
		else{
			event_msgrcvd(buff);
			printf("message %s\n", buff);
			free(buff);
		}
	}
	success(cmnd_str);
	ends(cmnd_str);
	return s;
}	
	
int refresh(char *cmnd_str, int s, struct list **listc, int *listlen, fd_set *fds){
	char buff[MAX_LINE], *buff2;
	sprintf(buff,"REFRESH %d",*listlen);
	int len;
	if(!sends(buff,s))
		return -1;
	buff2=recvs(s,fds);
	if(buff2 == NULL)
		return -1;
	else{
		len = atoi(buff2);
		printf("%s\n", buff2);
		free(buff2);
	}

	if(*listlen != len){
		if(len>max_clients){
			max_clients+=5;
			if(max_clients < len)
				max_clients = len;
		}
		free(*listc);
		*listc = malloc((max_clients)*sizeof(struct list));
		*listlen = len;
		for(int i=0;i< len;i++){
			buff2=recvs(s,NULL);
			if(buff2 == NULL){
				return -1;
			}
			else{
				printf("ITEM:%d :%s\n",i,buff2);
				sscanf(buff2, "%d %s %s %d",&(*listc)[i].list_id,(*listc)[i].hostname,(*listc)[i].ip_addr,&(*listc)[i].port_num);
				printf("%d %s %s %d\n",(*listc)[i].list_id,(*listc)[i].hostname,(*listc)[i].ip_addr,(*listc)[i].port_num);
				free(buff2);
			}
		}
	}
	success(cmnd_str);
	ends(cmnd_str);
	return 1;
				
}

int servsend(char *cmnd_str,int s,char * ip, char *msg, struct list *listc, int listlen){
	char buff[MAX_LINE];
	for(int i=0;i<listlen;i++){
		if(strcmp(ip,listc[i].ip_addr)==0){
			sprintf(buff,"SEND %s %s", ip, msg);
			if(!sends(buff,s))
				return -1;
			else{
				success(cmnd_str);
				ends(cmnd_str);
				return 1;
			}
		}
	}
	return -1;
			
}

int broadcasts(char *cmnd_str,int s, char *msg){
	char buff[MAX_LINE];
	sprintf(buff,"BROADCAST %s", msg);
		if(!sends(buff,s))
			return -1;
		else{
			success(cmnd_str);
			ends(cmnd_str);
			return 1;
		}
			
}

int block(char *cmnd_str, int s,char **blocked, char *ip, struct list *listc, int listlen){	
	char buff[MAX_LINE];
	for(int i=0;i<listlen;i++){
		if(strcmp(ip,listc[i].ip_addr)==0){
			int ctr =0;
			if(*blocked == NULL){
				*blocked = (char *)malloc(17*sizeof(char));
				sprintf(*blocked,"%s ",ip);
				printf("%s",*blocked);
			}
			else{
				char *temp1 = NULL, *token;
				token = strtok(*blocked," ");
				while(token!=NULL){
					if(strcmp(token,ip)==0)
						return -1;
					else{
						ctr++;
						token = strtok(NULL," ");
					}
				}
				temp1=realloc(*blocked,(ctr+1)*17*sizeof(char));
				if(!temp1){
					perror("No memmory\n");
					exit(1);
				}
				*blocked = temp1;
				sprintf((*blocked+(ctr*17)),"%s ",ip);
			}
			sprintf(buff,"BLOCK %s", ip);
			if(!sends(buff,s))
				return -1;
			else{
				success(cmnd_str);
				ends(cmnd_str);
				return 1;
			}
		}
	}		
	return -1;			
}

		
int unblock(char *cmnd_str, int s,char **blocked, char *ip){	
	char buff[MAX_LINE];
	sprintf(buff,"UNBLOCK %s", ip);
	if(*blocked == NULL){
		return -1;
	}
	else{
		char *temp1 = NULL, *token;
		token = strtok(*blocked," ");
		int ctr=0;
		while(token!=NULL){
			if(strcmp(token,ip)==0){
				token = strtok(*blocked, "");
				if(temp1 == NULL){
					if(token!=NULL){
						temp1=(char*)malloc((strlen(token)+1)*sizeof(char));
						strcpy(temp1,token);
						free(*blocked);
						*blocked = temp1;
					}
					else{
						free(*blocked);
					}
				}
				else{
					int len = strlen(temp1)+strlen(token)+1;
					char *temp2 = realloc(temp1,len*sizeof(char));
					if(!temp2){
						perror("No memmory");
						exit(1);
					}
					temp1 = temp2;
					sprintf((temp1+strlen(temp1)),"%s ",token);
					free(*blocked);
					*blocked = temp1;
				}
				if(!sends(buff,s))
					return -1;
				else{
					success(cmnd_str);
					ends(cmnd_str);			
					return 1;
				}
			}
			else{
				if(temp1 == NULL){
					temp1=(char*)malloc((strlen(token)+1)*sizeof(char));
					strcpy(temp1,token);
				}
				else{
					int len = strlen(temp1)+strlen(token)+1;
					char *temp2 = realloc(temp1,len*sizeof(char));
					if(!temp2){
						perror("No memmory");
						exit(1);
					}
					temp1 = temp2;
					sprintf((temp1+strlen(temp1)),"%s ",token);
				}
				token = strtok(*blocked," ");
			}
		}
		return -1;
	}							
}

int logout(char *cmnd_str, int s,struct list **listc, fd_set *fds){
	close(s);
	free(*listc);
	FD_CLR(s,fds);
	success(cmnd_str);
	ends(cmnd_str);
	return 1;
}

int dport(char *cmnd_str, char *port){
	success(cmnd_str);
	cse4589_print_and_log("PORT:%d\n", atoi(port));
	ends(cmnd_str);
	return 1;
}
 
/*int blockl(char *cmnd_str, struct list *listc, int listlen){
	success(cmnd_str);
	for(int i=0;i<listlen;i++){
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", listc[i].list_id, listc[i].hostname, listc[i].ip_addr, listc[i].port_num);
	}
	ends(cmnd_str);
	return 1;
}
*/

void lister(char * cmnd_str,struct list *listc, struct slist *lists, int listlen){	
	if(listc!=NULL){
		success(cmnd_str);
		printf("In CLIST\n");
		for(int i=0;i<listlen;i++){
			//printf("%d %s %s %d",listc[i].list_id,listc[i].hostname,listc[i].ip_addr,listc[i].port_num);
			cse4589_print_and_log("%-5d%-35s %-20s%-8d\n", listc[i].list_id, listc[i].hostname, listc[i].ip_addr, listc[i].port_num);
			printf("%-5dlol%-35slol%-20slol%-8d\n", listc[i].list_id, listc[i].hostname, listc[i].ip_addr, listc[i].port_num);
		}
		ends(cmnd_str);
	}
	else{
		success(cmnd_str);
		if(lists!=NULL){
			int ctr = 1;
			while(1){
				cse4589_print_and_log("%-5d%-35s %-20s%-8d\n", ctr, lists->hostname, lists->ip_addr, atoi(lists->port_num));
				printf("%-5dlol%-35slol%-20slol%-8d\n", ctr, lists->hostname, lists->ip_addr, atoi(lists->port_num));
				ctr++;
				if(lists->next!=NULL){
					lists=lists->next;
				}
				else
					break;
			}
		}
		ends(cmnd_str);
	}
}

	

int dip(char *cmnd_str){
	struct addrinfo hints,*result,*iter;
	int status;
	char hname[34];
	char ip_str[16];
	if(gethostname(hname,sizeof(hname))==-1){
		perror("HOSTNAME ERROR");
		return -1;
	}
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	if(getaddrinfo(hname,NULL,&hints,&result)!=0){
	       	 fprintf(stderr,"getaddrinfo error: %s\n",gai_strerror(status));
	       	 return -1;
	}
	for(iter=result;iter!=NULL;iter=iter->ai_next){
		void *addr;
		if(iter->ai_family==AF_INET){
			struct sockaddr_in *afinet = (struct sockaddr_in *)iter->ai_addr;
			addr = &(afinet->sin_addr);
		}
		else
		{
			continue;
		}

		if(inet_ntop(iter->ai_family,addr,ip_str,INET_ADDRSTRLEN)<=0)
		{
			printf("error in inet_ntop\n");
			return -1;
		}
		success(cmd_str);
		cse4589_print_and_log("IP:%s\n", ip_str);
		ends(cmd_str);
		freeaddrinfo(result);
		return 1;
	}
	freeaddrinfo(result);
	return -1;
}	

void stats(char *cmnd_str,struct slist *lists ){
	success(cmnd_str);
	if(lists!=NULL){
		int ctr = 1;
		while(1){
			cse4589_print_and_log("%-5d%-35s %-8d%-8d%-8s\n", ctr, lists->hostname, lists->sent, lists->recvd, (lists->status==0)?"offline":"online");
			ctr++;
			if(lists->next!=NULL){
				printf("Stats next exists\n");
				lists=lists->next;
			}
			else{
				printf("Stats else\n");
				break;
			}
		}
	}
	ends(cmnd_str);
}

int blockedf(char* cmnd_str, struct slist *lists, char *ip){
	if(lists!=NULL){
		printf("Lists present in blocked\n");
		int ctr = 1;
		while(1){
			if(strcmp(lists->ip_addr,ip)==0){
				for (int i = 0; i < lists->blocklen; i++){
					int swapped =0;
		        		for (int j = 0; j < (lists->blocklen-1); j++){
						if ((atoi(lists->blocked[j]->port_num)) > (atoi(lists->blocked[j + 1]->port_num))){
							struct slist *temp = lists->blocked[j];
							lists->blocked[j] = lists->blocked[j+1];
							lists->blocked[j+1] = temp;
							swapped =1;
						}
					}
					if (!swapped){
						break;
					}
				}
				success(cmnd_str);
				for(int i =0;i<lists->blocklen;i++){
					cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", ctr, lists->blocked[i]->hostname, lists->blocked[i]->ip_addr, lists->blocked[i]->port_num);
					ctr++;
				}
				ends(cmnd_str);
				return 1;
			}
			if(lists->next!=NULL){
				lists=lists->next;
			}
			else{
				break;
			}
		}
	}
	return -1;	
}
	

