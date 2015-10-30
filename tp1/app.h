#ifndef APP_H
#define APP_H

int checkFrames(char * buf);
int create_packets_send();
int send_rr_app(int equalize, int segmentNumber, char* buf);
int checkControl(int equalize);
int main (int argc, char ** argv);

#endif
