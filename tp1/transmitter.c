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
#include "transmitter.h"

volatile int STOP_REC=FALSE;

int re_send = 0, re_send_ti = 0, re_send_tf = 0, flag_al = 1;

void alarm_handler() {
    flag_al = 1;
    re_send++;
}

int send_final_ua() {
    unsigned char UA[5];
    UA[0] = FLAG;
    UA[1] = A_SEND;
    UA[2] = C_UA;
    UA[3] = UA[1]^UA[2];
    UA[4] = FLAG;

    //SEND UA

    write(appLayer.fd,UA,6);
    printf("Sent response!\n");

    return 0;
}

int prepare_send_final_ua() {
    printf("\nWaiting and then sending FINAL UA\n");
    sleep(3);
    re_send = 0;

    while (re_send < ll.numTransmissions) {
        if(flag_al) {
            alarm(ll.timeout);
            printf("SENDING UA n%d!\n",(re_send+1));
            flag_al=0;
            send_final_ua();
        }
        else
            break;
    }

    return closeConfig();
}

int receive_disc() {
    char buf[1];
    int res;
    int count = 0;
    printf("Receiving DISC...\n");

    while (1) {
        res = read(appLayer.fd,buf,1);

        if(res == 0)
            return -1;

        switch(count) {
            case 0:
            if(buf[res-1] == FLAG)
                count++;
            break;

            case 1:
            if(buf[res-1] == A_REC)
                count++;
            else if(buf[res-1] == FLAG)
                break;
            else
                count = 0;
            break;

            case 2:
            if(buf[res-1] == C_DISC)
                count++;
            else if(buf[res-1] == FLAG)
                count=1;
            else
                count = 0;
            break;

            case 3:
            if(buf[res-1] == (A_REC^C_DISC))
                count++;
            else if(buf[res-1] == FLAG)
                count=1;
            else
                count = 0;
            break;
            case 4:
            if(buf[res-1] == FLAG)
                return 0;
            else
                count = 0;
            break;
        }
    }

    return -1;
}


int send_disc() {
    char DISC[5];
    DISC[0] = FLAG;
    DISC[1] = A_SEND;
    DISC[2] = C_DISC;
    DISC[3] = DISC[1]^DISC[2];
    DISC[4] = FLAG;
    write(appLayer.fd,DISC, 6);

    return receive_disc();
}

int prepare_send_disc() {

    int returnInt;

    printf("Waiting and then sending DISC\n");
    sleep(3);
    re_send = 0;

    while (re_send < ll.numTransmissions) {
        if(flag_al) {
            alarm(ll.timeout);
            printf("SENDING DISC n%d!\n",(re_send+1));
            flag_al=0;

            if((returnInt=send_disc()) != 0)
                printf("Error: no acknowledgment received from data sending (DISC)!\n");
            else 
                break;
        }
    }

    return prepare_send_final_ua();
}

int byte_stuffing(char* seq, int seqNum) {
    int count1 = 0;
    int count2 = 4;

    strcpy(ll.frame,"");

    ll.frame[0] = FLAG;
    ll.frame[1] = A_SEND;
    ll.frame[2] = C_SI;
    ll.frame[3] = ll.frame[1]^ll.frame[2];

    while (count1 < seqNum) {
   
        if(seq[count1] == FLAG) {
            ll.frame[count2] = ESC;
            count2++;
            ll.frame[count2] = 0x5e;
            count2++;                            
        }
        else if(seq[count1] == ESC) {
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
    printf("Stuffed %d bytes!\n",count2+2);
    ll.sequenceNumber = count2+2;

    return 0;
}


int receive_RR(int control) {

	char buf[1];
	int res;
	int count = 0;

    if(control == 0) {
        while (1) {
	        res = read(appLayer.fd,buf,1);

	        if(res == 0)
                return -1;

            switch(count) {
                case 0:
                if(buf[res-1] == FLAG)
                    count++;
                break;

                case 1:
                if(buf[res-1] == A_REC)
                    count++;
                else if(buf[res-1] == FLAG)
                    break;
                else
                    count=0;
                break;

                case 2:
                if(buf[res-1] == C_RRI)
                    count++;            
                else if(buf[res-1] == FLAG)
                    count=1;
                else
                    count=0;
                break;

                case 3:
                if(buf[res-1] == (C_RRI^A_REC))
                    count++;
                else if(buf[res-1] == FLAG)
                    count=1;
                else
                    count=0;
                break;

                case 4:
                if(buf[res-1] == FLAG)
                    return 0;
                else
                    count=0;
                break;
            }
        }
    }

    else if(control == 1) {
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
                if(buf[res-1] == A_REC)
                    count++;
                else if(buf[res-1] == FLAG)
                    break;
                else
                    count=0;
                break;

                case 2:
                if(buf[res-1] == C_RRF)
                    count++;
                else if(buf[res-1] == FLAG)
                    count=1;
                else
                    count=0;
                break;

                case 3:
                if(buf[res-1] == (C_RRF^A_REC))
                    count++;
                else if(buf[res-1] == FLAG)
                    count=1;
                else
                    count=0;
                break;

                case 4:
                if(buf[res-1] == FLAG)
                    return 0;
                else
                    count=0;
                break;
            }
        }
    }

    return -1;
}


