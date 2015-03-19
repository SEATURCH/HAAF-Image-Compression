// CodingUnitStructure.h
// This header includes all the required functions/structures for the Coding Unit Structure
// Created on 03/03/2015 by Francis Lo and Alan Larson


#include "stdio.h"
#include "stdlib.h"
#include "Utility.h"


#ifndef CODINGUNITSTRUCTURE_H
#define CODINGUNITSTRUCTURE_H


/*** STRUCTURES ***/

typedef unsigned char CuBuffer[CODING_UNIT_SCRATCH_SIZE];
typedef int CuIntBuffer[CODING_UNIT_SCRATCH_SIZE];

typedef enum PredictionMode_s {
	Invalid			= -1,
	DC				= 0,
	Vertical		= 1,
	Horizontal		= 2,
	Planar			= 3,
	PredictionModeCount = 4,
} PredictionMode_t;

void PredictionModeDC(
	unsigned char *dst, 
	int dstStride, 
	unsigned char *referenceBuffer, 
	int size);
void PredictionModeVertical(
	unsigned char *dst, 
	int dstStride, 
	unsigned char *referenceBuffer, 
	int size);
void PredictionModeHorizontal(
	unsigned char *dst, 
	int dstStride, 
	unsigned char *referenceBuffer, 
	int size);
void PredictionModePlanar(
	unsigned char *dst, 
	int dstStride, 
	unsigned char *referenceBuffer, 
	int size);

static void (*PredictionFuncPtrTable[PredictionModeCount]) (
	unsigned char *dst, 
	int dstStride, 
	unsigned char *referenceBuffer, 
	int size) = 
	{
		PredictionModeDC, 
		PredictionModeVertical, 
		PredictionModeHorizontal,
		PredictionModePlanar,
	};

typedef enum QuantizationDirection_s
{
	QuantizeForward,
	QuantizeBackward,
	QuantizeModeCount
} QuantizationDirection_t;



void Quantize(int *coeff, int coeffStride, int width, int height, int qp);
void InverseQuantize(int *coeff, int coeffStride, int width, int height, int qp);

static void (*QuantizationFuncPtrTable[QuantizeModeCount]) (int *coeff, int coeffStride, int width, int height, int qp) = 
{
	Quantize,
	InverseQuantize
};


void CalculateResidual(
	unsigned char *residual, 
	int residualStride, 
	unsigned char *actual, 
	int actualStride, 
	unsigned char *prediction, 
	int predictionStride,
	int width,
	int height);

void CalculateResidualInt(
	// OUT
	int *residual, 
	int residualStride, 
	// IN
	unsigned char *actual, 
	int actualStride, 
	unsigned char *prediction, 
	int predictionStride, 
	// CONST
	int width,
	int height);


void CalculateActual(
	unsigned char *actual, 
	int actualStride, 
	unsigned char *residual, 
	int residualStride, 
	unsigned char *prediction, 
	int predictionStride, 
	int width,
	int height);

void CalculateReconInt(
	// OUT
	int *recon, 
	int reconStride, 
	// IN
	int *residual, 
	int residualStride, 
	unsigned char *prediction, 
	int predictionStride, 
	// CONST
	int width,
	int height);

int CheckIdenticalBuffers(
	unsigned char *left,
	int leftStride,
	unsigned char *right,
	int rightStride,
	int width,
	int height);

void CopyBlockByte(
	unsigned char *src,
	int srcStride,
	unsigned char *dest,
	int destStride,
	int width,
	int height);





// This is a general purpose structure that contains an entire YUV picture
typedef struct BufferDescriptor_s
{
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
	PredictionMode_t bestPredictionMode;
	int predictionModeCost[PredictionModeCount];

} CodingUnit_t;

typedef struct CodingUnitStructure_s
{
	// Coding Unit Variables
	CodingUnit_t *codingUnits;
	int numCusWidth;
	int numCusHeight;

	// Input Picture Variables
	BufferDescriptor_t inputPicture;
	int widthPicture;
	int heightPicture;

	// Transform Picture Buffers
	BufferDescriptor_t transformBestBuffer;

	// Recon Picture Buffers
	BufferDescriptor_t reconBestBuffer;

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

void CopyCharToIntBuffer(CuBuffer src, CuIntBuffer dst, int size);
void CopyIntToCharBuffer(CuIntBuffer src, CuBuffer dst,  int size);

/*** HELPER FUNCTIONS ***/
void palloc(
	unsigned char **picturePointer, 
	int width, 
	int height);

#endif
