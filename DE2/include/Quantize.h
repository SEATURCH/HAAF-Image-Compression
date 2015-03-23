// This header file defines functions that perform a VERY basic form of quantization.
// In the future, this quantization will be replaced by the correct method.

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "../include/Utility.h"

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

#endif