
#include "interface.h"

int readData(char* filepath){


	FILE *my_file;
	unsigned int i;

	my_file = fopen (filepath, "r");
	if (my_file == NULL)
	{
		printf ("File not found!\n");
		printf ("Or filepath is wrong!\n");
		return -1;
	}
	printf("Reading file and saving into a data struct\n");

	fseek (my_file, 0, SEEK_END);
	fileData.dataLength = ftell (my_file);
	ctrData.fileLength = fileData.dataLength;
	fseek (my_file, 0, SEEK_SET);
	
	fileData.data = (unsigned char *) malloc (sizeof(unsigned char) * fileData.dataLength);

	if(fileData.dataLength % MAX_SIZE_DATA == 0)
		fileData.numSeg = fileData.dataLength/MAX_SIZE_DATA;
	else fileData.numSeg = fileData.dataLength/MAX_SIZE_DATA + 1;

	printf("Num segments: %d\n", fileData.numSeg);
	printf("Length of file: %d\n", fileData.dataLength);

	for (i = 0; i < fileData.dataLength; i++)
		fileData.data[i] = getc(my_file);

	fclose (my_file);

	return 0;

}

char* createCtrlPackets(int control){

	int size = 5 + ctrData.fpLength + 8;
	int count = 0;
	int i = 0;

	char lengthValue[8];
	char* str[size];
	char fpLength;

	snprintf (lengthValue, 8, "%lX", ctrData.fileLength);

	printf("createCtrlPackets: lengthValue: %x\n",lengthValue);
	printf("createCtrlPackets: fpLength: %x\n",fpLength);

	if(control == 0) str[count] = C_START;
	else str[count] = C_END;
	count++;

	str[count] = C_TAM_FILE;
	count++;
	str[count] = SIZE;
	count++;

	for(i = 0; i < 8; i++){
		str[count] = lengthValue[i];
		count++;
	}

	str[count] = C_LEN_FILE;
	count++;
	str[count] = (unsigned char) ctrlData.fpLength;

	for(i = 0; i < strlen(ctrData.filePath); i++){
		str[count] = crtData.filePath[i];
	}

	return str;
}

char* createDataPacket(int segment){

	char seg;
	int count = 0;
	int i;

	if(segment == fileData.numSeg){

		int size = fileData.dataLength % MAX_SIZE_DATA;

	}
	else int size = MAX_SIZE_DATA;
	
	char * str[size+4];
	char L1 = (unsigned char) size,L2 = 0x00;

	str[count] = C_DATA;
	count++;

	
	printf("createDataPackets: segment: %x\n",segment);

	str[count] = (unsigned char) segment;
	count++;

	str[count] = L2;
	count++;
	str[count] = L1;
	count++;

	for(i = 0; i < size ; i++){

		str[count] = fileData[i+(segment-1) * MAX_SIZE_DATA];
		count++;
	}

	return str;
}

