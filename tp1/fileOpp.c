#include "fileOpp.h"

#define MAX_SIZE 256
#define MAX_SIZE_DATA 230

int readData(){


	FILE *my_file;
	unsigned int i;

	my_file = fopen (ctrData.filePath, "r");
	if (my_file == NULL)
	{
		printf ("File not found!\n");
		printf ("Or filepath is wrong!\n");
		return -1;
	}
	printf("Reading file and saving into struct data\n");

	fseek (my_file, 0, SEEK_END);
	fileData.dataLength = ftell (my_file);
	ctrData.fileLength = fileData.dataLength;
	fseek (my_file, 0, SEEK_SET);
	
	fileData.data = (unsigned char *) malloc (sizeof(unsigned char) * fileData.dataLength);

	if(fileData.dataLength % MAX_SIZE_DATA == 0)
		fileData.numSeg = fileData.dataLength/MAX_SIZE_DATA;
	else fileData.numSeg = fileData.dataLength/MAX_SIZE_DATA + 1;

	printf("Num segments: %d\n", fileData.numSeg);
	printf("Length of file: %ld bytes\n", fileData.dataLength);

	for (i = 0; i < fileData.dataLength; i++){
		fileData.data[i] = getc(my_file);
	}

	fclose (my_file);

	return 0;

}

int createCtrlPackets(int control){

	int count = 0;
	int i = 0;

	unsigned char lengthValue[4];
	lengthValue[0] = (int)((fileData.dataLength >> 24) & 0xFF) ;
 	lengthValue[1] = (int)((fileData.dataLength >> 16) & 0xFF) ;
 	lengthValue[2] = (int)((fileData.dataLength >> 8) & 0XFF);
 	lengthValue[3] = (int)((fileData.dataLength & 0XFF));

	

	if(control == 0) ctrData.frame[count] = C_START;
	else ctrData.frame[count] = C_END;
	count++;

	ctrData.frame[count] = C_SIZE_FILE;
	count++;
	ctrData.frame[count] = SIZE;
	count++;

	for(i = 0; i < 4; i++){
		ctrData.frame[count] = lengthValue[i];
		count++;
	}

	ctrData.frame[count] = C_NAME_FILE;
	count++;
	ctrData.frame[count] = (unsigned char) ctrData.fpLength;
	count++;
	
	for(i = 0; i < ctrData.fpLength; i++){
		ctrData.frame[count] = ctrData.filePath[i];
		count++;
	}
	
	ctrData.sequenceNumber = count;
	return 0;
}

int createDataPacket(int segment){

	int count = 0;
	int i;
	unsigned int size = 0;

	if(segment == fileData.numSeg){

		size = fileData.dataLength % MAX_SIZE_DATA;

	}
	else size = MAX_SIZE_DATA;

	unsigned char L1 = (short) size%256,L2 = (short) size/256;
	
	fileData.frame[count] = C_DATA;
	count++;

	
	printf("createDataPackets: segment: %d\n",segment);

	fileData.frame[count] = (unsigned char) segment;
	count++;

	fileData.frame[count] = L2;
	count++;
	fileData.frame[count] = '\0';
	fileData.frame[count] = (char) L1;
	count++;
	printf("L1: %x\n",L1);
	printf("L1: %x\n",fileData.frame[count-1]);

	for(i = 0; i < size ; i++){
	
		fileData.frame[count] = fileData.data[i+(segment-1) * MAX_SIZE_DATA];
		count++;
	}
	fileData.sequenceNumber = count;
	return 0;
}

int saveChunk(char* buf, int sequenceNumber){

	FILE *my_file;
	unsigned int i;

	my_file = fopen (ctrData.filePath, "a");	
        if(my_file == NULL){
		printf("Creating and writing on new file with name: %s\n",ctrData.filePath);
		my_file = fopen(ctrData.filePath,"w+");
        }else printf("Updating file with name: %s\n\n",ctrData.filePath);
	
	
	//fseek(my_file,MAX_SIZE_DATA*(segment-1), SEEK_SET);

	for(i = 0; i< sequenceNumber; i++){
		putc(buf[i],my_file);
	}

	//fwrite(buf, 1, ll.sequenceNumber,my_file);
	
	//fseek(my_file,0, SEEK_SET);

	fclose(my_file);

	return 0;
}
