// This header file specifies the prediction modes inside the encoder

#ifndef PREDICTION_H
#define PREDICTION_H

#include "../include/Utility.h"
#if N2_BUILD
#include "IO.h"
#else
#define DC_BASE_ADDRESS			0
#define VERTICAL_BASE_ADDRESS	0
#define HORIZONTAL_BASE_ADDRESS 0
#define PLANAR_BASE_ADDRESS		0
#endif

typedef enum PredictionMode_s {
	Invalid = -1,
	DC = 0,
	Vertical = 1,
	Horizontal = 2,
	Planar = 3,
	PredictionModeCount = 4,
} PredictionMode_t;

typedef enum ProcessingType_s {
	Software = 0,
	Hardware = 1,
	ProcessingTypeCount
} ProcessingType_t;

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
void PredictionModeDCRead(
		unsigned char *dst,
		int dstStride,
		unsigned char *referenceBuffer,
		int size);
void PredictionModeVerticalRead(
		unsigned char *dst,
		int dstStride,
		unsigned char *referenceBuffer,
		int size);
void PredictionModeHorizontalRead(
		unsigned char *dst,
		int dstStride,
		unsigned char *referenceBuffer,
		int size);
void PredictionModePlanarRead(
		unsigned char *dst,
		int dstStride,
		unsigned char *referenceBuffer,
		int size);

static void (*PredictionFuncPtrTable[ProcessingTypeCount][PredictionModeCount])(
		unsigned char *dst,
		int dstStride,
		unsigned char *referenceBuffer,
		int size
		) =
{
	{
		PredictionModeDC,
		PredictionModeVertical,
		PredictionModeHorizontal,
		PredictionModePlanar,
	},
	{
		PredictionModeDCRead,
		PredictionModeVerticalRead,
		PredictionModeHorizontalRead,
		PredictionModePlanarRead,
	},
};


/*** RESIDUAL/RECON ***/
void CalculateResidual(
		// OUT
		unsigned char *residual, int residualStride,
		// IN
		unsigned char *actual, int actualStride, unsigned char *prediction,
		int predictionStride,
		// CONST
		int width, int height);

void CalculateResidualDWord(
		// OUT
		int *residual, int residualStride,
		// IN
		unsigned char *actual, int actualStride, unsigned char *prediction,
		int predictionStride,
		// CONST
		int width, int height);

void CalculateRecon(
		// OUT
		unsigned char *actual, int actualStride,
		// IN
		unsigned char *residual, int residualStride, unsigned char *prediction,
		int predictionStride,
		// CONST
		int width, int height);

void CalculateReconDWord(
		// OUT
		int *recon, int reconStride,
		// IN
		int *residual, int residualStride, unsigned char *prediction,
		int predictionStride,
		// CONST
		int width, int height);

#endif
