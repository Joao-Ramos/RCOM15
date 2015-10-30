#ifndef RECEIVER_H
#define RECEIVER_H


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
#define C_DISC 0x0b


struct termios oldtio;

int receive_ua_nc();
int send_disc_nc();
int send_rr(int equalize, char * buf);
int send_ua();
int receive_inf(int control);
int saveConfigNC();
int newConfigNC();
int closeConfigNC();

#endif
