#ifndef OPENPICTURE_H
#define OPENPICTURE_H

#include "../include/Utility.h"
#include "../include/CodingUnitStructure.h"


#if N2_BUILD

#include <system.h>
#include "Altera_UP_SD_Card_Avalon_Interface.h"

#define de2Input (char *) RASP_INPUT_BASE			// 8 bits of GPIO input from PI
#define de2Output (char *)RASP_OUTPUT_BASE			// 8 bits of GPIO output to PI
#define recieveMode (char *) RECIEVE_MODE_BASE		// Signal input from PI, indicator for DE2 that it will receive from PI and run receiving algorithm
#define sendMode (char *) SEND_MODE_BASE			// Signal output to PI, indicator to PI that it will reeive from DE2 and run its receiving algorithm
#define WRITE (char *) READY_SEND_BASE				// Signal output to PI, indicator to PI that DE2 is ready to receive new data
#define READ (char *) READY_READ_BASE				// Signal input from PI, indicator for DE2 that new data from PI ready to be read
#define BUFFERSIZE PICTURE_YUV420_SIZE

void Receive(unsigned char* receive);
void Send(unsigned char* send);

#endif

void OpenYUVFileIntoInputPicture(
	BufferDescriptor_t* inputPicture, 
	const char* filename, 
	int width, 
	int height);

void SaveYUVToFile(
	const char* filename,
	BufferDescriptor_t *outputPicture);

void OpenSerialYUVIntoInputPicture(
	BufferDescriptor_t* inputPicture, 
	unsigned char* receivedBuffer, 
	int width, 
	int height);

void OpenReconBestIntoSerialYUV(
	BufferDescriptor_t* reconBest, 
	unsigned char* sendBuffer, 
	int width, 
	int height);

void SetYUVSamplesToValue(BufferDescriptor_t *inputPicture);




#endif