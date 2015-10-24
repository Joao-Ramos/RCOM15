/*Non-Canonical Input Processing*/
#include "interface.h"


int set = 0;
volatile int STOP=FALSE;


int receive_ua_nc(){

      int res; 
      char buf[MAX_SIZE];
      int count = 0;

      strcpy(buf,"");
     
      while (1) {
        res = read(appLayer.fd,buf,1);

        switch(count){
              case 0:
            if(buf[res-1] == FLAG)
               count++;
            break;
              case 1:
            if(buf[res-1] == A_SEND)
               count++;
                  else if(buf[res-1] == FLAG)
                     break;
            else count=0;
                  break;
              case 2:
                  if(buf[res-1] == C_UA)
                     count++;
                  else if(buf[res-1] == FLAG)
                     count=1;
            else count=0;
                  break;
              case 3:
            if(buf[res-1] == (C_UA^A_SEND))
                     count++;
                  else if(buf[res-1] == FLAG)
                     count=1;
            else count=0;
                  break;
              case 4:
            if(buf[res-1] == FLAG){
              printf("Received UA and closing down!\n");
	            set = 0;
              return 0;
	    }
            else count=0;
                  break;
        }
      }
         
      return -1;
    

}



int send_disc_nc(){
  char DISC[5];
  DISC[0] = FLAG;
  DISC[1] = A_REC;
  DISC[2] = C_DISC;
  DISC[3] = DISC[1]^DISC[2];
  DISC[4] = FLAG;
  write(appLayer.fd,DISC, 6);

  receive_ua_nc();
  return 1;
}


int send_rr(int control){

  char RR[5];
  RR[0] = FLAG;
  RR[1] = A_REC;
  if(control == 0){
    RR[2] = C_RRI;
  }
  else{
    RR[2] = C_RRF;
  }
  RR[3] = RR[1]^RR[2];
  RR[4] = FLAG;

  printf("send_rr: Sending rr number %d\n", control);
  write(appLayer.fd,RR, 6);

  return 0;
}


int send_ua() {
    unsigned char UA[5];
    UA[0] = FLAG;
    UA[1] = A_REC;
    UA[2] = C_UA;
    UA[3] = UA[1]^UA[2];
    UA[4] = FLAG;

    //SEND UA
    set = 1;
    write(appLayer.fd,UA,5);
    printf("Sent response!\n");

    return 0;
}

int receive_inf(int control){

  int state = 0;
  int res = 0;
  int count_buf = 0;
  char buf[MAX_SIZE];   
  char ant[1];

  //RECEIVE INF
  strcpy(buf,"");
    
  while(STOP == FALSE) {
	res = read(appLayer.fd,buf,1);

  	printf("%x\n", buf[res-1]);
  	switch(state){
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
      else state=0; 
        break; 
        
    case 2:
      if(buf[res-1] == C_SET) 
        state++; 
      else if(buf[res-1] == C_SI && set == 1){
        state = 5;
      } 
      else if(buf[res-1] == C_SF && control == 1 && set == 1){
        state=5;
      }
      else if (buf[res-1] == C_DISC)
        state=8;
      else if(buf[res-1] == FLAG) 
        state=1; 
      else state=0; 
      break; 
     
    case 3:
      if(buf[res-1] == (A_SEND^C_SET)) 
        state++; 
      else if (buf[res-1] == FLAG)
	      state = 1;
      else state = 0;
        break; 
    
    case 4:
      if (buf[res-1] == FLAG) {
        printf("Received SET!\n");
	state = 0;
        send_ua();
      }
      else state = 0;
      break;

    case 5:
      if(buf[res-1] == (C_SI^A_SEND) || buf[res-1] == (C_SF^A_SEND))
        state = 6;
      else if (buf[res-1] == FLAG) state = 1;
      else state = 0;
      ant[0] = buf[res-1];
      break;
      
    case 6:
      if(buf[res-1] == AFT_FLAG){
	if(ant[0] == ESC){
        	ll.frame[count_buf] = FLAG;
        	count_buf++;
	}
	else {
		ll.frame[count_buf] = AFT_FLAG;
		count_buf++;
	}
      }
      if(buf[res-1] == AFT_ESC){
	if(ant[0] == ESC){
        	ll.frame[count_buf] = ESC;
        	count_buf++;
	}
	else {
		ll.frame[count_buf] = AFT_ESC;
		count_buf++;
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
      else {ll.frame[count_buf] = buf[res-1];   count_buf++;}
      break;

    case 7:
      if(buf[res-1] == FLAG){
        strcpy(ll.frame,buf);
        printf("Successfully destuffed bytes and saved message in ll.frame!\n");
	ant[0]='\0';
	state = 0;
	ll.sequenceNumber = count_buf;
        send_rr(control);
	if(control == 0)control++;
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
      else if (buf[res-1] == FLAG) state = 1;
      else state = 0;
      break;

    case 9:
      if(buf[res-1] == FLAG){
        printf("Sending DISC and shutting down!\n");
        return send_disc();

      }
      else state = 0;
      break;
    default:
      printf("Error byte_destuffing: state is not between 0 and 9!\n");
      break;
    
    }

     
  }

  return -1;

 }


int saveConfigNC()
{
   /* if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", porta)!=0) && 
  	      (strcmp("/dev/ttyS1", porta)!=0) && 
  	      (strcmp("/dev/ttyS2", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS3", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
  
    
    appLayer.fd = open(ll.port, O_RDWR | O_NOCTTY );
    if (appLayer.fd <0) {perror(ll.port); exit(-1); }

    if ( tcgetattr(appLayer.fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    

    //RECEIVE SET
    return 0;
    
}


int newConfigNC(){

    struct termios newtio;

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = ll.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) proximo(s) caracter(es)
    */



    tcflush(appLayer.fd, TCIOFLUSH);

    if ( tcsetattr(appLayer.fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    return 0;
 }

int closeConfigNC(){

  printf("Ending program!\n");

  sleep(5);

  if ( tcsetattr(appLayer.fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(appLayer.fd);
  exit(0);
 
 }
