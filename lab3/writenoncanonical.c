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

  int re_send = 0, flag_al = 1;

  void alarm_handler(){

     flag_al = 1;
     re_send++;
  }
  
  int byte_stuffing(char* seq){
     
 
      int res;
      int count1 = 0;
      int count2 = 4;

      strcpy(ll.frame,"");

      ll.frame[0] = FLAG;
      ll.frame[1] = A_SEND;
      ll.frame[2] = C_SI;
      ll.frame[3] = ll.frame[1]^ll.frame[2];

      while (count1 < strlen(seq)) {
         printf("%x\n",seq[count1]);
         if(seq[count1] == FLAG){
               ll.frame[count2] = ESC;
               count2++;
	             ll.frame[count2] = 0x5e;
               count2++;                            
	       }
         else if(seq[count1] == ESC){
               ll.frame[count2] = ESC;
	             count2++;
               ll.frame[count2] = 0x5d;
               count2++;
         }
         else{
               ll.frame[count2] = seq[count1];
               count2++;
         }
         count1++;           
      }

      ll.frame[count2] = ll.frame[3];
      ll.frame[count2+1] = FLAG;
      printf("Stuffed count2+1 bytes!\n");

         
      return 0;    

  }
 

  
  int receive_RR(int control){

	char buf[1];
	int res;

	if(control == 0) {
		while (1) {
			res = read(appLayer.fd,buf,1);
		
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
			    if(buf[res-1] == C_RRI)
			       count++;
			    else if(buf[res-1] == FLAG)
			       count=1;
			    else count=0;
			    break;
			case 3:
			    if(buf[res-1] == (C_RRI^A_SEND))
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


	}

	else if(control == 1){
		while (1) {
			res = read(appLayer.fd,buf,1);
		
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
			    if(buf[res-1] == C_RRF)
			       count++;
			    else if(buf[res-1] == FLAG)
			       count=1;
			    else count=0;
			    break;
			case 3:
			    if(buf[res-1] == (C_RRF^A_SEND))
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


  }

	return -1;


  }



  int send_inf(int control){
    printf("Sending information!\n");
    write(appLayer.fd,ll.frame,strlen(ll.frame));
    if(control == 0)
      return receive_RR(0);
    else
      return receive_RR(1);

  }

  int prepare_inf(char* inf){     

     byte_stuffing(inf);

     int returnInt = -2;
     re_send=0;
     while (re_send < ll.numTransmissions){
        if(flag_al){
           alarm(ll.timeout);
           printf("SENDING DATA n%d!\n",(re_send+1));
           flag_al=0;
           if((returnInt=send_inf(0)) != 0){
             printf("Error: no acknowledgment received from data sending (TI)!\n");
           }
           else break;
         
      }
    }
    if(returnInt != 0 ){
      printf("Error during data sending\n");

    }
    else{
      ll.frame[2] = C_SF;
      ll.frame[3] = C_SF^A^SEND;
      re_send = 0;
      while (re_send < ll.numTransmissions){
        if(flag_al){
           alarm(ll.timeout);
           printf("SENDING DATA n%d!\n",(re_send+1));
           flag_al=0;
           if((returnInt=send_inf(1)) != 0){
             printf("Error: no acknowledgment received from data sending (TF)!\n");
           }
           else break;
         
        }
      }


    }
    if(returnInt == 0)
      printf("Sent information data successfully!\n");

    return returnInt;



  }

   int receive_ua(){

      int res; 
      char buf[255];
      int count = 0;

      strcpy(buf,"");
      while (STOP_REC == FALSE) {
        res = read(appLayer.fd,buf,1);
        
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

  int send_set(){

    
    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A_SEND;
    SET[2] = C_SET;
    SET[3] = SET[1]^SET[2];
    SET[4] = FLAG;

    //SEND SET
    write(appLayer.fd,SET_STUF,5);
    printf("Sent SET\n");

    //RECEIVE UA
     
    return receive_ua();
   

  }


  int prepare_set(){

    int returnInt;
    while (re_send < ll.numTransmissions){
        if(flag_al){
           alarm(ll.timeout);
           printf("SENDING n%d!\n",(re_send+1));
           flag_al=0;
           if((returnInt=send_set()) != 0){
             printf("Error: no acknowledgment received (UA)!\n");
           }
           else break;
         
      }
    }
    return returnInt;

  }



  int saveConfig(){
      int res;
      

      char buf[MAX_SIZE*2];
      char buf1[MAX_SIZE];
      char rec[MAX_SIZE];

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

      appLayer.fd = open(ll.port, O_RDWR | O_NOCTTY );

      if (appLayer.fd <0) {perror(ll.port); exit(-1); }

      if ( tcgetattr(appLayer.fd,&oldtio1) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
      }
   
      /*if(newConfig() != 0){
        perror("tcsetattr");
        exit(-1);
      }
      
      //send SET
      if(prepare_set() != 0){
	printf("Error: no aknowledgment received after %d tries! (UA) Shuting down...\n", ll.numTransmissions);
	return closeConf(appLayer.fd);
      }

      

     //write
      printf("Write something: ");
      strcpy(buf,"");
      gets(buf);
      byte_stuffing(buf);
      res = write(appLayer.fd,buf,strlen(buf) + 1); 
 
      printf("%d bytes written\n", res);
   

    //read
      strcpy(buf1,"");
      strcpy(rec,"");
      while (STOP == FALSE) {  
             
          res = read(appLayer.fd,rec,1);  
          if(res != -1){
              strncat(buf1,rec,res);
              if (rec[res-1]=='\0') STOP = TRUE;
	      else rec[res]=0;
          }
      }

      printf("Received: %s\n", buf1);
    */
      return 0;
  }

  int newConfig(){

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
      leitura do(s) próximo(s) caracter(es)
    */



      tcflush(appLayer.fd, TCIOFLUSH);
      if ( tcsetattr(appLayer.fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }

      printf("New termios structure set\n");

      return 0;
  }



  int closeConfig(){

      sleep(5);

      if ( tcsetattr(appLayer.fd,TCSANOW,&oldtio1) == -1) {
        perror("tcsetattr");
        exit(-1);
      }

      close(appLayer.fd);
      return 0;


  }