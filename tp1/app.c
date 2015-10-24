
  #include "interface.h"




int llopen(int port, int flag){

	char sPort[20];

	strcpy(sPort,"");
	sprintf(sPort, "/dev/ttyS%d",port);

	appLayer.status = flag;
	strcpy(ll.port,sPort);

	printf("llopen: opening ports and sending SET bytes!\n");
	printf("%s\n",ll.port);

	if(appLayer.status == 0){
		if(saveConfig(ll.port) != 0)
			return -1;
		if(newConfig() != 0)	
			return -1;
		return prepare_set();
		
	}
	else {
		if(saveConfigNC(ll.port) != 0)
			return -1;
		if(newConfigNC() != 0)	
			return -1;
		return 0;
		
	}

	return -1; 
}

int llwrite(char* str){

	

	printf("llwrite: sending info!\n");
	return prepare_inf(str);

 
}

int llread(){

	printf("llread: reading info!\n");
	return  receive_inf(0);

}

int llclose(){

	
	if(appLayer.status == 0){

		if(prepare_send_disc() != 0){
			printf("Error closing program!\n");
			return -1;
		}
	}
	else {
		
		closeConfigNC();

	}

	return 0;

}

int main (int argc, char ** argv){ //argv[1] = porta (0 a 5) argv[2] = flag (0 ou 1)

	
	ll.baudRate = 9600;
	ll.timeout = 3;
	ll.numTransmissions = 3;


	int port = 0;
	int flag = 0;

    char str[MAX_SIZE];

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

	

	if(llopen(port,flag) == -1){	
		printf("Error llopen!\n");
		exit(1);

	}

	if(appLayer.status == 0){
	    printf("Write something: ");
		strcpy(str,"");
		gets(str);

		if(llwrite(str) == -1){
			printf("Error llwrite!\n");
			exit(1);
		}
	}
	else{
	
		llread();

	}

	if(llclose() == -1){
		printf("Error llclose!\n");
		exit(1);
	}

	return 0;

}

