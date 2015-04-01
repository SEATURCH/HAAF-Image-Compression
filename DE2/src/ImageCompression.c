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

#define PICTURE_WIDTH	(1280)
#define PICTURE_HEIGHT	(720)
#define PICTURE_QP		(DEFAULT_QP_VALUE)

#define INPUT_YUV_FILE			("Z:\\EncodedFiles\\catlarge.yuv")
#define OUTPUT_RECON_YUV		("Z:\\EncodedFiles\\recon.yuv")
#define OUTPUT_BITSTREAM_FILE	("Z:\\EncodedFiles\\catlarge.haaf")


#define INPUT_BITSTREAM			("Z:\\EncodedFiles\\catlarge.haaf")
#define OUTPUT_YUV				("Z:\\EncodedFiles\\Decoded_catlarge.yuv")


int main(int argc, char* argv[])
{

#if ENABLE_ENCODING 
	/**** ENCODING ****/
	{
		CodingUnitStructure_t codingUnitStructure;
		BufferDescriptor_t inputPicture;
		Bitstream_t outputBitstream;

		int pictureWidth = PICTURE_WIDTH;
		int pictureHeight = PICTURE_HEIGHT;
		int requestedQP = PICTURE_QP;

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
			INPUT_YUV_FILE, 
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
			OUTPUT_BITSTREAM_FILE);

		printf("Encode Done\n");

		// Output the picture
	#if N2_BUILD
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
	#elif VS_BUILD
		SaveYUVToFile(
			OUTPUT_RECON_YUV, 
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

		const char *inputFile = INPUT_BITSTREAM;

		/*** CONSTRUCTION ***/
		OpenBitstreamFromFile(
			inputFile,
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
			OUTPUT_YUV, 
			&(codingUnitStructure.reconBestBuffer));
#endif


		/*** DECONSTRUCTION ***/
		
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BitstreamDeconstructor(&inputBitstream);
		
	}

#endif


	return 0;
}

