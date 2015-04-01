#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "../include/Utility.h"
#include "../include/Prediction.h"
#include "../include/CodingUnitStructure.h"
#include "../include/lz4io.h"

// BITSTREAM
#define EXPECTED_COMPRESSION_RATIO		1


// Defined in implementation of write header
#define HEADER_WIDTH_OFFSET				0
#define HEADER_HEIGHT_OFFSET			2
#define HEADER_BWIDTH_OFFSET			4
#define HEADER_BHEIGHT_OFFSET			5
#define HEADER_QP_OFFSET				6
#define HEADER_LZ4_OFFSET				7
#define BITSTREAM_HEADER_LEN			11 

// PREDICTION
#define NUMBITSPERMODE					(2)
#define MAXPREDMODES					(1 << NUMBITSPERMODE)



/*** STRUCTURE ***/
typedef struct Bitstream_s
{
	unsigned char *data;
	int size;
	int maxSize;
} Bitstream_t;

/*** CONSTRUCTOR / DECONSTRUCTOR ***/

void BitstreamConstructor(
	Bitstream_t *bitstream, 
	int maxWidth, 
	int maxHeight);
void BitstreamConstructorMaxSize(
	Bitstream_t *bitstream, 
	int maxSize);
void BitstreamDeconstructor(Bitstream_t *bitstream);


/*** IO ***/
void WriteBitstreamToFile(
	Bitstream_t *bitstream,
	const char *filename);

void OpenBitstreamFromFile(
	const char *filename,
	Bitstream_t *inputBitstream,
	int *pictureWidth,
	int *pictureHeight,
	int *qp);

/*** DECODING ***/
void DecodeBitstream(
	CodingUnitStructure_t *codingUnitStructure,
	Bitstream_t *inputBitstream);


/*** ENCODING ***/
void EncodePredictionModes(
	// OUT
	unsigned char *outputBuffer,
	int *outputBufferLength,
	// IN
	PredictionMode_t *predictionModes,
	int predictionModesLen,
	int numPredictionModes);

void DecodePredictionModes(
	// OUT
	PredictionMode_t *predictionModes,
	int *predictionModesLen,
	// IN
	unsigned char *inputBuffer,
	int inputBufferLen,
	int numPredictionModes);

void WriteHeaderInfo(
	unsigned char *outputBuffer, // Assume this has enough memory
	int *bytesWritten, // Keep track of all the bytes written by the fn
	unsigned short width,
	unsigned short height,
	unsigned char blockWidth,
	unsigned char blockHeight,
	unsigned char qp,
	unsigned int transformCoeffLen);

void EncodeBitstream(
	// OUTPUT
	Bitstream_t *outputBitstream,
	// IN
	unsigned char *transformCoeffs,
	int transformCoeffsSize, // Total size of transformCoeffs (remember, they are ints, not chars)
	PredictionMode_t *predictionModes,
	int predictionModesLen,
	int width,
	int height,
	int qp);


#endif