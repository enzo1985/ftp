#ifndef _COMMON_H
#define _COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <types.h>
#include <unistd.h>

#define DEBUG 1
#define MAX_SIZE 512
#define CLIENT_PORT_ID 30020

struct command 
{
	char arg[255];
	char code[5];
};

int  socket_create(int port);
int  socket_accept(int sock_listen);
int  socket_connect(int port, char* host);
int  recv_data(int sockfd, char* buf, int bufsize);
int  send_response(int sockfd, int rc);
void trimstr(char* str, int n);
void read_input(char* buffer, int size);

#endif
