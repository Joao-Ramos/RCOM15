#ifndef FILEOPP_H
#define FILEOPP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


struct data{

	unsigned long dataLength;
	unsigned char * data; 
	unsigned int numSeg;
	char frame[MAX_SIZE*2];
	unsigned int sequenceNumber;

};

struct controlData{

	unsigned long fileLength;
	char filePath[MAX_SIZE_DATA];
	unsigned int fpLength;
	char frame[MAX_SIZE*2];
	unsigned int sequenceNumber;
};

int readData();
int createCtrlPackets(int control);
int createDataPacket(int segment);
int saveChunk(char* buf, int sequenceNumber);

struct data fileData;
struct controlData ctrData;

#endif