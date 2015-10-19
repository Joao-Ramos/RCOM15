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


int byte_destuffing(){

  int res;
  int count = 0, state = 0;
  char buf[MAX_SIZE];


  for(count = 0; count < strlen(ll.frame); count++) {
    printf("%x\n",ll.frame[count]);
    switch (state){
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    }
    if(ll.frame[count] == FLAG){

    }

  }

  ll.frame[count2] = ll.frame[3];
  ll.frame[count2+1] = FLAG;
  printf("Stuffed count2+1 bytes!\n");
    
  return 0;
}

int send_rr(int control){

  return 0;
}

int receive_inf(int control){

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

int receive_set(int count){


    int res;
    char buf[255];   

    //RECEIVE SET
    strcpy(buf,"");
    while (STOP_REC == FALSE) {               
        res = read(appLayer.fd,buf,1);
	
  	printf("%x\n", buf[res-1]);
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
           if(buf[res-1] == C_SET) 
              count++; 
           else if(buf[res-1] == FLAG) 
              count=1; 
           else count=0; 
           break; 
        case 3:
           if(buf[res-1] == (A_SEND^C_SET)) 
              count++; 
           else if (buf[res-1] == FLAG)
	      count = 1;
           else count = 0;
           break; 
        case 4:
           if (buf[res-1] == FLAG) {
      	      printf("Received SET!\n");
      	      return send_ua(appLayer.fd);
           }
           else count = 0;
           break;
        }    
    }

    return -1;

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