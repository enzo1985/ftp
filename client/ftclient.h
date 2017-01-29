#ifndef FTP_CLIENT
#define FTP_CLIENT

#include "../common/common.h"

int  read_reply();
void print_reply(int rc);
int  ftclient_read_command(char* buf, int size, struct command* cstruct);
int  ftclient_get(int data_sock, int sock_control, char* arg);
int  ftclient_open_conn(int sock_conn);
int  ftclient_list(int sock_data, int sock_conn);
int  ftclient_send_cmd(struct command* cmd);
void ftclient_login();

#endif
