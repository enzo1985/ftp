#include "ftserve.h"

/* main function entrance. */
int main(int argc, char** argv)
{
	if(argc!=2)
	{
		perror("usage: ./ftsere port");
		exit(0);
	}

	int port = atoi(argv[1]);

	/* create listen socket. */
	int sock_listen;
	if((sock_listen=socket_create(port))<0)
	{
		perror("Error creating socket");
		exit(1);
	}

	/* loop accept requests from different client. */
	int sock_control;
	int pid;
	while(1)
	{
		if((sock_control=socket_accept(sock_listen))<0)
		{
			break;
		}

		if((pid=fork())<0)
		{
			perror("Error forking child process");
		}
		else if(pid==0)
		{
			close(sock_listen);
			ftserve_process(sock_control);
			close(sock_control);
			exit(0);
		}

		close(sock_control);
	}

	close(sock_listen);

	return 0;
};

/*
 * Sends a specific file over a data socket.
 * Control information is exchanged over the control socket
 * dealing with invalid or non-existent file names.
 */
void ftserve_retr(int sock_control, int sock_data, char* filename)
{
	FILE* fd = NULL;
	char data[MAXSIZE];
	size_t num_read;

	fd = fopen(filename, "r"); // open file
	if(!fd)
	{
		send_response(sock_control, 550); // send error code (550 Requested action not taken)
	}
	else
	{
		send_response(sock_control, 150); // send okay (150 File status okay)

		do
		{
			num_read = fread(data, 1, MAXSIZE, fd); // read the contents of the file.
			if(num_read<0)
			{
				printf("error in fread()\n");
			}
			if(send(sock_data, data, num_read, 0)<0) // Send data (file contents)
			{
				perror("error sending file\n");
			}
		}while(num_read>0)
		
		send_response(sock_control, 226); // send a message (226: closing conn, file transfer successful)

		fclose(fd);
	}
};

/*
 * Responding to the requests: send a list of directory entries for the current directory. 
 * Close the data connection.
 * return : error -1, correct 0
 */
int ftserve_list(int sock_data, int sock_control)
{
	char data[MAXSIZE];
	size_t num_read;

	// use the system call function 'system' to execute the command and redirect to the tmp.txt file.
	int rs = system("ls -l | tail -n+2 > tmp.txt"); 
	if(rs<0)
	{
		exit(1);
	}

	FILE* fd = fopen("tmp.txt", "r");
	if(!fd)
		exit(1);

	/* seek to the beginning of the file. */
	fseek(fd, SEEK_SET, 0);

	send_response(sock_control, 1);

	memset(data, 0, MAXSIZE);

	/* through the data connection, send tmp.txt file content. */
	while((num_read=fread(data, 1, MAXSIZE, fd))>0)
	{
		if(send(sock_data, data, num_read, 0)<0)
		{
			perror("err");
			memset(data, 0, MAXSIZE);
		}
	}
	
	fclose(fd);
	send_response(sock_control, 226); 

	return 0;
};


/*
 * creates a data connection to the client.
 * success : return the socket for the data connection.
 * failure : -1
 */
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];
	int wait, sock_data;

	if(recv(sock_control, &wait, sizeof wait, 0)<0)
	{
		perror("Error while waiting");
		return -1;
	}

	struct sockaddr_in client_addr;
	socklen_t len = sizeof client_addr;
	getpeername(sock_control, (struct sockaddr*) &client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	if((sock_data=socket_connect(CLIENT_PORT_ID, buf))<0)
	{
		return -1;
	}

	return sock_data;
};


/*
 * User authentication.
 * success : 1, failure : 0
 */
