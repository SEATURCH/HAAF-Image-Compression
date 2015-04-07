// This header file defines functions that perform a VERY basic form of quantization.
// In the future, this quantization will be replaced by the correct method.

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "../include/Utility.h"

// Debug flag
#define DEBUG_QUANT				0


#define QP_MAX		51

// QUANTIZATION PARAMS
#define AzA_MIN		80
#define BzB_MIN		80
#define CzC_MIN		80
#define DzD_MIN		33
#define EzE_MIN		33
#define FzF_MIN		33
#define GzG_MIN		33
#define HzH_MIN		33
#define IzI_MIN		33
#define JzJ_MIN		40
#define KzK_MIN		40
#define LzL_MIN		40
#define MzM_MIN		40
#define NzN_MIN		47
#define OzO_MIN		48
#define PzP_MIN		49
#define QzQ_MIN		50
#define RzR_MIN		105
#define SzS_MIN		110
#define TzT_MIN		131
#define UzU_MIN		133
#define VzV_MIN		135
#define WzW_MIN		145
#define XzX_MIN		140
#define YzY_MIN		120
#define ZzZ_MIN		100
 
// QP 51
#define AzA_MAX		300
#define BzB_MAX		300
#define CzC_MAX		300
#define DzD_MAX		250
#define EzE_MAX		250
#define FzF_MAX		250
#define GzG_MAX		250
#define HzH_MAX		250
#define IzI_MAX		250
#define JzJ_MAX		250
#define KzK_MAX		250
#define LzL_MAX		250
#define MzM_MAX		250
#define NzN_MAX		250
#define OzO_MAX		250
#define PzP_MAX		250
#define QzQ_MAX		250
#define RzR_MAX		400
#define SzS_MAX		400
#define TzT_MAX		400
#define UzU_MAX		400
#define VzV_MAX		400
#define WzW_MAX		600
#define XzX_MAX		600
#define YzY_MAX		600
#define ZzZ_MAX		600

#define GetQuantizationParam(max, min, qpMax, qpValue) ((((max - min) / qpMax) * qpValue) + min)

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
	int *qp
	);
void InverseQuantize(
	int *coeff, 
	int coeffStride, 
	int width, 
	int height, 
	int *qp
	);

static void (*QuantizationFuncPtrTable[QuantizeModeCount]) (
	int *coeff, 
	int coeffStride, 
	int width, 
	int height,
	int *qp
	) = 
{
	Quantize,
	InverseQuantize
};

#endif