#include "../include/Quantize.h"

#define QP_MAX		51


// qp0
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

#define AA			(((AzA_MAX - AzA_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + AzA_MIN
#define BB			(((BzB_MAX - BzB_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + BzB_MIN
#define CC			(((CzC_MAX - CzC_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + CzC_MIN
#define DD			(((DzD_MAX - DzD_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + DzD_MIN
#define EE			(((EzE_MAX - EzE_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + EzE_MIN
#define FF			(((FzF_MAX - FzF_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + FzF_MIN
#define GG			(((GzG_MAX - GzG_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + GzG_MIN
#define HH			(((HzH_MAX - HzH_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + HzH_MIN
#define II			(((IzI_MAX - IzI_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + IzI_MIN
#define JJ			(((JzJ_MAX - JzJ_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + JzJ_MIN
#define KK			(((KzK_MAX - KzK_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + KzK_MIN
#define LL			(((LzL_MAX - LzL_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + LzL_MIN
#define MM			(((MzM_MAX - MzM_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + MzM_MIN
#define NN			(((NzN_MAX - NzN_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + NzN_MIN
#define OO			(((OzO_MAX - OzO_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + OzO_MIN
#define PP			(((PzP_MAX - PzP_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + PzP_MIN
#define QQ			(((QzQ_MAX - QzQ_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + QzQ_MIN
#define RR			(((RzR_MAX - RzR_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + RzR_MIN
#define SS			(((SzS_MAX - SzS_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + SzS_MIN
#define TT			(((TzT_MAX - TzT_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + TzT_MIN
#define UU			(((UzU_MAX - UzU_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + UzU_MIN
#define VV			(((VzV_MAX - VzV_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + VzV_MIN
#define WW			(((WzW_MAX - WzW_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + WzW_MIN
#define XX			(((XzX_MAX - XzX_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + XzX_MIN
#define YY			(((YzY_MAX - YzY_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + YzY_MIN
#define ZZ			(((ZzZ_MAX - ZzZ_MIN) / QP_MAX) * DEFAULT_QP_VALUE) + ZzZ_MIN




static const int QuantizationTable[16][16] = 
{
	{AA, BB, CC, DD, EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP},
	{BB, CC, DD, EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ},
	{CC, DD, EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR},
	{DD, EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS},
	{EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT},
	{FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU},
	{GG, HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV},
	{HH, II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW},
	{II, JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX},
	{JJ, KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY},
	{KK, LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ},
	{LL, MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ, ZZ},
	{MM, NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ, ZZ, ZZ},
	{NN, OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ, ZZ, ZZ, ZZ},
	{OO, PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ, ZZ, ZZ, ZZ, ZZ},
	{PP, QQ, RR, SS, TT, UU, VV, WW, XX, YY, ZZ, ZZ, ZZ, ZZ, ZZ, ZZ},
};

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
	)
{
	int puCursorX;
	int puCursorY;
	
#if !USE_REAL_QUANTIZATION
	// Lossless Compression
	if(qp == 0)
		return;

	if(qp < 0 || qp > 50000)
	{
		printf("INVALID QP!!!!!\n");
		return;
	}
#endif

#if DEBUG_QUANT
	printf("Quantizing:\n");
#endif
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
#if USE_REAL_QUANTIZATION
			int coeffVal = abs(coeff[(puCursorY * coeffStride) + puCursorX]) << 2;
			char coeffSign = (coeffVal < 0) ? (-1) : (1);
			
			coeff[(puCursorY * coeffStride) + puCursorX] = ((coeffVal * quantTable[puCursorY & 0x1][puCursorX]) + QBITS_ROUND) >> (QBITS);
			coeff[(puCursorY * coeffStride) + puCursorX] *= coeffSign;

#else
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] /= QuantizationTable[puCursorY][puCursorX];//qp;
#endif

#if DEBUG_QUANT
			printf("[%d][%d]: CoeffVal: %d, QuantCoeffVal: %d, quantTable: %d\n", puCursorX, puCursorY, coeffVal, coeff[(puCursorY * coeffStride) + puCursorX], quantTable[puCursorY & 0x1][puCursorX]);
#endif
		}
#if DEBUG_QUANT
		if(puCursorY == 3)
			break;
#endif
	}
}


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
	)
{
	int puCursorX;
	int puCursorY;
	
#if !USE_REAL_QUANTIZATION
	// Lossless Compression
	if(qp == 0)
		return;

	if(qp < 0 || qp > 50000)
	{
		printf("INVALID QP!!!!!\n");
		return;
	}
#endif

#if DEBUG_QUANT
	printf("Inverse Quantize!\n");
#endif
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{

#if DEBUG_QUANT
			printf("[%d][%d]: QuantCoeffVal: %d, ", puCursorX, puCursorY, coeff[(puCursorY * coeffStride) + puCursorX]);
#endif

#if USE_REAL_QUANTIZATION
			coeff[(puCursorY * coeffStride) + puCursorX] *= iQuantTable[puCursorY & 0x1][puCursorX];
#else
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] *= QuantizationTable[puCursorY][puCursorX];//qp;
#endif

#if DEBUG_QUANT
			printf("InvQuantCoeffVal: %d, IQuantTable: %d\n", coeff[(puCursorY * coeffStride) + puCursorX], iQuantTable[puCursorY & 0x1][puCursorX]);
#endif
		}
#if DEBUG_QUANT
		if(puCursorY == 3)
			break;
#endif
	}
}

