// This header file defines functions that perform a VERY basic form of quantization.
// In the future, this quantization will be replaced by the correct method.

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "../include/Utility.h"

#define USE_REAL_QUANTIZATION	0
#define QP_VAL					0

// Debug flag
#define DEBUG_QUANT				0


// DO NOT TOUCH
#define QP_BASE		(QP_VAL%6)
#define QP_MULT_1	(QP_VAL / 6)

#define QP_MULT_2	(QP_MULT_1 << 1)

#if !QP_MULT_2
#define QP_MULT		1
#else
#define QP_MULT		QP_MULT_2
#endif

#define QBITS			(17 + QP_MULT_1)//(15 + QP_MULT_1)
#define QBITS_ROUND		((1 << (QBITS)))/3

// Stores a^2, (b^2)/4, ab/2
static const int QPP[6][3] = 
{
	//{13107, 5243,  8066}, // QP0
	{13107, 5243/**/, 8066}, // QP0 // MODIFIED, 4.0, 2.56, 3.2


	{11916, 4660,  7490}, // QP1
	{10082, 4194,  6554}, // QP2
	{9362,  3647,  5825}, // QP3
	{8192,  3355,  5243}, // QP4
	{7282,  2893,  4559}, // QP5
};

// Stores a^2, (b^2)/4, ab/2
static const int IQPP[6][3] = 
{
	{10,  16,  13}, // QP0
	{11,  18,  14}, // QP1
	{13,  20,  16}, // QP2
	{14,  23,  18}, // QP3
	{16,  25,  20}, // QP4
	{18,  29,  23}, // QP5
};

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