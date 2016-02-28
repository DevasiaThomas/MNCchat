#ifndef SERVER_H_
#define SERVER_H_

struct slist{
	int ssockc;
	char hostname[35];
	char ip_addr[16];
	char port_num[5];
	int sport;
	int sent;
	int recvd;
	int status;
	char msg[30][286];
	int msglen;
	struct slist **blocked;
	int blocklen,bmax;
	struct slist **blockedby;
	int blockbylen,bbmax;
	struct slist *next;
	struct slist *prev;	
};
static int slistlen;
static int ssockl;

void server(char *mode,char* port);


#endif
