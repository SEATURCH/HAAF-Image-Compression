// ImageCompression.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Utility.h"
#include "CodingUnitStructure.h"
#include "Transform.h"
#include "Encode.h"


#include <time.h>
#include <system.h>
#include "Altera_UP_SD_Card_Avalon_Interface.h"

#define de2Input (char *) RASP_INPUT_BASE			// 8 bits of GPIO input from PI
#define de2Output (char *)RASP_OUTPUT_BASE			// 8 bits of GPIO output to PI
#define recieveMode (char *) RECIEVE_MODE_BASE		// Signal input from PI, indicator for DE2 that it will receive from PI and run receiving algorithm
#define sendMode (char *) SEND_MODE_BASE			// Signal output to PI, indicator to PI that it will reeive from DE2 and run its receiving algorithm
#define WRITE (char *) READY_SEND_BASE			// Signal output to PI, indicator to PI that DE2 is ready to receive new data
#define READ (char *) READY_READ_BASE			// Signal input from PI, indicator for DE2 that new data from PI ready to be read
#define BUFFERSIZE PICTURE_YUV420_SIZE
#define TOTAL_KB 1
#define DEBUG 0

//Parse the yuv file
void OpenYUVFileIntoInputPicture(BufferDescriptor_t* inputPicture, const char* filename, int width, int height){
	FILE *file_handler;
	file_handler = fopen(filename,"rb");
	if(!file_handler) {
		printf("Could not open the file.\n");
	}

	fseek(file_handler, 0, SEEK_SET);

	//Reading Y values
	fread(inputPicture->yBuffer,sizeof(char), width*height, file_handler);

	//Reading U values
	fread(inputPicture->uBuffer,sizeof(char), (width >> 1)*(height >> 1), file_handler);

	//Reading V values
	fread(inputPicture->vBuffer,sizeof(char), (width >> 1)*(height >> 1), file_handler);

	fclose(file_handler);

}

//Parse the yuv file
void OpenSerialYUVIntoInputPicture(BufferDescriptor_t* inputPicture, unsigned char* receivedBuffer, int width, int height){
	int y_size;
	int uv_size;

	int y_offset;
	int u_offset;
	int v_offset;

	//Sizes of the YUV portions
	y_size = width*height;
	uv_size = (width >> 1) * (height >> 1);

	//Offsets for the YUV in the receievedBuffer
	y_offset=0;
	u_offset=y_size;
	v_offset=u_offset + uv_size;


	//Reading Y values
	memcpy(inputPicture->yBuffer, receivedBuffer+y_offset, y_size);

	//Reading U values
	memcpy(inputPicture->uBuffer, receivedBuffer+u_offset, uv_size);

	//Reading V values
	memcpy(inputPicture->vBuffer, receivedBuffer+v_offset, uv_size);


}

void OpenReconBestIntoSerialYUV(BufferDescriptor_t* reconBest, unsigned char* sendBuffer, int width, int height){
	int y_size;
	int uv_size;

	int y_offset;
	int u_offset;
	int v_offset;

	//Sizes of the YUV portions
	y_size = width*height;
	uv_size = (width >> 1) * (height >> 1);

	//Offsets for the YUV in the receievedBuffer
	y_offset=0;
	u_offset=y_size;
	v_offset=u_offset + uv_size;


	//Writing Y values
	memcpy(sendBuffer+y_offset, reconBest->yBuffer, y_size);

	//Writing U values
	memcpy(sendBuffer+u_offset, reconBest->uBuffer, uv_size);

	//Writing V values
	memcpy(sendBuffer+v_offset, reconBest->vBuffer, uv_size);


}

// Used for simulated picture
void SetYUVSamplesToValue(BufferDescriptor_t *inputPicture)
{
	SetInputPictureSamplesToArbitrary(inputPicture->yBuffer, inputPicture->yStride, inputPicture->yHeight);
	SetInputPictureSamplesToArbitrary(inputPicture->uBuffer, inputPicture->uStride, inputPicture->uHeight);
	SetInputPictureSamplesToArbitrary(inputPicture->vBuffer, inputPicture->vStride, inputPicture->vHeight);
}


