#ifndef FILEOPP_H
#define FILEOPP_H

#define C_START 0x01
#define C_END 0x02
#define C_SIZE_FILE 0x00
#define C_NAME_FILE 0x01
#define SIZE 0x04
#define C_DATA 0x00

#define MAX_SIZE 256
#define MAX_SIZE_DATA 230


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
