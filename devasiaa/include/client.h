#ifndef CLIENT_H_
#define CLIENT_H_

static int login;
struct list{
	int list_id;
	char hostname[35];
	char ip_addr[16];
	int port_num;
	};
static int listlen;
static int csockl, csocks, *csockp, csockplen;
static char *blocked;

void client(char *mode,char* port);


#endif
