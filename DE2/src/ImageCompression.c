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

typedef enum ProcessType {
	INVALID,
	ENCODE_PICTURE,
	DECODE_PICTURE,
	ENCODE_DECODE_PICTURE,
};

int main(int argc, char* argv[])
{

	// Input Arguments
	const char *bitstreamFile;
	const char *yuvFile;
	int pictureWidth;
	int pictureHeight;
	int qp;

	int ProcessingType = INVALID;

	// Determine whether encoding or decoding by using number of arguments
#if VS_BUILD
	// Encoder Arguments:
	// EncoderDecoder.exe inputYUVFile outputBitstreamFile width height QP
#define NUM_ENC_ARGS	6
	// Decoder Arguments:
	// EncoderDecoder.exe inputBitstreamFile outputYUVFile
#define NUM_DEC_ARGS	3

	if(argc == NUM_ENC_ARGS) 
	{
		ProcessingType = ENCODE_PICTURE;
		yuvFile = argv[1];
		bitstreamFile = argv[2];
		pictureWidth = atoi(argv[3]);
		pictureHeight = atoi(argv[4]);
		qp = atoi(argv[5]);
	}
	else if(argc == NUM_DEC_ARGS)
	{
		ProcessingType = DECODE_PICTURE;
		bitstreamFile = argv[1];
		yuvFile = argv[2];
	}
	else 
	{
		printf("Unknown number of arguments...\n");
		printf("Encoder.exe inputYUVFile outputBitstreamFile width height QP\n");
		printf("Decoder.exe inputBitstreamFile outputYUVFile\n");
		return 0;
	}

// DE2 Build
#elif N2_BUILD
	ProcessingType = ENCODE_PICTURE;
	pictureWidth = DEFAULT_PICTURE_WIDTH;
	pictureHeight = DEFAULT_PICTURE_HEIGHT;
	qp = 15;
#endif


	/**** ENCODE ****/
	if(ProcessingType == ENCODE_PICTURE)
	{
		CodingUnitStructure_t codingUnitStructure;
		BufferDescriptor_t inputPicture;
		Bitstream_t outputBitstream;

		// Error check inputs
		if(qp < 0 || qp > 51) {
			printf("Invalid QP (%d), please choose a valid QP between 0-51", qp);
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
			qp);

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
		// Only 320x240 is supported for NiosII
		unsigned char receivedBuffer[BUFFERSIZE];
		unsigned char sendBuffer[BUFFERSIZE];

		*WRITE = 0x0;
		printf("Receiving \n");
		Receive(receivedBuffer);
		printf("Recieve done \n");

		OpenSerialYUVIntoInputPicture(
			&inputPicture,
			receivedBuffer, 
			DEFAULT_PICTURE_WIDTH, 
			DEFAULT_PICTURE_HEIGHT);
	#elif VS_BUILD
		printf("Opening image from file: \'%s\'...\t", yuvFile);
		OpenYUVFileIntoInputPicture(
			&inputPicture, 
			yuvFile, 
			pictureWidth, 
			pictureHeight);
		printf("Done!\n");
	#endif

		printf("Input Picture: %dx%d, qp: %d\n", pictureWidth, pictureHeight, qp);

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
#if N2_BUILD
		OpenDataIntoSerialData(
				sendBuffer,
				BUFFERSIZE,
				outputBitstream.data,
				outputBitstream.size
				);
		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
#elif VS_BUILD
		WriteBitstreamToFile(
			&outputBitstream,
			bitstreamFile);
#endif


// ENABLE ENCODER RECON OUT
// Enables output of best reconstructed CUs
#define ENABLE_ENCODER_RECON_OUT	(0)
		// Recon Code (deprecated)
#if ENABLE_ENCODER_RECON_OUT
		// Output the picture
	#if N2_BUILD
		OpenReconBestIntoSerialYUV(
				&inputPicture,//&codingUnitStructure.reconBestBuffer,
				sendBuffer,
				DEFAULT_PICTURE_WIDTH,
				DEFAULT_PICTURE_HEIGHT);

		printf("Sending\n");
		Send(sendBuffer);
		printf("Send Done\n");
	#elif VS_BUILD
		SaveYUVToFile(
			OUTPUT_RECON_YUV, 
			&(codingUnitStructure.reconBestBuffer));
	#endif
#endif 

		printf("Done!\n\n\n");


		/*** DECONSTRUCTION ***/
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BufferDescriptorDeconstructor(&inputPicture);
		BitstreamDeconstructor(&outputBitstream);

	}

	/**** DECODE ****/
	else if(ProcessingType == DECODE_PICTURE)
	{
		CodingUnitStructure_t codingUnitStructure;
		Bitstream_t inputBitstream; 

		printf("HAAF IMAGE DECODER V1.0\n");

		/*** CONSTRUCTION ***/
		OpenBitstreamFromFile(
			bitstreamFile,
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
		//OpenReconBestIntoSerialYUV(&codingUnitStructure.reconBestBuffer, sendBuffer, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
		//printf("Sending\n");
		//Send(sendBuffer);
		//printf("Send Done\n");
#elif VS_BUILD
		printf("Writing to file: \'%s\'...\t", yuvFile);
		SaveYUVToFile(
			yuvFile, 
			&(codingUnitStructure.reconBestBuffer));
		printf("Done!\n\n\n");
#endif


		/*** DECONSTRUCTION ***/
		
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BitstreamDeconstructor(&inputBitstream);
		
	}

	return 0;
}

