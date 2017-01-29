#ifndef _FTP_SERVER
#define _FTP_SERVER

#include "../common/common.h"

void ftserve_retr(int sock_control, int sock_data, char* filename);
int  ftserve_list(int sock_data, int sock_control);
int  ftserve_start_data_conn(int sock_control);
int  ftserve_check_user(char* user, char* pass);
int  ftserve_login(int sock_control);
int  ftserve_recv_cmd(int sock_control, char* cmd, char* arg);
void ftserve_process(int sock_control);

#endif
