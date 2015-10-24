#ifndef WRITENONCANONICAL_H
#define WRITENONCANONICAL_H

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
#define C_DISC 0x0b

#define MAX_SIZE 255

struct termios oldtio;

struct applicationLayer {

	int fd;	/*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/

};


struct linkLayer {
	
	char port[20];	/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão*/
	unsigned int sequenceNumber;   /*Número de bytes após stuffing */
	unsigned int timeout;	/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE*2];	/*Trama*/

};

//writenoncanonical.c methods

int send_final_ua();
int receive_disc();
int send_disc();
int prepare_send_disc();
int byte_stuffing(char* seq);
int receive_RR(int control);
int send_inf(int control);
int prepare_inf(char* inf);
int send_set();
int receive_ua();
int prepare_set();
int saveConfig();
int newConfig();
int closeConfig();

//nc.c methods
int receive_ua_nc();
int send_disc_nc();
int send_rr(int control);
int send_ua();
int receive_inf(int control);
int saveConfigNC();
int newConfigNC();
int closeConfigNC();

struct applicationLayer appLayer;
struct linkLayer ll;

#endif
