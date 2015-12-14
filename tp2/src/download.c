#include "download.h"

//function to create and save contents received in a new file
int save_into_file(int fsockfd, char* fname){

	char buf[SIZE];
	FILE *my_file;
	int received_b = 0;
	unsigned long totalReceived = 0;

	my_file = fopen (fname, "w");

    if(my_file == NULL) {
		printf("Failed to open and write in file %s\n", fname);
		exit(1);
    }
    else 
    	printf("\nCreated or erased file %s!\nStarted retrieving file, please wait...\n", fname);
	
	while((received_b = recv(fsockfd, buf,SIZE,0)) != 0){
		totalReceived += received_b;
		fwrite(buf,sizeof(char),received_b, my_file);
	}
		
	printf("\n\nSuccess! Received %lu bytes and saved into file %s!\n\n", totalReceived, fname);
	fclose(my_file);

	return 0;
}

//function to receive answers from the socket
int receive_answer(int sockfd, char * buf){

	int code;
	memset(buf, 0, SIZE);
	sleep(1);
	strcpy(buf, "");
	recv(sockfd,buf,SIZE, 0);
	printf("\n\nReceived message:\n\n%s\n\n",buf);

	if(sscanf(buf, "%d %*s", &code) == 1){

		if(code >= 500)
		{
			printf("Failed with code %d!\n", code);
			exit(1);
		}
	}
	return 0;
}

//function to send an answer through the socket
int send_answer(int sockfd, char* msg){

	printf("\n\nSending message:\n\n%s\n\n",msg);
	strcat(msg,"\n");
	send(sockfd,msg, strlen(msg),0);

	return 0;
}

int argvHandler(char* argv){

	//ftp://[<user>:<password>@]<host>/<url->
    if (sscanf(argv, "ftp://%[^:]:%[^@]@%[^/]/%s\n", ftps.user, ftps.password, ftps.host, ftps.url_path) == 4){
    	
    	printf("Copied to user: %s\n",ftps.user);
    	printf("Copied to password: %s\n",ftps.password);
    	printf("Copied to host: %s\n",ftps.host);
    	printf("Copied to url_path: %s\n",ftps.url_path);
    	printf("Argv successfully decripted!\n\n");
    	
    	return 0;

    }
    if (sscanf(argv, "ftp://%[^/]/%s\n", ftps.host, ftps.url_path) == 2){
    	
    	strcpy(ftps.user, "anonymous");
    	strcpy(ftps.password, "123");
    	printf("Copied to user: %s\n",ftps.user);
    	printf("Copied to password: %s\n",ftps.password);
    	printf("Copied to host: %s\n",ftps.host);
    	printf("Copied to url_path: %s\n",ftps.url_path);
    	printf("Argv successfully decripted!\n\n");
    	
    	return 0;

    }

    return -1;

}

