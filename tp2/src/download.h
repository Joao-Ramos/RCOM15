#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <errno.h> 
#include <string.h>

#define SERVER_PORT "21"
#define SIZE 1024

struct ftpStruct {

	char user[64];
	char password[64];
	char host[256];
	char url_path[256];
};

struct ipStruct {

	int ip1;
	int ip2;
	int ip3;
	int ip4;
	int port1;
	int port2;
	int fport;
};


struct ftpStruct ftps;
struct ipStruct ips;

int receive_answer(int sockfd, char * buf);
int send_answer(int sockfd, char * msg);
int argvHandler(char* argv);
int main(int argc, char** argv);
int save_into_file(int fsockfd, char* fname);

#endif