#include "../include/OpenPicture.h"

#include "string.h"

#if N2_BUILD
void Receive(unsigned char* receive) {
	int index = 0;
		while (index < BUFFERSIZE) {

				while(*READ != 0x1);
				receive[index] = 0x0;
				receive[index] = (*de2Input);
				(*WRITE) = 0x1;
				while(*READ != 0x0);
				*WRITE = 0x0;
				index++;
		}
}

// DE2 Sending, PI Reading
void Send(unsigned char* send) {
	int index = 0;
		while (index < BUFFERSIZE) {
			while(*READ != 0x0);
			(*de2Output) = send[index];
			*WRITE = 0x1;
			while(*READ != 0x1);
			*WRITE = 0x0;
			index++;
		}
}

#endif

//Parse the yuv file
void OpenYUVFileIntoInputPicture(
	BufferDescriptor_t* inputPicture, 
	const char* filename, 
	int width, 
	int height)
{

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

void SaveYUVToFile(
	const char* filename,
	BufferDescriptor_t *outputPicture)
{
	FILE* file_handler;
	int bytesWritten;

	file_handler = fopen(filename,"wb");
	if(file_handler == NULL)
	{
		printf("Could not open file for output!!\n");
		return;
	}
		
	// Write Y
	bytesWritten = fwrite(outputPicture->yBuffer, 1, outputPicture->yHeight * outputPicture->yStride, file_handler);
	if(!bytesWritten) printf("Y WRITE ERROR!\n");
		
	// Write U
	bytesWritten = fwrite(outputPicture->uBuffer, sizeof(char), outputPicture->uHeight * outputPicture->uStride, file_handler);
	if(!bytesWritten) printf("U WRITE ERROR!\n");
		
	// Write V
	bytesWritten = fwrite(outputPicture->vBuffer, sizeof(char), outputPicture->vHeight * outputPicture->vStride, file_handler);
	if(!bytesWritten) printf("V WRITE ERROR!\n");
		
	//Write the quantized output back to the PI via a serial stream
	fclose(file_handler);
}

void OpenDataIntoSerialData(
		unsigned char *dst,
		int dstLength,
		unsigned char *src,
		int srcLength)
{
	unsigned char *dstCursor = dst;

	// Check header overflow
	if(srcLength + sizeof(int) > dstLength)
	{
		printf("Bitstream does not fit inside bitstream!\n");
	}

	*((int *) dstCursor) = srcLength;
	dstCursor += sizeof(int);

	memcpy(dstCursor, src, sizeof(unsigned char) * srcLength);

}


//Parse the yuv file
void OpenSerialYUVIntoInputPicture(
	BufferDescriptor_t* inputPicture, 
	unsigned char* receivedBuffer, 
	int width, 
	int height)
{
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

void OpenReconBestIntoSerialYUV(
	BufferDescriptor_t* reconBest, 
	unsigned char* sendBuffer, 
	int width, 
	int height)
{
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
