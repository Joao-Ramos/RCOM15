#include "interface.h"



int llopen(int port, int flag){

	char sPort[20];

	strcpy(sPort,"");
	sprintf(sPort, "/dev/ttyS%d",port);

	appLayer.status = flag;
	strcpy(ll.port,sPort);


	if(flag == 0){
		if(readData() != 0){
			printf("llopen: file doesn't exist!\n");
			return -1;
		}
		printf("llopen: opening ports and sending SET bytes!\n");
	}
	else printf("llopen: opening ports!\n");
	

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

int llwrite(){

	int i, timeout = 0;
	
	printf("llwrite: sending cp1!\n");
	while (timeout < ll.numTransmissions){
		strcpy(ll.frame,"");
		strcpy(ctrData.frame,"");
		createCtrlPackets(0);
		if(prepare_inf(ctrData.frame,ctrData.sequenceNumber) != 0)
			printf("llwrite: error sending cp1! Try number %d\n",timeout+1);
		else break;
		timeout++;
	}
	if(timeout == 3){
		printf("llwrite: timed out cp1!\n");
		return -1;
	}
	for(i = 1; i <= fileData.numSeg; i++){
		strcpy(ll.frame,"");
		strcpy(fileData.frame,"");
		createDataPacket(i);
		printf("llwrite: sending data segment %d!\n",i);
		timeout=0;
		while (timeout < ll.numTransmissions){
			if(prepare_inf(fileData.frame, fileData.sequenceNumber) != 0)
				printf("llwrite: error sending cp1! Try number %d\n",timeout+1);
			else break;
			timeout++;
		}
		if(timeout == 3){
			printf("llwrite: timed out data sending!\n");
			return -1;
		}
	}
	
	
	printf("llwrite: sending cp2!\n");
	timeout=0;
	while (timeout < ll.numTransmissions){
		strcpy(ctrData.frame,"");
		strcpy(ll.frame,"");
		createCtrlPackets(1);
		if(prepare_inf(ctrData.frame, ctrData.sequenceNumber) != 0)
			printf("llwrite: error sending cp2! Try number %d\n",timeout+1);
		else break;
		timeout++;

	}
	if(timeout == 3){
		printf("llwrite: timed out cp2!\n");
		return -1;
	}

	return 0;
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

	if (argc < 3) {
		printf("Usage:\t./app PortNumber flag\n\tex: ./app 5 1\n");
		exit(1);
	}

	port = atoi(argv[1]);
	flag = atoi(argv[2]);
	
	if(port > 5 || port < 0){
		
		printf("Port number must be between 0 and 5!\n");
		exit(1);

	}
	if(flag != 0 && flag != 1){
		
		printf("Flag must be 0 or 1!\n");
		exit(1);

	}
	if(flag==0){
		printf("Write the name of the file you want to copy: ");
		strcpy(ctrData.filePath,"");
		gets(ctrData.filePath);
		ctrData.fpLength = strlen(ctrData.filePath);
	}

	if(llopen(port, flag) == -1){	
		printf("Error llopen!\n");
		exit(1);

	}

	if(appLayer.status == 0){
		
		if(llwrite() == -1){
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

