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


int main(int argc, char* argv[])
{
	CodingUnitStructure_t codingUnitStructure;
	Bitstream_t outputBitstream;

	/*** CONSTRUCTION ***/

	// Coding Unit Structure Initializes the Coding Unit's and Input Pictures
	CodingUnitStructureConstructor(
		&codingUnitStructure, 
		DEFAULT_PICTURE_WIDTH, 
		DEFAULT_PICTURE_HEIGHT);

	BitstreamConstructor(
		&outputBitstream, 
		DEFAULT_PICTURE_WIDTH, 
		DEFAULT_PICTURE_HEIGHT);

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
		&codingUnitStructure.inputPicture, 
		"Z:\\EncodedFiles\\Cats_320x240_420.yuv", 
		DEFAULT_PICTURE_WIDTH, 
		DEFAULT_PICTURE_HEIGHT);
#endif


	printf("Encoding\n");
	// Encode the image
	EncodeLoop(&codingUnitStructure);

	GenerateBitstream(
		&codingUnitStructure, 
		&outputBitstream);

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

	return 0;
}

