#ifndef INTERFACE_H
#define INTERFACE_H

#define _POSIX_SOURCE 1 // POSIX compliant source 
#define FALSE 0
#define TRUE 1

#define MAX_SIZE 256
#define MAX_SIZE_DATA 230


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
	char compFrame[MAX_SIZE*2]; /*Trama para comparar*/ //TESTAR 

};


int checkFrames(char*buf);

int llopen();
int llread(int control);
int llwrite(int control, char * buf, int seqNum);
int llclose();

struct applicationLayer appLayer;
struct linkLayer ll;



#endif
