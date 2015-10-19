#ifndef WRITENONCANONICAL_H
#define WRITENONCANONICAL_H

#define MAX_SIZE 255

struct applicationLayer {

	int fileDescriptor;	/*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/

};


struct linkLayer {
	
	char port[20];	/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão*/
	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	unsigned int timeout;	/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE];	/*Trama*/

};

int openConf(char * porta);
int newConfig(int fd);
int send_set(int fd);
int receive_ua(int fd);
int prepare_set(int fd);

int ola(void);

struct applicationLayer appLayer;
struct linkLayer ll;

#endif