void Receive(unsigned char* receive) {
	int i = 0, index = 0;
	while(i < TOTAL_KB){
		while (index < BUFFERSIZE) {

		//		if(DEBUG)printf("%d [waiting for data to be written...]\n", index);
				while(*READ != 0x1);
				receive[index] = 0x0;
		//		if(DEBUG)printf("[Receiving...]");
				receive[index] = (*de2Input);
		//		if(DEBUG)printf(" %c\n", receive[index]);
				(*WRITE) = 0x1;		// Indicates to PI that DE2 is ready to recieve NEW data
				while(*READ != 0x0);
				*WRITE = 0x0;
				index++;
		}
	index = 0;
	i++;
	}
}

// DE2 Sending, PI Reading
void Send(unsigned char* send) {
	int i = 0, index = 0;
	while(i < TOTAL_KB){
		while (index < BUFFERSIZE) {
			while(*READ != 0x0);
			(*de2Output) = send[index];
			*WRITE = 0x1;
		//	if(DEBUG)printf("[waiting for %c to be read...]\n", send[index]);
			while(*READ != 0x1);
			*WRITE = 0x0;
			index++;
		}
		index = 0;
		i++;
	}
}


void writeToYuv(unsigned char* received ){
	FILE *fp;
   fp = fopen("Cats_320x240_420.yuv", "w+");
   fprintf(fp, "%s", received);
   fclose(fp);
}


/*** MAIN ***/
int main(int argc, char* argv[])
{
	unsigned char receivedBuffer[BUFFERSIZE];
	unsigned char sendBuffer[BUFFERSIZE];
	*WRITE = 0x0;
    printf("Receiving \n");
	Receive(receivedBuffer);
	//writeToYuv(receivedBuffer);
	//Send(sendBuffer);
	printf("Recieve done \n");
	CodingUnitStructure_t codingUnitStructure;

	/*** INITIALIZATION ***/

	// Coding Unit Structure Initializes the Coding Unit's and Input Pictures
	CodingUnitStructureConstructor(&codingUnitStructure, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);

	// Open Arbitrary Picture
	//OpenYUVFileIntoInputPicture(&codingUnitStructure.inputPicture, "Cats_320x240_420.yuv", DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
	OpenSerialYUVIntoInputPicture(&codingUnitStructure.inputPicture, receivedBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);

	/*** MAIN ALGORITHM ***/
	printf("Encoding\n");
	// Encode the image
	EncodeLoop(&codingUnitStructure);
	printf("Encode Done\n");
	
	// Write the image back out to file
	{
		FILE* file_handler;
		int bytesWritten;

		unsigned char blank[320*240] = {128};

		//file_handler = fopen("C:\\ReconCats_320x240_420.yuv","wb");

		//if(file_handler == NULL)
		//{
		//	printf("Could not open file for output!!\n");
		//	return 0;
		//}
		//bytesWritten = fwrite(codingUnitStructure.inputPicture.yBuffer, 1, codingUnitStructure.inputPicture.yHeight * codingUnitStructure.inputPicture.yStride, file_handler);
		//bytesWritten = fwrite(codingUnitStructure.reconBestBuffer.yBuffer, 1, codingUnitStructure.reconBestBuffer.yHeight * codingUnitStructure.reconBestBuffer.yStride, file_handler);
		//if(!bytesWritten) printf("1WRITE ERROR!\n");
		//
		//bytesWritten = fwrite(codingUnitStructure.inputPicture.uBuffer, sizeof(char), codingUnitStructure.inputPicture.uHeight * codingUnitStructure.inputPicture.uStride, file_handler);
		//bytesWritten = fwrite(codingUnitStructure.reconBestBuffer.uBuffer, sizeof(char), codingUnitStructure.reconBestBuffer.uHeight * codingUnitStructure.reconBestBuffer.uStride, file_handler);
		//if(!bytesWritten) printf("2WRITE ERROR!\n");
		//
		//bytesWritten = fwrite(codingUnitStructure.inputPicture.vBuffer, sizeof(char), codingUnitStructure.inputPicture.vHeight * codingUnitStructure.inputPicture.vStride, file_handler);
		//bytesWritten = fwrite(codingUnitStructure.reconBestBuffer.vBuffer, sizeof(char), codingUnitStructure.reconBestBuffer.vHeight * codingUnitStructure.reconBestBuffer.vStride, file_handler);
		//if(!bytesWritten) printf("3WRITE ERROR!\n");
		//
		////Write the quantized output back to the PI via a serial stream
		//fclose(file_handler);
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		
	}


	/*** DECONSTRUCTION ***/
	CodingUnitStructureDeconstructor(&codingUnitStructure);
    printf("Done\n");

    printf("Sending\n");
    Send(sendBuffer);
    printf("Send Done\n");
	return 0;
}

