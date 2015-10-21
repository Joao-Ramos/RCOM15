/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "writenoncanonical.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define ESC 0x7d
#define AFT_ESC 0x5d 
#define AFT_FLAG 0x5e
#define A_SEND 0x03   //sender comand
#define A_REC 0x01  // receiver command
#define C_SET 0x07
#define C_UA 0x03
#define C_SI 0x00
#define C_SF 0x20
#define C_RRI 0x21
#define C_RRF 0x01
#define C_REJ 0x25
 

volatile int STOP=FALSE;
volatile int STOP_REC=FALSE;


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
            if(buf[res-1] == FLAG)
              printf("Received UA and closing down!\n");
              return closeConfigNC();
            else count=0;
                  break;
        }
      }
         
      return -1;
    

  }

}


int send_disc(){
  char DISC[5];
  DISC[0] = FLAG;
  DISC[1] = A_SEND;
  DISC[2] = C_DISC;
  DISC[3] = DISC[1]^DISC[2];
  DISC[4] = FLAG;
  write(appLayer.fd,DISC, 6);

  receive_ua_nc();
  return 1;
}

/*int byte_destuffing(int state){

  int count_buf = 0;
  int count = 0;
  char buf[MAX_SIZE*2];


  for(count = 0; count < strlen(ll.frame); count++) {
    printf("%x\n",ll.frame[count]);
    switch (state){

    case 0:
      if(ll.frame[count] == FLAG)
        state = 1;
      else state = 0;
      break;

    case 1:
      if(ll.frame[count] == A_SEND)
        state = 2;
      else if (ll.frame[count] == FLAG) state = 1;
      else state = 0;
      break;

    case 2:
      if(ll.frame[count] == C_SI || ll.frame[count] == C_SF)
        state = 3;
      else if (ll.frame[count] == FLAG) state = 1;
      else state = 0;
      break;

    case 3:
      if(ll.frame[count] == C_SI^A_SEND || ll.frame[count] == C_SF^A_SEND)
        state = 4;
      else if (ll.frame[count] == FLAG) state = 1;
      else state = 0;
      break;
      
    case 4:
      if(ll.frame[count] == ESC && ll.frame[count+1] == AFT_FLAG){
        buf[count_buf] = FLAG;
        count++;
      }
      else if(ll.frame[count] == ESC && ll.frame[count+1] == AFT_ESC){
        buf[count_buf] = ESC;
        count++;
      }
      else if(ll.frame[count] == C_SI^A_SEND || ll.frame[count] == C_SF^A_SEND)
        state = 5;
      else buf[count_buf] = ll.frame[count];
      break;
    case 5:
      if(ll.frame[count] == FLAG){
        strcpy(ll.frame,buf);
        printf("Successfully destuffed bytes and saved message in ll.frame!\n");
        return 0;
      }
      else{
        buf[count_buf] = ll.frame[count-1];
        count_buf++;
        buf[count_buf] == ll.frame[count];
        state = 4;
      }
      break;
    default:
      printf("Error byte_destuffing: state is not between 0 and 4!\n");
      break;
    }

    count_buf++;

  }
  printf("The string you wanted to destuff doesn't end on BCC2 and FLAG!\n");
    
  return -1;
}*/

