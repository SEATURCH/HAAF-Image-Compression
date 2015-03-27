// CodingUnitStructure.h
// This header includes all the required functions/structures for the Coding Unit Structure
// Created on 03/03/2015 by Francis Lo and Alan Larson

#include "../include/Utility.h"
#include "../include/Prediction.h"
#include "../include/Bitstream.h"

#ifndef CODINGUNITSTRUCTURE_H
#define CODINGUNITSTRUCTURE_H

/*** STRUCTURES ***/

// This is a general purpose structure that contains an entire YUV picture
typedef struct BufferDescriptor_s
{
	// Contains the full picture
	unsigned char *fullPicturePointer;
	int yuvSize;

	// Luma
	unsigned char *yBuffer;
	int yStride;
	int yHeight;

	// Cr
	unsigned char *uBuffer;
	int uStride;
	int uHeight;

	// Cb
	unsigned char *vBuffer;
	int vStride;
	int vHeight;

	int sampleSize;
} BufferDescriptor_t;

typedef struct CodingUnit_s
{
	int blockIndex;
	
	// Offsets in the picture of where the coding unit starts
	int yBufferOffset;
	int uvBufferOffset;

	// Prediction Variables
	int predictionModeCost[PredictionModeCount];

} CodingUnit_t;

typedef struct CodingUnitStructure_s
{
	// Coding Unit Variables
	CodingUnit_t *codingUnits;
	int numCusWidth;
	int numCusHeight;
	
	// Input Picture Variables
	BufferDescriptor_t *inputPicture;
	int widthPicture;
	int heightPicture;

	// Transform Picture Buffers
	BufferDescriptor_t transformBestBuffer;

	// Prediction Variables
	PredictionMode_t *bestPredictionModes;

	// Recon Picture Buffers
	BufferDescriptor_t reconBestBuffer;

	// Encoded Bitstream Buffer
	Bitstream_t bitstream;

} CodingUnitStructure_t;

/*** FUNCTION DEFINITIONS ***/
void BufferDescriptorConstructor(BufferDescriptor_t *pictureBuffer,
	int width,
	int height,
	int sampleSize);

void BufferDescriptorDeconstructor(BufferDescriptor_t *pictureBuffer);

void CodingUnitStructureConstructor(
	CodingUnitStructure_t *codingUnitStructure, 
	int pictureWidth, 
	int pictureHeight);
void CodingUnitStructureDeconstructor(CodingUnitStructure_t *codingUnitStructure);

void SetInputPicture(
	CodingUnitStructure_t *codingUnitStructure,
	BufferDescriptor_t *inputPicture);



#endif
