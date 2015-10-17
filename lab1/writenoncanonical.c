  /*Non-Canonical Input Processing*/

  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <unistd.h>

  #define BAUDRATE B9600
  #define MODEMDEVICE "/dev/ttyS4"
  #define _POSIX_SOURCE 1 /* POSIX compliant source */
  #define FALSE 0
  #define TRUE 1



  volatile int STOP=FALSE;

  
  int main(int argc, char** argv)
  {
      int fd, res;
      struct termios oldtio,newtio;
      char buf[255];
      char buf1[255];
      char rec[255];
      
      if ( (argc < 2) || 
    	     ((strcmp("/dev/ttyS4", argv[1])!=0) && 
              (strcmp("/dev/ttyS5", argv[1])!=0) && 
    	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
      }


    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */

      fd = open(argv[1], O_RDWR | O_NOCTTY );

      if (fd <0) {perror(argv[1]); exit(-1); }

      if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
      }
      bzero(&newtio, sizeof(newtio));
      newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
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



      tcflush(fd, TCIOFLUSH);
      if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }

      printf("New termios structure set\n");


     //write
      gets(buf);

      res = write(fd,buf,strlen(buf) + 1);  
 
      printf("%d bytes written\n", res);
   

    //read
      strcpy(buf1,"");
      strcpy(rec,"");
      while (STOP == FALSE) {  
             
          res = read(fd,rec,1);  
          strncat(buf1,rec,res);
          if (rec[res-1]=='\0') STOP = TRUE;
	  else rec[res]=0;
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
