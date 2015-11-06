#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "transmitter.h"
#include "receiver.h"
#include "interface.h"

int setNum = 0;

int llopen() {

	if(appLayer.status == 0) {		
		printf("llopen: opening ports and sending SET bytes!\n");
		return saveConfig();
	}

	printf("llopen: opening ports!\n");
	return saveConfigNC();
}

int llread(int control) {

    int state = 0;
    int equalize = 0;
    int res = 0;
    int count_buf = 0;
    char buf[MAX_SIZE];   
    char ant[1];

    //RECEIVE INF
    strcpy(buf,"");
    
    while(TRUE) {
	    res = read(appLayer.fd,buf,1);

        switch(state) {
            case 0:
	            if(buf[res-1] == FLAG)
        	       state++;  
                count_buf = 0;
                ll.sequenceNumber = 0;
            break; 
    
            case 1:
                if(buf[res-1] == A_SEND) 
                    state++; 
                else if(buf[res-1] == FLAG) 
                    break; 
                else 
                    state=0; 
                strcpy(ll.frame,"");
                count_buf = 0;
                ll.sequenceNumber = 0;
                ant[0] = '\0';
            break; 
        
            case 2:
                if(buf[res-1] == C_SET) 
                    state++; 
                else if(buf[res-1] == C_SI && setNum == 1)
                    state = 5;
                else if(buf[res-1] == C_SF && control == 1 && setNum == 1)
                    state=5;
                else if (buf[res-1] == C_DISC)
                    state=8;
                else if(buf[res-1] == FLAG) 
                    state=1; 
                else 
                    state=0; 
            break;
     
            case 3:
                if(buf[res-1] == (A_SEND^C_SET)) 
                    state++; 
                else if (buf[res-1] == FLAG)
	               state = 1;
                else 
                    state = 0;
            break; 
    
            case 4:
                if (buf[res-1] == FLAG) {
                    printf("Received SET!\n");
                    state = 0;
	                setNum = 1;
                    return send_ua();
                }
                else 
                    state = 0;
            break;

            case 5:
                if(buf[res-1] == (C_SI^A_SEND)) {
                    state = 6;
                    equalize = 0;
                }
                else if(buf[res-1] == (C_SF^A_SEND)) {
                    state = 6;
                    equalize = 1;
                }
                else if (buf[res-1] == FLAG) 
                    state = 1;
                else 
                    state = 0;
                ant[0] = buf[res-1];
            break;
      
            case 6:
                if(buf[res-1] == FLAG) 
                    state = 1;
                else if(buf[res-1] == AFT_FLAG) {
	                if(ant[0] == ESC) {
        	            ll.frame[count_buf] = FLAG;
        	            count_buf++;
                        ant[0] = '\0';
	                }
	                else {
                        ll.frame[count_buf] = AFT_FLAG;
                        count_buf++;
                        ant[0] = AFT_FLAG;
	                }
                }
                else if(buf[res-1] == AFT_ESC){
	                if(ant[0] == ESC){
                        ll.frame[count_buf] = ESC;
        	            count_buf++;
		                ant[0] = '\0';
	                }
	                else {
		                ll.frame[count_buf] = AFT_ESC;
                        count_buf++;
                        ant[0] = AFT_ESC;
            	    }
                }
                else if(buf[res-1] == ESC) {
            		ant[0] = ESC;
            		break;
                }
                else if(buf[res-1] == (C_SI^A_SEND) || buf[res-1] == (C_SF^A_SEND)){
                    state = 7;
                    ant[0] = buf[res-1];
                }
                else {
                    ll.frame[count_buf] = buf[res-1];
                    count_buf++; 
                    ant[0] = buf[res-1];
                }
            break;

            case 7:
                if(buf[res-1] == FLAG){
                    printf("Successfully destuffed bytes and saved message! %d bytes\n", count_buf);
        	        ant[0]='\0';
        	        state = 0;
        	        ll.sequenceNumber = count_buf;
            	    return equalize+1;
                }
                else{
                    ll.frame[count_buf] = ant[0]; 
                    count_buf++;
                    ll.frame[count_buf] = buf[res-1];
        	        ant[0]=buf[res-1];
                    count_buf++;
                    state=6;
                }
            break;

            case 8:
                if(buf[res-1] == (A_SEND^C_DISC))
                    state=9;
                else if (buf[res-1] == FLAG)
                    state = 1;
                else 
                    state = 0;
            break;

            case 9:
                if(buf[res-1] == FLAG){
                    printf("Sending DISC and shutting down!\n");
                    return 3; //send_disc_nc();
                }
                else
                    state = 0;
            break;

            default:
                printf("Error llread: state is not between 0 and 9!\n");
            break;
        }
  }

  return -1;
}

int llwrite(int control, char * buf, int seqNum) {

	switch(control) {
		case 0:
			return send_rr(control,buf);
			break;

		case 1:
			return send_rr(control,buf);
			break;

		case 2:
			return prepare_inf(buf, seqNum);
			break;
	}
	return -1;
}

int llclose() {

	if(appLayer.status == 0)
		return prepare_send_disc();
    
	send_disc_nc();
	closeConfigNC();

	return 0;
}
