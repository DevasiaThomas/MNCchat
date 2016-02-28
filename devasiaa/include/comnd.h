#ifndef COMND_H_
#define COMND_H_

#include "../include/server.h"
#include "../include/client.h"

int sends(char *strs, int s);
char* recvs(int s, fd_set *fds);
void success(char *cmnd_str);
void errors(char *cmnd_str);
void ends(char *cmnd_str);
void author(char *cmnd_str);
void exitf(char *cmnd_str);
void errr(char * cmnd_str);
void event_msgrcvd(char *buff);
void event_msgrld(char *fromip, char *toip, char *msg);
int logon(char * cmnd_str, struct sockaddr_in *server, char *port, struct list **listc, int *listlen);
int refresh(char *cmnd_str, int s, struct list **listc, int *listlen, fd_set *fds);
int servsend(char *cmnd_str,int s,char * ip, char *msg, struct list *listc, int listlen);
int broadcasts(char *cmnd_str,int s, char *msg);
int block(char *cmnd_str, int s,char **blocked, char *ip, struct list *listc, int listlen);
int unblock(char *cmnd_str, int s,char **blocked, char *ip);
int logout(char *cmnd_str, int s,struct list **listc, fd_set *fds);
int dport(char *cmnd_str, char *port);
//int block(char *cmnd_str, struct list *listc, int listlen);
void lister(char * cmnd_str,struct list *listc, struct slist *lists, int listlen);
int dip(char *cmnd_str,struct sockaddr_in *client);
void stats(char *cmnd_str, struct slist *lists);
int blockedf(char* cmnd_str, struct slist *lists, char *ip);


#endif
