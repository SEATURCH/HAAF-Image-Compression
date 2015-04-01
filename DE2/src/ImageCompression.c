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

// ENUM - DO NOT TOUCH
#define ENCODE_PICTURE			(0)
#define DECODE_PICTURE			(1)
#define ENCODE_DECODE_PICTURE	(2)


/**** Change this to determine process type ****/
#define PROCESS_TYPE	(ENCODE_DECODE_PICTURE)

// Enable/Disable Encoding/Decoding
#if PROCESS_TYPE == ENCODE_DECODE_PICTURE

#define ENABLE_ENCODING	(1)
#define ENABLE_DECODING	(1)

#elif PROCESS_TYPE == ENCODE_PICTURE

#define ENABLE_ENCODING	(1)
#define ENABLE_DECODING	(0)

#else if PROCESS_TYPE == DECODE_PICTURE

#define ENABLE_ENCODING	(0)
#define ENABLE_DECODING	(1)

#endif

int main(int argc, char* argv[])
{

#if ENABLE_ENCODING 
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
			"Z:\\EncodedFiles\\Cats.haaf");

//		{
//#define VALID_VALUE_RANGE	5000000
//			int i;
//
//			for(i = 0; i < /*(codingUnitStructure.transformBestBuffer.yuvSize / 4)*/1000; i++)
//			{
//				int sampleValue = ((int *)codingUnitStructure.transformBestBuffer.fullPicturePointer)[i];
//				printf("%d: %d\n", i, sampleValue);
//				if( sampleValue > VALID_VALUE_RANGE ||
//					sampleValue < -VALID_VALUE_RANGE)
//				{
//					//printf("FUCK\n");
//				}
//			}
//		}


		printf("Encode Done\n");

		// Output the picture
	#if N2_BUILD
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
	#elif VS_BUILD
		SaveYUVToFile(
			"Z:\\EncodedFiles\\EncoderReconCats_320x240.yuv", 
			&(codingUnitStructure.reconBestBuffer));
	#endif

		/*** DECONSTRUCTION ***/
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BufferDescriptorDeconstructor(&inputPicture);
		BitstreamDeconstructor(&outputBitstream);

	}

#endif


#if ENABLE_DECODING
	/**** DECODE FILE INTO RECON ****/
	{
		CodingUnitStructure_t codingUnitStructure;
		BufferDescriptor_t inputPicture;
		Bitstream_t inputBitstream; //  Will be constructed by OpenBitstreamFromFile

		int pictureWidth;
		int pictureHeight;
		int qp;

		const char *inputFile = "Z:\\EncodedFiles\\Cats.haaf";

		/*** CONSTRUCTION ***/
		OpenBitstreamFromFile(
			"Z:\\EncodedFiles\\Cats.haaf",
			&inputBitstream,
			&pictureWidth,
			&pictureHeight,
			&qp);

		CodingUnitStructureConstructor(
			&codingUnitStructure,
			pictureWidth,
			pictureHeight);

		/*** MAIN ALGORITHM ***/

		// Decode bitstream into CodingUnitStructure
		DecodeBitstream(
			&codingUnitStructure, 
			&inputBitstream);

		codingUnitStructure.qp = qp;

		DecodeLoop(&codingUnitStructure);

	// Output the picture
#if N2_BUILD
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
#elif VS_BUILD
		SaveYUVToFile(
			"Z:\\EncodedFiles\\DecodedReconCats_320x240.yuv", 
			&(codingUnitStructure.reconBestBuffer));
#endif


		/*** DECONSTRUCTION ***/
		
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BitstreamDeconstructor(&inputBitstream);
		
	}

#endif


	return 0;
}

