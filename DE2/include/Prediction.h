// This header file specifies the prediction modes inside the encoder

#ifndef PREDICTION_H
#define PREDICTION_H

#include "../include/Utility.h"

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