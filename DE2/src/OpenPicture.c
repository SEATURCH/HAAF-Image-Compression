#include "../include/OpenPicture.h"

#include "string.h"
#include "ctype.h"
#include "../include/libbmp/bmpfile.h"

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
	if(srcLength + (int)sizeof(int) > dstLength)
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


FileType_t GetFileType(const char *fileName)
{
	int stringLength = strlen(fileName);
	int cursor = 0;

	// Booleans
	int dotFound = 0;

	while(cursor < stringLength)
	{
		// If the previous character was a '.',
		// check if the next three characters are a recognized extension
		if(dotFound && cursor == (stringLength - 3)) 
		{
			if(tolower(fileName[cursor]) == 'b'
				&& tolower(fileName[cursor+1]) == 'm'
				&& tolower(fileName[cursor+2]) == 'p')
			{
				return FileTypeBmp;
			}
			else if(tolower(fileName[cursor]) == 'y'
				&& tolower(fileName[cursor+1]) == 'u'
				&& tolower(fileName[cursor+2]) == 'v')
			{
				return FileTypeYuv;
			}
			dotFound = 0;
		}

		if(fileName[cursor] == '.')
		{
			dotFound = 1;
		}

		cursor++;
	}

	return NoFileType;
}



#define RGB_BYTES_PER_SAMPLE	3

#define clip(var) ((var>=255)?255:(var<=0)?0:var)
void ConvertYUVIntoBMP(
	unsigned char *yuvPtr,
	bmpfile_t *bmp,
	int width,
	int height)
{
	// a = Y_Picture1
	// b = U_Picture1
	// c = V_Picture1
	int a, b, c;

	// Variables used for looping
	int i, j;

	unsigned char *yPtr = yuvPtr;
	unsigned char *uPtr = yPtr + (width * height);
	unsigned char *vPtr = uPtr + (width >> 1)*(height >> 1);

	int uv_spacing = (width+1)>>1;

	rgb_pixel_t pixel;
	pixel.alpha = 0;

	// Iterate through every pixel, starting from top left, going right horizontally and down vertically
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			// Get the Y U V pixels and add the offset
			a = yPtr[ j*width + i ] - 16;
			b = uPtr[ (j>>1)*uv_spacing + (i>>1) ] - 128;
			c = vPtr[ (j>>1)*uv_spacing + (i>>1) ] - 128;

			//http://en.wikipedia.org/wiki/YUV
			//[R]   [ 1.00  0.00  1.28 ] [Y]
			//[G] = [ 1.00 -0.21 -0.38 ] [U]
			//[B] 	[ 1.00  2.13  0.00 ] [V]

			// Apply integer-ized YUV to RGB conversion
			pixel.red = clip(( 298*a + 409*c + 128) >> 8);
			pixel.green = clip((298*a - 100*b - 208*c + 128) >> 8);
			pixel.blue = clip((298*a + 516*b + 128) >> 8);

			bmp_set_pixel(bmp, i, j, pixel);
		}
	}
}


void SaveYUVtoBMP(
	unsigned char *yuvPtr,
	int width,
	int height,
	const char *outputFileName)
{
	bmpfile_t *bmp;

	// 8 bits per pixel
	const int bitDepth = 24;

	if ((bmp = bmp_create(width, height, bitDepth)) == NULL) {
		printf("Invalid depth value: %s.\n", bitDepth);
		return;
    }

	ConvertYUVIntoBMP(yuvPtr, bmp, width, height);

	if(!bmp_save(bmp, outputFileName))
	{
		printf("Could not save bitmap to file!\n");
	}

	bmp_destroy(bmp);
}