int send_rr(int control){

  char RR[5];
  RR[0] = FLAG;
  RR[1] = A_SEND;
  if(control == 1){
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
    UA[1] = A_SEND;
    UA[2] = C_UA;
    UA[3] = UA[1]^UA[2];
    UA[4] = FLAG;

    //SEND UA
    write(appLayer.fd,UA,5);
    printf("Sent response!\n");

    return 0;
}

int receive_inf(int control){

  int state = 0
  int res, count;
  int count_buf = 0;
  int fin_control = control;
  char buf[MAX_SIZE];   

  //RECEIVE INF
  strcpy(buf,"");
    
  for(count = 0; count < strlen(ll.frame); count++) {
	
  	printf("%x\n", ll.frame[count]);
  	switch(state){
  	case 0:
	    if(ll.frame[count] == FLAG) 
        state++;  
      break; 
    
    case 1:
      if(ll.frame[count] == A_SEND) 
        state++; 
      else if(ll.frame[count] == FLAG) 
        break; 
      else state=0; 
        break; 
        
    case 2:
      if(ll.frame[count] == C_SET) 
        state++; 
      else if(ll.frame[count] == C_SI && control > 0){
        state = 5;
        fin_control = 1;
      } 
      else if(ll.frame[count] == C_SF && control == 2){
        state=5;
        fin_control = control;
      }
      else if (ll.frame[count] == C_DISC)
        state=8;
      else if(ll.frame[count] == FLAG) 
        state=1; 
      else state=0; 
      break; 
     
    case 3:
      if(ll.frame[count] == (A_SEND^C_SET)) 
        state++; 
      else if (ll.frame[count] == FLAG)
	      state = 1;
      else state = 0;
        break; 
    
    case 4:
      if (ll.frame[count] == FLAG) {
        printf("Received SET!\n");
        return send_ua();
      }
      else state = 0;
      break;

    case 5:
      if(ll.frame[count] == C_SI^A_SEND || ll.frame[count] == C_SF^A_SEND)
        state = 6;
      else if (ll.frame[count] == FLAG) state = 1;
      else state = 0;
      break;
      
    case 6:
      if(ll.frame[count] == ESC && ll.frame[count+1] == AFT_FLAG){
        buf[count_buf] = FLAG;
        count++;
        count_buf++;
      }
      else if(ll.frame[count] == ESC && ll.frame[count+1] == AFT_ESC){
        buf[count_buf] = ESC;
        count++;
        count_buf++;
      }
      else if(ll.frame[count] == C_SI^A_SEND || ll.frame[count] == C_SF^A_SEND)
        state = 7;
      else {buf[count_buf] = ll.frame[count];   count_buf++;}
      break;

    case 7:
      if(ll.frame[count] == FLAG){
        strcpy(ll.frame,buf);
        printf("Successfully destuffed bytes and saved message in ll.frame!\n");
        return send_rr(fin_control);
      }
      else{
        buf[count_buf] = ll.frame[count-1];
        count_buf++;
        buf[count_buf] == ll.frame[count];
        count_buf++;
        state=6;
      }
      break;

    case 8:
      if(ll.frame[count] == A_SEND^C_DISC)
        state=9;
      else if (ll.frame[count] == FLAG) state = 1;
      else state = 0;
      break;

    case 9:
      if(ll.frame[count] == FLAG){
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

 int prepare_inf_nc(int control){

  int res;
  int rec;

  char buf[2];
  char buf1[MAX_SIZE];
  strcpy(buf,"");
  strcpy(buf1,"");
    
  while (STOP==FALSE) {       /* loop for input */
    res = read(appLayer.fd,buf,1);
    strncat(buf1, buf, res);
    if (buf[res-1]=='\0') STOP=TRUE;
    else buf[res]=0;
  }

  strcpy(ll.frame,buf1);
  rec = receive_inf(control);

  if(control == 0 && rec == 0){
    printf("Successfully received set message!\n");
    return 0;
  }
  else if(rec && control == 1)){
  
    receive_inf(control++);

  }
  else if(rec == 0 && control == 2){
    receive_inf(control);
  }
  
  return rec;
}


int saveConfigNC()
{
    int res;
    char buf[255];
    char buf1[255];

   /* if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", porta)!=0) && 
  	      (strcmp("/dev/ttyS1", porta)!=0) && 
  	      (strcmp("/dev/ttyS2", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS3", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    /*
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


    strcpy(buf1,"");
    
    char ant1, ant2;
    while (STOP==FALSE) {       /* loop for input */
      res = read(appLayer.fd,buf,1);

      ant1 = ant2;
      ant2 = buf[0];
      
      if (ant1 == FLAG && ant2 == A_SEND) {
          if(buf[0] == C_SET) {
            receive_set(appLayer.fd,1);
          }
	  else if (buf[0] == 0|1)
	  
      }
      else{
         strncat(buf1, buf, res);
         if (buf[res-1]=='\0') STOP=TRUE;
         else buf[res]=0;
      }
    }

    printf("%s\n", buf1);
    res = write(appLayer.fd,buf1,strlen(buf1) + 1);  
    printf("%d bytes written\n", res);





    
}


int newConfigNC(){

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
    leitura do(s) próximo(s) caracter(es)
    */



    tcflush(appLayer.fd, TCIOFLUSH);

    if ( tcsetattr(appLayer.fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
 }

int closeConfigNC(){

  tcsetattr(appLayer.fd,TCSANOW,&oldtio);
  close(appLayer.fd);
  return 0;
 
 }