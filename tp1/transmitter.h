#ifndef TRANSMITTER_H
#define TRANSMITTER_H


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

int send_final_ua();
int receive_disc();
int send_disc();
int prepare_send_disc();
int byte_stuffing(char* seq,int seqNum);
int receive_RR(int control);
int send_inf(int control);
int prepare_inf(char* inf, int seqNum);
int send_set();
int receive_ua();
int prepare_set();
int saveConfig();
int newConfig();
int closeConfig();


#endif