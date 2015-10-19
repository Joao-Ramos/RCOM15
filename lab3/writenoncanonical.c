  /*Non-Canonical Input Processing*/

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

//  #define BAUDRATE B9600
//  #define MODEMDEVICE "/dev/ttyS5"
  #define _POSIX_SOURCE 1 /* POSIX compliant source */
  #define FALSE 0
  #define TRUE 1
  
  #define FLAG 0x7e
  #define A_SEND 0x03   //sender comand
  #define A_REC 0x01  // receiver command
  #define C_SET 0x07
  #define C_UA 0x03
  
//  #define TIME_ALARM 3 
//  #define ATEMPTS 3

  volatile int STOP=FALSE;
  volatile int STOP_REC=FALSE;

  int re_send = 0, flag_al = 1;

  void alarm_handler(){

     flag_al = 1;
     re_send++;
  }

  int receive_ua(int fd){

      int res; 
      char buf[255];
      int count = 0;

      strcpy(buf,"");
      while (STOP_REC == FALSE) {
        res = read(fd,buf,1);
        
        if(res == 0)
           return -1;

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
               return 0;
	    else count=0;
            break;
	}
      }
         
      return -1;
    

  }

  int send_set(int fd){

    
    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A_SEND;
    SET[2] = C_SET;
    SET[3] = SET[1]^SET[2];
    SET[4] = FLAG;

    //SEND SET
    write(fd,SET,5);
    printf("Sent SET\n");

    //RECEIVE UA
     
    return receive_ua(fd);
   

  }

  int newConfig(int fd){

      struct termios newtio;

      bzero(&newtio, sizeof(newtio));
      newtio.c_cflag = ll.baudRate | CS8 | CLOCAL | CREAD;
      newtio.c_iflag = IGNPAR;
      newtio.c_oflag = 0;
      /* set input mode (non-canonical, no echo,...) */
      newtio.c_lflag = 0;
      newtio.c_cc[VTIME]    = ll.timeout * 10;   /* inter-character timer unused */
      newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    /* 
      VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
      leitura do(s) pr�ximo(s) caracter(es)
    */



      tcflush(fd, TCIOFLUSH);
      if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }

      printf("New termios structure set\n");

      return 0;
  }

  int prepare_set(int fd){

    while (re_send < ll.numTransmissions){
        if(flag_al){
           alarm(ll.timeout);
           printf("SENDING n%d!\n",(re_send+1));
           flag_al=0;
           if(send_set(fd) != 0){
             printf("Error: no acknowledgment received (UA)!\n");
           }
           else break;
         
      }
    }

  }

  int openConfig(char* porta){
      int fd, res;
      struct termios oldtio;

      char buf[255];
      char buf1[255];
      char rec[255];

      (void) signal(SIGALRM, alarm_handler);
      
      
      /*if ( (argc < 2) || 
    	     ((strcmp("/dev/ttyS4", argv[1])!=0) && 
              (strcmp("/dev/ttyS5", argv[1])!=0) && 
    	      (strcmp("/dev/ttyS1", argv[1])!=0) && 
    	      (strcmp("/dev/ttyS0", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
      }


    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */

      fd = open(porta, O_RDWR | O_NOCTTY );

      if (fd <0) {perror(porta); exit(-1); }

      if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
      }
   
      if(newConfig(fd) != 0){
        perror("tcsetattr");
        exit(-1);
      }
      
      //send SET
      if(prepare_set(fd) != 0){

      }

      

     //write
      printf("Write something: ");
      strcpy(buf,"");
      gets(buf);

      res = write(fd,buf,strlen(buf) + 1);  
 
      printf("%d bytes written\n", res);
   

    //read
      strcpy(buf1,"");
      strcpy(rec,"");
      while (STOP == FALSE) {  
             
          res = read(fd,rec,1);  
          if(res != -1){
              strncat(buf1,rec,res);
              if (rec[res-1]=='\0') STOP = TRUE;
	      else rec[res]=0;
          }
      }

      printf("Received: %s\n", buf1);

      sleep(5);

      if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }




      close(fd);
      return 0;
  }
