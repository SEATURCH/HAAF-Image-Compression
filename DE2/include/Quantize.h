// This header file defines functions that perform a VERY basic form of quantization.
// In the future, this quantization will be replaced by the correct method.

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "../include/Utility.h"

// Debug flag
#define DEBUG_QUANT				0

typedef enum QuantizationDirection_s
{
	QuantizeForward,
	QuantizeBackward,
	QuantizeModeCount
} QuantizationDirection_t;

void Quantize(
	int *coeff, 
	int coeffStride, 
	int width, 
	int height, 
#if USE_REAL_QUANTIZATION
	int (*quantTable)[CODING_UNIT_WIDTH]
#else
	int qp
#endif
	);
void InverseQuantize(
	int *coeff, 
	int coeffStride, 
	int width, 
	int height, 
#if USE_REAL_QUANTIZATION
	int (*iQuantTable)[CODING_UNIT_WIDTH]
#else
	int qp
#endif
	);

static void (*QuantizationFuncPtrTable[QuantizeModeCount]) (
	int *coeff, 
	int coeffStride, 
	int width, 
	int height, 
#if USE_REAL_QUANTIZATION
	int (*quantTable)[CODING_UNIT_WIDTH]
#else
	int qp
#endif
	) = 
{
	Quantize,
	InverseQuantize
};

#endif