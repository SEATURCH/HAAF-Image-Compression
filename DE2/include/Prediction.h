// This header file specifies the prediction modes inside the encoder

#ifndef PREDICTION_H
#define PREDICTION_H

#include "../include/Utility.h"
#if N2_BUILD
#include "IO.h"
#endif

typedef enum PredictionMode_s {
	Invalid			= -1,
	DC				= 0,
	Vertical		= 1,
	Horizontal		= 2,
	Planar			= 3,
	PredictionModeCount = 4,
} PredictionMode_t;


#if N2_BUILD
#define BASE (volatile int *) 0x4000
#define DC_BASE_ADDRESS 768
#define VERTICAL_BASE_ADDRESS 512
#define HORIZONTAL_BASE_ADDRESS 256
#define PLANAR_BASE_ADDRESS 1024
#define TOPLEFT_ELEMENT_BASE_ADDRESS 1280
#define TOP_ELEMENT_BASE_ADDRESS 1281
#define LEFT_ELEMENT_BASE_ADDRESS 1297
#define GO_BIT_BASE_ADDRESS 1313
#define DONE_BIT_BASE_ADDRESS 1314

void PredictionVHDL(
	// IN
	unsigned char *referenceBuffer);

void PredictionModeDCRead (
	// OUT
	unsigned char *dst, 
	int dstStride);

void PredictionModeVerticalRead (
	// OUT
	unsigned char *dst, 
	int dstStride);

void PredictionModeHorizontalRead (
	// OUT
	unsigned char *dst, 
	int dstStride);

void PredictionModePlanarRead (
	// OUT
	unsigned char *dst, 
	int dstStride);
#endif



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
#if VS_BUILD
	unsigned char *dst, 
	int dstStride,
	unsigned char *referenceBuffer, 
	int size
#elif N2_BUILD
	unsigned char *dst, 
	int dstStride
#endif
) = 
	{
#if N2_BUILD
		PredictionModeDCRead,
		PredictionModeVerticalRead,
		PredictionModeHorizontalRead,
		PredictionModePlanarRead,
#elif VS_BUILD
		PredictionModeDC, 
		PredictionModeVertical, 
		PredictionModeHorizontal,
		PredictionModePlanar,
#endif
	};

/*** RESIDUAL/RECON ***/
void CalculateResidual(
	// OUT
	unsigned char *residual, 
	int residualStride, 
	// IN
	unsigned char *actual, 
	int actualStride, 
	unsigned char *prediction, 
	int predictionStride,
	// CONST
	int width,
	int height);

void CalculateResidualDWord(
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

void CalculateRecon(
	// OUT
	unsigned char *actual, 
	int actualStride, 
	// IN
	unsigned char *residual, 
	int residualStride, 
	unsigned char *prediction, 
	int predictionStride, 
	// CONST
	int width,
	int height);

void CalculateReconDWord(
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


#endif