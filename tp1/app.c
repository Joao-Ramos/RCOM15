#include "app.h"
#include "interface.h"
#include "fileOpp.h"

int checkFrames(char * buf){

  int i;

  for(i = 0; i < ll.sequenceNumber; i++)
      if(ll.compFrame[i] != buf[i])
          return -1;

  return 0;
}

int create_packets_send(){

	int i, timeout = 0;

		
	printf("llwrite: sending cp1!\n");
	while (timeout < ll.numTransmissions){
		strcpy(ll.frame,"");
		strcpy(ctrData.frame,"");
		createCtrlPackets(0);
		if(llwrite(2,ctrData.frame,ctrData.sequenceNumber) != 0)
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
		timeout=0;
		while (timeout < ll.numTransmissions){
			if(llwrite(2,fileData.frame, fileData.sequenceNumber) != 0)
				printf("llwrite: error sending dataFrame! Try number %d\n",timeout+1);
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
		if(prepare_inf(2,ctrData.frame, ctrData.sequenceNumber) != 0)
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

int send_rr(int equalize, int segmentNumber, char* buf){

  int i = 0;

  if(equalize == 0){
    for(i = 0; i < ll.sequenceNumber; i++)
      ll.compFrame[i] = buf[i];
  	return llwrite(equalize,buf,0);
  }
  else if (equalize == 1){

    if(checkFrames(buf) == 0){
      if(segmentNumber > 0)
      	saveChunk(buf,segmentNumber);
    }
     return llwrite(equalize,buf,0);
  }

  return -1;
}

int checkControl(int equalize){ //change this function

  int i;
  unsigned int size;
  int segmentNumber = 0;
  char buf[MAX_SIZE*2];

  if(ll.frame[0] == C_START){
    if(ll.frame[1] == C_SIZE_FILE){
      size = (unsigned int) ll.frame[2];
      char flength[size];
      for(i = 3; i < (3+size); i++){
        flength[i] = ll.frame[i];
      }

      ctrData.fileLength = ( (flength[0] << 24) 
                   + (flength[1] << 16) 
                   + (flength[2] << 8) 
                   + (flength[3] ) );
      i = 3+size;
      if(ll.frame[i] == C_NAME_FILE){
        i++;
        size = (unsigned int) ll.frame[i];
	i++;
        int curi = i;
        for(; i < (curi+size); i++){
          ctrData.filePath[i-curi] = ll.frame[i];
        }
        ctrData.fpLength = i-curi+1;
      }
    }
    return send_rr(equalize,0,"");
  }
  else if (ll.frame[0] == C_END){
    return send_rr(equalize,0,"");
  }
  else if(ll.frame[0] == C_DATA){

    segmentNumber = ll.frame[1];
    short L2 = (short) ll.frame[2]; 
    short L1 = (short) ll.frame[3];

    int size1 = 256*L2+L1;
	

    ll.sequenceNumber = ll.sequenceNumber - 4;

    for(i=0; i < ll.sequenceNumber; i++){
	buf[i] = ll.frame[i+4]; 
    }
    /*
    for(i=0; i < ll.sequenceNumber; i++){
	ll.frame[i] = buf[i];

    }*/

    
  
    return send_rr(equalize,segmentNumber,buf);
  }

  return -1;
}


int main (int argc, char ** argv){ //argv[1] = porta (0 a 5) argv[2] = flag (0 ou 1)

	
	ll.baudRate = 9600;
	ll.timeout = 2;
	ll.numTransmissions = 3;


	int port = 0;
	int flag = 0;
	int read_ret = 0;

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

	char sPort[20];

	strcpy(sPort,"");
	sprintf(sPort, "/dev/ttyS%d",port);

	appLayer.status = flag;
	strcpy(ll.port,sPort);

	if(llopen() == -1){	
		printf("Error llopen!\n");
		exit(1);

	}

	if(appLayer.status == 0){
		
		if(create_packets_send() == -1){
			printf("Error llwrite!\n");
			exit(1);
		}
	}
	else{

		while(TRUE){

			read_ret = llread();

			if (read_ret == 1){
				checkControl(read_ret-1);
			}
			else if(read_ret == 2){
				checkControl(read_ret-1);
			}
			else if(read_ret == 3){
				if(llclose() != 0){
					printf("\n\nError: llclose()!\n\n");
					exit(-1);
				}
			}

		}
	}

	llclose();

	return 0;

}

