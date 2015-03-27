#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "../include/Utility.h"
#include "../include/Prediction.h"
#include "../include/lz4io.h"

// BITSTREAM
#define EXPECTED_COMPRESSION_RATIO		1
#define BITSTREAM_HEADER_LEN			10

// PREDICTION
#define NUMBITSPERMODE					(2)
#define MAXPREDMODES					(1 << NUMBITSPERMODE)



typedef struct Bitstream_s
{
	unsigned char *data;
	int size;
	int maxSize;
} Bitstream_t;

void BitstreamConstructor(
	Bitstream_t *bitstream, 
	int maxWidth, 
	int maxHeight);
void BitstreamDeconstructor(Bitstream_t *bitstream);

void EncodeBitstream(
	// OUTPUT
	Bitstream_t *outputBitstream,
	// IN
	unsigned char *transformCoeffs,
	int transformCoeffsSize, // Total size of transformCoeffs (remember, they are ints, not chars)
	PredictionMode_t *predictionModes,
	int predictionModesLen,
	int width,
	int height);

void GetPictureResolutionFromHeader(
	Bitstream_t *bitstream,
	int *width,
	int *height);

void WriteBitstreamToFile(
	Bitstream_t *bitstream,
	const char *filename);

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
	unsigned int transformCoeffLen);

#endif