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


// ENCODER
//#define PICTURE_WIDTH			(320)//(1280)
//#define PICTURE_HEIGHT		(240)//(720)
#define PICTURE_WIDTH			(1280)
#define PICTURE_HEIGHT			(720)
#define PICTURE_QP				(DEFAULT_QP_VALUE)
//#define INPUT_YUV_FILE			("Z:\\EncodedFiles\\Cats_320x240_420.yuv")//("Z:\\EncodedFiles\\catlarge.yuv")
//#define OUTPUT_RECON_YUV		("Z:\\EncodedFiles\\recon.yuv")//("Z:\\EncodedFiles\\recon.yuv")
//#define OUTPUT_BITSTREAM_FILE	("Z:\\EncodedFiles\\cats_320x240.haaf")//("Z:\\EncodedFiles\\catlarge.haaf")
#define INPUT_YUV_FILE			("Z:\\EncodedFiles\\catlarge.yuv")
#define OUTPUT_RECON_YUV		("Z:\\EncodedFiles\\recon.yuv")
#define OUTPUT_BITSTREAM_FILE	("Z:\\EncodedFiles\\catlarge.haaf")

// DECODER
//#define INPUT_BITSTREAM			("Z:\\EncodedFiles\\cats_320x240.haaf")
//#define OUTPUT_YUV				("Z:\\EncodedFiles\\Decoded_cats_320x240.yuv")
#define INPUT_BITSTREAM			("Z:\\EncodedFiles\\catlarge.haaf")
#define OUTPUT_YUV				("Z:\\EncodedFiles\\Decoded_catlarge_1280x720_NewCost_approximate.yuv")


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

		// Error check inputs
		if(requestedQP < 0 || requestedQP > 51) {
			printf("Invalid QP (%d), please choose a valid QP between 0-51", requestedQP);
			return 0;
		}
		if(((pictureWidth % 16) != 0) || ((pictureHeight % 16) != 0)) {
			printf("Width/Height not divisible by 16! Exiting...");
			return 0;
		}
		
		printf("HAAF IMAGE ENCODER V1.0\n");

		/*** CONSTRUCTION ***/

		// Coding Unit Structure Initializes the Coding Unit's and Input Pictures
		CodingUnitStructureConstructor(
			&codingUnitStructure, 
			pictureWidth, 
			pictureHeight,
			requestedQP);

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
		printf("Opening image from file: \'%s\'...\t", INPUT_YUV_FILE);
		OpenYUVFileIntoInputPicture(
			&inputPicture, 
			INPUT_YUV_FILE, 
			pictureWidth, 
			pictureHeight);
		printf("Done!\n");
	#endif

		printf("Input Picture: %dx%d, qp: %d\n", pictureWidth, pictureHeight, requestedQP);

		SetInputPicture(
			&codingUnitStructure, 
			&inputPicture);

		printf("Encoding Bitstream...\t");

		// Encode the image
		EncodeLoop(
			&codingUnitStructure);

		// Generate the bitstream
		GenerateBitstream(
			&codingUnitStructure, 
			&outputBitstream);

		// Write the bitstream to file
		WriteBitstreamToFile(
			&outputBitstream,
			OUTPUT_BITSTREAM_FILE);

		printf("Done!\n\n\n");

#if ENABLE_ENCODER_RECON_OUT
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

		printf("HAAF IMAGE DECODER V1.0\n");

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
			pictureHeight,
			qp);

		/*** MAIN ALGORITHM ***/
		
		printf("Output Picture: %dx%d, qp: %d\n", pictureWidth, pictureHeight, qp);

		// Decode bitstream into CodingUnitStructure
		printf("Decoding Bitstream....\t");
		DecodeBitstream(
			&codingUnitStructure, 
			&inputBitstream);

		DecodeLoop(&codingUnitStructure);
		printf("Done!\n");

	// Output the picture
#if N2_BUILD
		OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
#elif VS_BUILD
		printf("Writing to file: \'%s\'...\t", OUTPUT_YUV);
		SaveYUVToFile(
			OUTPUT_YUV, 
			&(codingUnitStructure.reconBestBuffer));
		printf("Done!\n\n\n");
#endif


		/*** DECONSTRUCTION ***/
		
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BitstreamDeconstructor(&inputBitstream);
		
	}

#endif


	return 0;
}

