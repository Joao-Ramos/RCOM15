/*Non-Canonical Input Processing*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "interface.h"
#include "receiver.h"

unsigned int segmentNumber;

int receive_ua_nc() {
    int res; 
    char buf[MAX_SIZE];
    int count = 0;

    strcpy(buf,"");
    
    while (1) {
        res = read(appLayer.fd,buf,1);

        switch(count) {
            case 0:
            if(buf[res-1] == FLAG)
               count++;
            break;
            
            case 1:
            if(buf[res-1] == A_SEND)
               count++;
            else if(buf[res-1] == FLAG)
                break;
            else
                count=0;
            break;
            
            case 2:
            if(buf[res-1] == C_UA)
                count++;
            else if(buf[res-1] == FLAG)
                count=1;
            else
                count=0;
            break;
            
            case 3:
            if(buf[res-1] == (C_UA^A_SEND))
                count++;
            else if(buf[res-1] == FLAG)
                count=1;
            else
                count=0;
            break;
            
            case 4:
            if(buf[res-1] == FLAG) {
                printf("Received UA and closing down!\n");
                return 0;
            }
            else
                count=0;
            break;
        }
    }
    
    return -1;
}

int send_disc_nc() {
    char DISC[5];
    DISC[0] = FLAG;
    DISC[1] = A_REC;
    DISC[2] = C_DISC;
    DISC[3] = DISC[1]^DISC[2];
    DISC[4] = FLAG;
    write(appLayer.fd,DISC,6);

    receive_ua_nc();
    return 1;
}



int send_rr(int equalize, char* buf) {
    char RR[5];

    RR[0] = FLAG;
    RR[1] = A_REC;

    if(equalize == 0)
        RR[2] = C_RRI;
    else if (equalize == 1) {
        if(checkFrames(buf) == 0) {
            RR[2] = C_RRF;
        }
        else{
            RR[2] = C_REJ;
        }
    }

    RR[3] = RR[1]^RR[2];
    RR[4] = FLAG;

    printf("send_rr: Sending rr number %d\n\n", equalize);
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
    write(appLayer.fd,UA,5);
    printf("Sent response (UA)!\n\n");

    return 0;
}


int saveConfigNC() {
    appLayer.fd = open(ll.port, O_RDWR | O_NOCTTY );

    if (appLayer.fd <0) {
        perror(ll.port); 
        exit(-1);
    }

    if ( tcgetattr(appLayer.fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    //RECEIVE SET
    return newConfigNC();    
}


int newConfigNC() {
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

int closeConfigNC() {
    printf("Ending program!\n");

    sleep(5);

    if ( tcsetattr(appLayer.fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(appLayer.fd);

    exit(0);
}
