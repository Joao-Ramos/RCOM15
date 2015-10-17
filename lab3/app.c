  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <signal.h>
  #include "writenoncanonical.h"


int llopen(int porta){

	char sPorta[11];

	strcpy(sPorta,"");
	sprintf(sPorta, "/dev/ttyS%d",porta);
	printf("%s\n",sPorta);
	return openConf(sPorta);

 
}

int main (int argc, char ** argv){ //argv[1] = porta (0 a 5) argv[2] = flag (0 ou 1)


	int port = 0;
	int flag = 0;

	if (argc < 3) {
        printf("Usage:\t./app PortNumber flag\n\tex: ./app 5 1\n");
        exit(1);
        }

	port = atoi(argv[1]);
	flag = atoi(argv[2]);
	printf("%d\n%d\n",port,flag);
	
	if(port > 5 || port < 0){
	
		printf("Port number must be between 0 and 5!\n");
        	exit(1);

	}
	if(flag != 0 && flag != 1){
	
		printf("Flag must be 0 or 1!\n");
        	exit(1);

	}

	if(llopen(port) == -1){	
		printf("Error llopen!\n");

	}

	return 0;

}