int main(int argc, char** argv){


	char buf[SIZE];
	char msg[512];
	char IP[128];
	char port[8];
	char IP_str[INET_ADDRSTRLEN];

	char fname[128];
	char *split;

	int sockfd, fsockfd;  
	struct addrinfo host_info, *server_info, *aux, *new_server;
	int getaddr;

    if (argc != 2) {  
        fprintf(stderr,"usage: ftp://[<user>:<password>@]<host>/<url_path>\n");
        exit(1);
    }

    if(argvHandler(argv[1]) != 0){
    	fprintf(stderr,"Argv format is wrong!\n Usage: ftp://[<user>:<password>@]<host>/<url_path>\n");
    	exit(1);
    }

   split = strtok(ftps.url_path, "/");
   
   while( split != NULL ) 
   {
      strcpy(fname,split);
      split = strtok(NULL, "/");
   }

   printf("\n\nDestination filename is: %s\n\n", fname);
	memset(&host_info, 0, sizeof(host_info));
	host_info.ai_family = AF_INET;
	host_info.ai_socktype = SOCK_STREAM;
	host_info.ai_protocol = 0;

	if ((getaddr = getaddrinfo(ftps.host, SERVER_PORT, &host_info, &server_info)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddr));
	    exit(1);
	}
    
	for(aux = server_info; aux != NULL; aux = aux->ai_next) {
	    if ((sockfd = socket(aux->ai_family, aux->ai_socktype, aux->ai_protocol)) == -1) {
			perror("Failed to create sockfd!");
			continue;
	    }

	    if (connect(sockfd, aux->ai_addr, aux->ai_addrlen) == -1) {
			close(sockfd);
			perror("Failed to connect to host!");
			continue;
	    }


		printf("Host name is:	%s\n", ftps.host);

		struct in_addr  *addr;  
	    if (aux->ai_family == AF_INET) { 
	        struct sockaddr_in *ipv = (struct sockaddr_in *)aux->ai_addr; 
	        addr = &(ipv->sin_addr);  
	    } 
	    else { 
	        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)aux->ai_addr; 
	        addr = (struct in_addr *) &(ipv6->sin6_addr); 
	    }
        inet_ntop(p->ai_family, addr, IP_str, sizeof ipstr); 
       /* printf("Server address is:	 %s\n", IP_str);

	    if(aux->ai_family == AF_INET){
	    	inet_ntop(aux->ai_family, &(((struct sockaddr_in *)addr)->sin_addr),
                    IP_str, sizeof(IP_str));
	    	
	    }
	    else{
	    	inet_ntop(aux->ai_family, &(((struct sockaddr_in6 *)addr)->sin6_addr),
                    IP6_str, sizeof(IP6_str));
	    	printf("Server address is:	 %s\n", IP6_str);
	    }*/

	    break;
	}

	if (aux == NULL) {
	    fprintf(stderr, "Failed to connect!\n");
	    exit(2);
	}

	

    //receive answer from server
    receive_answer(sockfd, buf);
    //send user
    memset(msg,0,512);
    strcpy(msg, "USER ");
    strcat(msg, ftps.user);
    send_answer(sockfd, msg);

    //receive 
    receive_answer(sockfd, buf);
    //send pass
    memset(msg, 0,512);
    strcpy(msg, "PASS ");
    strcat(msg, ftps.password);
    send_answer(sockfd, msg);

    //receive 
	receive_answer(sockfd, buf);
	//send pasv
	memset(msg, 0, 512);
    strcpy(msg, "PASV");
    send_answer(sockfd, msg);
    receive_answer(sockfd, buf);

	//receive answer as 193.231.321.31.2312.3 // check page from guião 1ªa aula
	// res is the buffer where the answer is saved
	if(sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)\n", &ips.ip1, &ips.ip2, &ips.ip3, &ips.ip4, &ips.port1, &ips.port2) != 6)
	{
		printf("Failed to read ip from server response! %s\n", buf);
		exit(1);
	}

	memset(IP, 0, 128);
	sprintf(IP,"%d.%d.%d.%d", ips.ip1, ips.ip2, ips.ip3, ips.ip4);
	printf("\nCopied ip: %s\n", IP);
	memset(port, 0, 8);
	ips.fport = 256 * ips.port1 + ips.port2; 
	sprintf(port,"%d",ips.fport);
	printf("\nCopied port: %s\n", port);

	if ((getaddr = getaddrinfo(IP, port, &host_info, &new_server)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddr));
	    exit(1);
	}
    
	for(aux = new_server; aux != NULL; aux = aux->ai_next) {
	    if ((fsockfd = socket(aux->ai_family, aux->ai_socktype, aux->ai_protocol)) == -1) {
			perror("Failed to create sockfd!");
			continue;
	    }

	    if (connect(fsockfd, aux->ai_addr, aux->ai_addrlen) == -1) {
			close(fsockfd);
			perror("Failed to connect to host!");
			continue;
	    }

	    break;
	}

	if (aux == NULL) {
	    fprintf(stderr, "Failed to connect!\n");
	    exit(2);
	}

	memset(msg, 0, 512);
    strcpy(msg, "RETR ");
    strcat(msg, ftps.url_path);
    send_answer(sockfd, msg);

    receive_answer(sockfd,buf);

    save_into_file(fsockfd,fname);

	close(fsockfd);
	close(sockfd);
	freeaddrinfo(server_info);
	freeaddrinfo(new_server);
	exit(0);
}