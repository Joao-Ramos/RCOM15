/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/sockfdet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <errno.h> 

#define SERVER_PORT "21"
#define SIZE 2048

struct ftpStruct{
	char[64]user;
	char[64] password;
	char[256] host;
	char[256] url-path;
};

struct ipStruct{

	int ip1;
	int ip2;
	int ip3;
	int ip4;
	int port1;
	int port2;
};

struct ftpStruct ftps;
struct ipStruct ips;

//function to receive answers from the socket
int receive_answer(int sockfd, char * buf){

	int code;
	char* rest;
	memset(buf, 0, SIZE);
	sleep(2);
	strcpy(buf, "");
	recv(sockfd,buf,SIZE, 0);
	printf("\n\nReceived message:\n\n%s\n\n",buf);

	if(sscanf(buf, "%d %s", code, rest) == 1){

		if(code >= 500)
		{
			printf("Failed with code %d!\n", code);
			exit(1);
		}
		else{
			return;
		}
	}
	return 0;
}

//function to send an answer through the socket
int send_answer(int sockfd, char* msg){

	printf("\n\nSending message:\n\n%s\n\n",msg);
	send(sockfd,msg, strlen(msg),0);

	return 0;
}

int argvHandler(char* argv){

	//ftp://[<user>:<password>@]<host>/<url-path>
    if (sscanf(argv, "ftp://%[^:]:%[^@]@%[^/]/%s\n", ftps.user, ftps.password, ftps.host, ftps.url-path) == 4){
    	
    	printf("Copied to user: %s\n",ftps.user);
    	printf("Copied to password: %s\n",ftps.password);
    	printf("Copied to host: %s\n",ftps.host);
    	printf("Copied to url-path: %s\n",ftps.url-path);
    	printf("Argv successfully decripted!\n\n");
    	
    	return 0;

    }
    if (sscanf(argv, "ftp://%[^/]/%s\n", ftps.host, ftps.url-path) == 2){
    	
    	strcpy(ftps.user, "anonymous");
    	strcpy(ftps.pass, "123");
    	printf("Copied to user: %s\n",ftps.user);
    	printf("Copied to password: %s\n",ftps.password);
    	printf("Copied to host: %s\n",ftps.host);
    	printf("Copied to url-path: %s\n",ftps.url-path);
    	printf("Argv successfully decripted!\n\n");
    	
    	return 0;

    }

    return -1;

}

int main(int argc, char** argv){


	char * buf[SIZE];
	char msg[SIZE];

	int sockfd;  
	struct addrinfo host_info, *server_info, *aux, *new_server;
	struct sockfdaddr_in *addr;
	int getaddr;

    if (argc != 2) {  
        fprintf(stderr,"usage: ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    if(argvHandler(argv[1] != 0)){
    	fprintf(stderr,"Argv format is wrong!\n Usage: ftp://[<user>:<password>@]<host>/<url-path>\n");
    	exit(1);
    }

	memset(&host_info, 0, sizeof(host_info));
	host_info.ai_family = AF_INET;
	host_info.ai_sockfdtype = sockfd_STREAM;
	host_info.ai_protocol = 0;

	if ((getaddr = getaddrinfo(ftps.host, SERVER_PORT, &host_info, &server_info)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddr));
	    exit(1);
	}
    
	for(aux = server_info; aux != NULL; aux = aux->ai_next) {
	    if ((sockfd = sockfdet(aux->ai_family, aux->ai_sockfdtype, aux->ai_protocol)) == -1) {
			perror("Failed to create sockfdet!");
			continue;
	    }

	    if (connect(sockfd, aux->ai_addr, aux->ai_addrlen) == -1) {
			close(sockfd);
			perror("Failed to connect to host!");
			continue;
	    }

	    break;
	}

	if (aux == NULL) {
	    fprintf(stderr, "Failed to connect!\n");
	    exit(2);
	}

	printf("Host name is:	%s\n", ftps.host);
	addr = (struct sockfdaddr_in *)server_info->ai_addr; 
	printf("Server address is:	 %s\n", inet_ntoa((struct in_addr)addr->sin_addr));

	//recever resposta do servidor
    receive_answer(sockfd, buf);
    //send user
    memset(msg, 0, SIZE);)
    strcpy(msg, "USER ");
    strcat(msg, ftps.user);
    send_answer(sockfd, msg);

	//receive 
	receive_answer(sockfd, buf);
	//send pass
	memset(msg, 0, SIZE);)
    strcpy(msg, "PASS ");
    strcat(msg, ftps.pass);
    send_answer(sockfd, msg);

    //receive 
	receive_answer(sockfd, buf);
	//send pasv
	memset(msg, 0, SIZE);)
    strcpy(msg, "PASV");
    send_answer(sockfd, msg);
    receive_answer(sockfd, buf);

	//receive answer as 193.231.321.31.2312.3 // check page from guião 1ªa aula
	// res is the buffer where the answer is saved
	if(sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)\n", ips.ip1, ips.ip2, ips.ip3, ips.ip4, ips.port1, ips.port2) != 6)
		{
			printf("Could not read the 6 bytes from the server response: %s\n", res);
			return -1;
		}

	close(sockfd);
	freeaddrinfo(server_info);
	exit(0);
}