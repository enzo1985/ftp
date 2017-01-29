#include "ftclient.h"

int sock_control;

int main(int argc, char** argv)
{
	int data_sock, retcode, s;
	char buffer[MAXSIZE];
	struct command cmd;

	if(argc!=3)
	{
		printf("usage: ./ftclient ip port\n");
		exit(0);
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);
	
	sock_control = socket_connect(port, ip);

	/* connect success, print message. */
	printf("Connected to %s.\n", ip);
	print_reply(read_reply());

	/* Get the user's name and password */
	ftclient_login();

	while(1)
	{ // loop, until user input quit.
		if(ftclient_read_command(buffer, sizeof(buffer), &cmd)<0)
		{
			printf("Invalid command\n");
			continue;
		}

		/* send command to server */
		if(send(sock_control, buffer, (int)strlen(buffer), 0)<0)
		{
			close(sock_control);
			exit(1);
		}

		retcode = read_reply(); // read server's response

		if(retcode==221)
		{
			print_reply(221);
			break;
		}

		if(retcode==502)
		{
			printf("%d Invalid command\n");
		}
		else
		{ // command is legal(rc=200), deal with command
			if(data_sock=ftclient_open_conn(sock_control)<0)
			{
				perror("Error opening socket for data connection.");
				exit(1);
			}

			// run command
			if(strcmp(cmd.code, "LIST")==0)
				ftclient_list(data_sock, sock_control);
			else if(strcmp(cmd.code, "RETR")==0)
			{
				if(read_reply()==550)
				{
					print_reply(550);
					close(data_sock);
					continue;
				}
				ftclient_get(data_sock, sock_control, cmd.arg);
				print_reply(read_reply());
			}

			close(data_sock);
		}
	} // loop, get more user's input

	close(sock_control);
	return 0;
};




/*
 * accept the server response.
 * success : status code
 * error : -1
 */
int read_reply()
{
	int retcode = 0;
	if(recv(sock_control, &retcode, sizeof retcode, 0)<0)
	{
		perror("client: error reading message from server\n");
		return -1;
	}
	return ntohl(retcode);
};

/* Prints a response message */
void print_reply(int rc)
{
	switch(rc)
	{
	case 220:
		printf("220 Welcome, server ready.\n");
		break;
	case 221:			
		printf("221 Goodbye.\n");
		break;
	case 226:
		printf("226 Closing data connection, Requested file action successful.\n");
		break;
	case 550:
		printf("550 Requested action not taken. File unavailable.\n");
		break;
	}
};

/* Parses the command line to the structure */
int ftclient_read_command(char* buf, int size, struct command* cstruct)
{
	memset(cstruct->code, 0, sizeof(cstruct->code));
	memset(cstruct->arg, 0, sizeof(cstruct->arg));

	printf("ftclient>"); // Enter the prompt
	fflush(stdout);
	read_input(buf, size); // Wait for the user to enter the command.
	
	char* arg = NULL;
	arg = strtok(buf, " ");
	arg = strtok(NULL, " ");

	if(NULL!=arg)
	{
		strncpy(cstruct->arg, arg, strlen(arg));
	}

	if(strcmp(buf, "list")==0)
		strcpy(cstruct->code, "LIST");
	else if(strcmp(buf, "get")==0)
		strcpy(cstruct->code, "RETR");
	else if(strcmp(buf, "quit"))
		strcpy(cstruct->code, "QUIT");
	else
		return -1; // illegal
		
	memset(buf, 0, 400);
	strcpy(buf, cstruct->code); // Store the command to the beginning of buf.
	
	/* If the command takes a parameter, append it to the buf. */		
	if(NULL!=arg)
	{
		strcat(buf, " ");
		strncat(buf, cstruct->arg, strlen(cstruct->arg));
	}

	return 0;
};


/* Implement the "get <filename>" command line. */
int ftclient_get(int data_sock, int sock_control, char* arg)
{
	char data[MAXSIZE];
	int size;
	FILE* fd = fopen(arg, "w"); // create and open a file with the name arg

	/* Writes the data(file contents) from the server to a locally created file. */
	while((size=recv(data_sock, data, MAXSIZE, 0))>0)
	{
		fwrite(data, 1, size, fd);
	}

	if(size<0)
		perror("error\n");

	fclose(fd);
	return 0;
};


/* open data connection */
int ftclient_open_conn(int sock_con1)
{
	int sock_listen = socket_create(CLIENT_PORT_ID);

	int ack = 1;
	if((send(sock_con1, (char*) &ack, sizeof(ack), 0))<0)
	{
		printf("client: ack write error: %d\n", errno);
		exit(1);
	}

	int sock_con2 = socket_accept(sock_listen);
	close(sock_listen);
	return sock_con2;
};


/* Implement of "list" command */
int ftclient_list(int sock_data, int sock_con)
{
	size_t num_recvd;
	char buf[MAXSIZE];
	int tmp = 0;

	/* Wait for the server to start */
	if(recv(sock_con, &tmp, sizeof tmp, 0)<0)
	{
		perror("client: error reading message from server");
		return -1;
	}

	memset(buf, 0, sizeof(buf));

	/* Accept the data from the server */
	while((num_recvd=recv(sock_data, buf, MAXSIZE, 0))>0)
	{
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}

	if(num_recvd<0)
	{
		perror("error");
	}

	/* waiting for the message about the server to complete */
	if(recv(sock_con, &tmp, sizeof tmp, 0)<0)
	{
		perror("client: error reading message from server.");
		return -1;
	}
	return 0;
};


/* 
 * Input the structure include a command and parameters.
 * Link code and arg, put them to a string, then send to server.
 */
int ftclient_send_cmd(struct command* cmd)
{
	char buffer[MAXSIZE];
	int rc;

	sprintf(buffer, "%s %s", cmd->code, cmd->arg);

	/* sendint command string to server */
	rc = send(sock_control, buffer, (int)strlen(buffer), 0);
	if(rc<0)
	{
		perror("Error sending a command to server");
		return -1;
	}

	return 0;
};

/*
 * obtain login information.
 * send to server for authentication.
 */
void ftclient_login()
{
	struct command cmd;
	char user[256];
	char pass[256];
	memset(user, 0, 256);
	memset(pass, 0, 256);
	memset(cmd, 0, sizeof cmd);

	/* obtain user name */
	printf("Name:");
	fflush(stdout);
	read_input(user, 256);

	/* sending username to server */
	strcpy(cmd.code, "USER");
	strcpy(cmd.arg, user);
	ftclient_send_cmd(&cmd);

	/* Waiting for the answer code 331 */
	int wait;
	recv(sock_control, &wait, sizeof wait, 0);
	memset(cmd, 0, sizeof cmd);

	/* Obtain password */
	printf("Password:");
	fflush(stdout);
	read_input(pass, 256);

	/* sending password to server */
	strcpy(cmd.code, "PASS");
	strcpy(cmd.arg, pass);
	ftclient_send_cmd(&cmd);

	/* waiting for response */
	int retcode = read_reply();
	switch(retcode)
	{
	case 430:
		printf("Invalid username/password\n");
		exit(0);
	case 230:
		printf("Successful login.\n");
		break;
	default:
		perror("error reading message from server.");
		exit(1);
		break;
	}
};







