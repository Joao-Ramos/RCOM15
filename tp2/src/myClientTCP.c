/*      (C)2000 FEUP  */

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

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

struct ftpStruct{
	char[64]user;
	char[64] password;
	char[256] host;
	char[256] url-path;
};

struct ftpStruct ftps;

int argvHandler(char* argv){

	//ftp://[<user>:<password>@]<host>/<url-path>
    if (sscanf(argv, "ftp://%[^:]:%[^@]@%[^/]/%s\n", ftps.user, ftps.password, ftps.host, ftps.url-path) == 4){
    	
    	printf("Copied to user: %s\n",ftps.user);
    	printf("Copied to password: %s\n",ftps.password);
    	printf("Copied to host: %s\n",ftps.host);
    	printf("Copied to url-path: %s\n",ftps.url-path);
    	printf("Argv successfully decripted!\n");
    	
    	return 0;

    }

    return -1;

}

int main(int argc, char** argv){

	char * server_addr;
	struct hostent *h;

        if (argc != 2) {  
            fprintf(stderr,"usage: client address\n");
            exit(1);
        }

	if ((h=gethostbyname(argv[1])) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }

	server_addr = inet_ntoa(*((struct in_addr *)h->h_addr));
	printf ("Server address is: %s!\n", server_addr);

	int	sockfd;
	struct	sockaddr_in server_addr;
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";  
	int	bytes;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	
	/*connect to the server*/
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}

	//recever resposta do servidor
    
    //send user
	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);
	//receive 

	//send pass

	//send pasv

	//receive answer as 193.231.321.31.2312.3 // check page from guião 1ªa aula
	// res is the buffer where the answer is saved
	if(6 != sscanf(res, "%*[^(](%d,%d,%d,%d,%d,%d)\n", &(ctrl_ftp.pasv[0]), &(ctrl_ftp.pasv[1]), &(ctrl_ftp.pasv[2]), &(ctrl_ftp.pasv[3]), &(ctrl_ftp.pasv[4]), &(ctrl_ftp.pasv[5])))
		{
			printf("Could not read the 6 bytes from the server response: %s\n", res);
			return -1;
		}
	close(sockfd);
	exit(0);
}