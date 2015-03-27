// ImageCompression.cpp 
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "../include/Utility.h"
#include "../include/CodingUnitStructure.h"
#include "../include/Transform.h"
#include "../include/Encode.h"
#include "../include/OpenPicture.h"
#include "../include/Bitstream.h"


int main(int argc, char* argv[])
{
	/**** ENCODING ****/
	{
		CodingUnitStructure_t codingUnitStructure;
		BufferDescriptor_t inputPicture;
		Bitstream_t outputBitstream;

		int pictureWidth = DEFAULT_PICTURE_WIDTH;
		int pictureHeight = DEFAULT_PICTURE_HEIGHT;
		int requestedQP = DEFAULT_QP_VALUE;

		/*** CONSTRUCTION ***/

		// Coding Unit Structure Initializes the Coding Unit's and Input Pictures
		CodingUnitStructureConstructor(
			&codingUnitStructure, 
			pictureWidth, 
			pictureHeight);

		BufferDescriptorConstructor(
			&inputPicture,
			pictureWidth,
			pictureHeight,
			sizeof(unsigned char));

		BitstreamConstructor(
			&outputBitstream, 
			pictureWidth, 
			pictureHeight);

		/*** MAIN ALGORITHM ***/

		// Input the picture
	#if N2_BUILD
		unsigned char receivedBuffer[BUFFERSIZE];
		unsigned char sendBuffer[BUFFERSIZE];

		*WRITE = 0x0;
		printf("Receiving \n");
		Receive(receivedBuffer);
		printf("Recieve done \n");

		OpenSerialYUVIntoInputPicture(
			&codingUnitStructure.inputPicture, 
			receivedBuffer, 
			DEFAULT_PICTURE_WIDTH, 
			DEFAULT_PICTURE_HEIGHT);
	#elif VS_BUILD
		OpenYUVFileIntoInputPicture(
			&inputPicture, 
			"Z:\\EncodedFiles\\Cats_320x240_420.yuv", 
			pictureWidth, 
			pictureHeight);
	#endif

		SetInputPicture(
			&codingUnitStructure, 
			&inputPicture);

		printf("Encoding\n");

		// Encode the image
		EncodeLoop(
			&codingUnitStructure, 
			requestedQP);

		// Generate the bitstream
		GenerateBitstream(
			&codingUnitStructure, 
			&outputBitstream);

		// Write the bitstream to file
		WriteBitstreamToFile(
			&outputBitstream,
			"Z:\\Cats.haaf");


		printf("Encode Done\n");

		// Output the picture
	#if N2_BUILD
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
	#elif VS_BUILD
		SaveYUVToFile(
			"Z:\\EncodedFiles\\ReconCats_320x240.yuv", 
			&(codingUnitStructure.reconBestBuffer));
	#endif

		/*** DECONSTRUCTION ***/
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BufferDescriptorDeconstructor(&inputPicture);
		BitstreamDeconstructor(&outputBitstream);

	}


	/**** DECODE FILE INTO RECON ****/
	//CodingUnitStructureDeconstructor(&codingUnitStructure);
	{
		//int width, height;
		//GetPictureResolutionFromHeader(
		//	&outputBitstream,
		//	&width,
		//	&height);

		// Deconstruct current structure, we want
		//CodingUnitStructureConstructor(
		//	&codingUnitStructure, 
		//	width, 
		//	height);
	}








	

	return 0;
}

