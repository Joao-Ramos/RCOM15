#ifndef INTERFACE_H
#define INTERFACE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


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

#define C_START 0x01
#define C_END 0x02
#define C_TAM_FILE 0x00
#define C_LEN_FILE 0x01
#define SIZE 0x08
#define C_DATA 0x00

#define MAX_SIZE 256
#define MAX_SIZE_DATA 252

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

struct data{

	unsigned long dataLength;
	char * data; 
	unsigned int numSeg;

};

struct controlData{

	unsigned long fileLength;
	char * filePath;
	unsigned int fpLength;
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

//fileReader.c methods
int readData(char* filePath);
char* createCtrlPackets(int control);
char* createDataPacket(int segment, int size);

struct applicationLayer appLayer;
struct linkLayer ll;
struct data fileData;
struct controlData ctrData;

#endif