int ftserve_check_user(char* user, char* pass)
{
	char username[MAXSIZE];
	char password[MAXSIZE];
	char* pch;
	char buf[MAXSIZE];
	char* line = NULL;
	size_t num_read;

	size_t len = 0;
	FILE* fd;
	int auth = 0;

	fd = fopen(".auth", "r"); // open the authentication file (record user name and password)
	if(NULL==fd)
	{
		perror("file not found");
		exit(1);
	}

	while((num_read=getline(&line, &len, fd))!=-1)
	{
		memset(buf, 0, MAXSIZE);
		strcpy(buf, line);

		pch = strtok(buf, " ");
		strcpy(username, pch);

		if(pch!=NULL)
		{
			pch = strtok(NULL, " ");
			strcpy(password, pch);
		}
		
		trimstr(password, (int)strlen(password));

		if((strcmp(user, username)==0) && (strcmp(pass, password))==0)
		{
			auth=1;
			break;
		}
	}

	free(line);
	fclose(fd);
	return auth;
};

/*
 * user login.
 */
int ftserve_login(int sock_control)
{
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];

	memset(buf, 0, MAXSIZE);
	memset(user, 0, MAXSIZE);
	memset(pass, 0, MAXSIZE);

	/* obtain the user name from the client. */
	if((recv_data(sock_control, buf, sizeof(buf)))==-1)
	{
		perror("recv error\n");
		exit(1);
	}

	int i=5;
	int n=0;
	while(buf[i]!=0) //buf[0-4]="USER"
		user[n++] = buf[i++];

	/* The user name is correct, informing the user to enter the password. */
	send_response(sock_control, 331);

	/* obtain the password from the client. */
	memset(buf, 0, MAXSIZE);
	if((recv_data(sock_control, buf, sizeof(buf)))==-1)
	{
		perror("recv error\n");
		exit(1);
	}

	i = 5;
	n = 0;
	while(buf[i]!=0) // buf[0-4] = "PASS"
		pass[n++] = buf[i++];

	return (ftserve_check_user(user, pass)); // user name and password authentication, and return.
};

/* accept the client's command, the respond, return the response code. */
int ftserve_recv_cmd(int sock_control, char* cmd, char* arg)
{
	int rc = 200;
	char buffer[MAXSIZE];

	memset(buffer, 0, MAXSIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAXSIZE);

	/* accept the client's request */
	if((recv_data(sock_control, buffer, sizeof(buffer)))==-1)
	{
		perror("recv error\n");
		return -1;
	}

	/* parse out the user's commands and parameters */
	strncpy(cmd, buffer, 4);
	char* tmp = buffer+5;
	strcpy(arg, tmp);

	if(strcmp(cmd, "QUIT")==0)
	{
		rc = 221;
	}
	else if((strcmp(cmd, "USER")==0) || (strcmp(cmd, "PASS")==0)
		 || (strcmp(cmd, "LIST")==0) || (strcmp(cmd, "RETR")==0))
	{
		rc = 200;
	}
	else
	{
		rc = 502; // invalid command.
	}

	send_response(sock_control, rc);
	return rc;
};

/* Handles client requests. */
void ftserve_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	send_response(sock_control, 220); // send the welcome answer code.

	/* user authentication */
	if(ftserve_login(sock_control)==1) // authentication success.
	{
		send_response(sock_control, 230);
	}
	else // authentication failure.
	{
		send_response(sock_control, 430);
		exit(0);
	}

	/* deal with the client's request */
	while(1)
	{
		/* Accept the command, and parse, get the command and parameters. */
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		if((rc<0) || (rc==221)) // the user enters the command QUIT
			break;

		if(rc==220)
		{
			// create a data connection with the client.
			if((sock_data=ftserve_start_data_conn(sock_control))<0)
			{
				close(sock_control);
				exit(1);
			}

			// execute command
			if(strcmp(cmd, "LIST")==0)
			{
				ftserve_list(sock_data, sock_control);
			}
			else if(strcmp(cmd, "RETR")==0)
			{
				ftserve_retr(sock_control, sock_data, arg);
			}
			close(sock_data);
		}
	}
};