int send_inf(int control) {
    printf("Sending %d bytes!\n",ll.sequenceNumber);
    write(appLayer.fd,ll.frame,ll.sequenceNumber);

    return receive_RR(control);
}


int prepare_inf(char* inf, int seqNum) {
    byte_stuffing(inf, seqNum);
    int returnInt = -2;
    printf("Waiting and then sending TI\n");
    sleep(ll.timeout);
    re_send=0;

    while (re_send < ll.numTransmissions) {
        if(flag_al) {
            alarm(ll.timeout);
            printf("SENDING DATA n%d!\n",(re_send+1));
            flag_al=0;
            if((returnInt=send_inf(0)) != 0) {
                printf("Error: no acknowledgment received from data sending (TI)!\n");
            }
            else
                break;
        }
    }
  
    if(returnInt != 0 )
        printf("Error during data sending\n");
    else {
        printf("Waiting and then sending TF\n");
        sleep(ll.timeout);
        ll.frame[2] = C_SF;
        ll.frame[3] = C_SF^A_SEND;
        ll.frame[ll.sequenceNumber-2] = ll.frame[3];
        re_send = 0;

        while (re_send < ll.numTransmissions) {
            if(flag_al) {
                alarm(ll.timeout);
                printf("SENDING DATA n%d!\n",(re_send+1));
                flag_al=0;

                if((returnInt=send_inf(1)) != 0) {
                    printf("Error: no acknowledgment received from data sending (TF)!\n");
                    if(returnInt == 1) {
                        printf("Data received was not the same, trying again!\n");
                        return prepare_inf(inf,seqNum);
                    }
                }
                else 
                    break;
            }
        }
    }
    
    if(returnInt == 0)
        printf("Sent information data successfully!\n\n");
    
    return returnInt;
}

int receive_ua() {
    int res; 
    char buf[MAX_SIZE];
    int count = 0;

    strcpy(buf,"");

    while (STOP_REC == FALSE) {
        res = read(appLayer.fd,buf,1);

        if(res == 0)
            return -1;

        switch(count) {
            case 0:
            if(buf[res-1] == FLAG)
                count++;
            break;

            case 1:
            if(buf[res-1] == A_REC)
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
            if(buf[res-1] == (C_UA^A_REC))
                count++;
            else if(buf[res-1] == FLAG)
                count=1;
            else
                count=0;
            break;

            case 4:
            if(buf[res-1] == FLAG)
                return 0;
            else
                count=0;
            break;
        }
    }

    return -1;
}


int send_set() {
    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A_SEND;
    SET[2] = C_SET;
    SET[3] = SET[1]^SET[2];
    SET[4] = FLAG;

    //SEND SET
    write(appLayer.fd,SET,6);
    printf("Sent SET\n");

    //RECEIVE UA
    return receive_ua();
}


int prepare_set() {

    int returnInt;
    while (re_send < ll.numTransmissions) {
        if(flag_al) {
            alarm(ll.timeout);
            printf("SENDING n%d!\n",(re_send+1));
            flag_al=0;

            if((returnInt=send_set()) != 0){
                printf("Error: no acknowledgment received (UA)!\n");
            }
            else
                break;
        }
    }

    return returnInt;
}



int saveConfig() {

    (void) signal(SIGALRM, alarm_handler);
    appLayer.fd = open(ll.port, O_RDWR | O_NOCTTY );

    if (appLayer.fd <0) {
        perror(ll.port);
        exit(-1);
    }

    if ( tcgetattr(appLayer.fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    return newConfig();
}

int newConfig() {

    struct termios newtio;
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = ll.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = ll.timeout * 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;   /* blocking read until 5 chars received */

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
    return prepare_set();
}


int closeConfig() {

    printf("Ending program!\n");
    sleep(5);

    if ( tcsetattr(appLayer.fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(appLayer.fd);
    
    return 0;
}
