#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define PATH_LEN 256
#define MAX_PENDING 5
#define MAX_LINE 286
#define STDIN 0

static char public_ip[16];
static char const * const ubit_name = "devasiaa";
char cmd_str[11], ip_str[16], port_str[6], msg[MAX_LINE],c_str[286], *token;
static int max_clients;

#endif
