#ifndef WRITENONCANONICAL_H
#define WRITENONCANONICAL_H

#define MAX_SIZE 255

struct termios oldtio;

struct applicationLayer {

	int fd;	/*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/

};


struct linkLayer {
	
	char port[20];	/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão*/
	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	unsigned int timeout;	/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE*2];	/*Trama*/

};

int byte_stuffing(char* seq);
int receive_RR(int control);
int send_inf(int control);
int prepare_inf(char* inf);
int send_set();
int receive_ua();
int prepare_set();
int saveConfig(char* porta);
int newConfig();
int closeConfig();

struct applicationLayer appLayer;
struct linkLayer ll;

#endif
