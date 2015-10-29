#ifndef APP_H
#define APP_H


#define C_START 0x01
#define C_END 0x02
#define C_SIZE_FILE 0x00
#define C_NAME_FILE 0x01
#define SIZE 0x04
#define C_DATA 0x00

int checkFrames(char * buf);
int create_packets_send();
int send_rr(int equalize, int segmentNumber, char* buf);
int checkControl(int equalize);
int main (int argc, char ** argv);

#endif