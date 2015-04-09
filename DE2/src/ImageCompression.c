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
#if PI_BUILD
	PI_DE2_ENCODE_PICTURE,
#endif
};

int main(int argc, char* argv[])
{

	// Input Arguments
	const char *bitstreamFile;
	const char *rawFile;
	int pictureWidth = -1;
	int pictureHeight = -1;
	int qp = -1;

	FileType_t rawFileType;

	int ProcessingType = INVALID;

	// Determine whether encoding or decoding by using number of arguments
#if VS_BUILD
	// Encoder Arguments:
	// EncoderDecoder.exe inputrawFile outputBitstreamFile width height QP
#define NUM_ENC_ARGS	6
	// Decoder Arguments:
	// EncoderDecoder.exe inputBitstreamFile outputrawFile
#define NUM_DEC_ARGS	3
	// Pi-DE2 Arguments:
	// EncoderDecoder.exe outputHAAFFile
#define NUM_PI_ARGS		2

	if(argc == NUM_ENC_ARGS) 
	{
		ProcessingType = ENCODE_PICTURE;
		rawFile = argv[1];
		bitstreamFile = argv[2];
		pictureWidth = atoi(argv[3]);
		pictureHeight = atoi(argv[4]);
		qp = atoi(argv[5]);

		// Not used
		rawFileType = FileTypeYuv;
	}
	else if(argc == NUM_DEC_ARGS)
	{
		ProcessingType = DECODE_PICTURE;
		bitstreamFile = argv[1];
		rawFile = argv[2];

		// Detect the extension in the rawFileName for output
		rawFileType = GetFileType(rawFile);
	}
#if PI_BUILD
	else if(argc == NUM_PI_ARGS)
	{
		ProcessingType = PI_DE2_ENCODE_PICTURE;
		bitstreamFile = argv[1];
	}
#endif
	else 
	{
		printf("Unknown number of arguments...\n");
		printf("Encoder.exe inputrawFile outputBitstreamFile width height QP\n");
		printf("Decoder.exe inputBitstreamFile outputrawFile\n");
		return 0;
	}


// DE2 Build
#elif N2_BUILD
	ProcessingType = ENCODE_PICTURE;
	pictureWidth = DEFAULT_PICTURE_WIDTH;
	pictureHeight = DEFAULT_PICTURE_HEIGHT;
	qp = 15;
#endif

#if PI_BUILD
	if(ProcessingType == PI_DE2_ENCODE_PICTURE)
	{
		open_picture(bitstreamFile);
	}
	/**** ENCODE ****/
	else 
#endif
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
		printf("Opening image from file: \'%s\'...\t", rawFile);
		OpenYUVFileIntoInputPicture(
			&inputPicture, 
			rawFile, 
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
		{
			printf("Writing to file: \'%s\'...\t", rawFile);
			
			// If the file type is BMP, perform conversion
			if(rawFileType == FileTypeBmp)
			{
				// Write file to Bitmap
				SaveYUVtoBMP(
					codingUnitStructure.reconBestBuffer.fullPicturePointer,
					pictureWidth, 
					pictureHeight, 
					rawFile);
			}
			// Always save to .yuv in the non-bmp case
			else
			{
				SaveYUVToFile(
					rawFile, 
					&(codingUnitStructure.reconBestBuffer));
			}
			printf("Done!\n\n\n");
		}
#endif


		/*** DECONSTRUCTION ***/
		
		CodingUnitStructureDeconstructor(&codingUnitStructure);
		BitstreamDeconstructor(&inputBitstream);
		
	}

	return 0;
}

