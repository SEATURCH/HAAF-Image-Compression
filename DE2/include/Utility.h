#ifndef UTILITY_H
#define UTILITY_H


#include "stdio.h"
#include "stdlib.h"
#include "string.h"


/*** DEFINES ***/

#define MAX_CU_DEPTH             6                          // log2(CTUSize)
#define MAX_CU_SIZE             (1<<(MAX_CU_DEPTH))         // maximum allowable size of CU, surely 64? (not 1<<7 = 128)

// This algorithm only processes blocks that are 16x16 in size
#define CODING_UNIT_WIDTH			(16)
#define CODING_UNIT_HEIGHT			(16)
#define CODING_UNIT_SCRATCH_SIZE	(CODING_UNIT_WIDTH*CODING_UNIT_HEIGHT)

#define CODING_UNIT_REF_BUFFER_SIZE_Y		(CODING_UNIT_WIDTH + CODING_UNIT_HEIGHT + 1)


#define CODING_UNIT_REF_BUFFER_SIZE_UV		((CODING_UNIT_WIDTH >> 1) + (CODING_UNIT_HEIGHT >> 1) + 1)

// Input Format:
// Width/Height must be divisible evenly by 16
// YUV420 only

#define DEFAULT_PICTURE_WIDTH		(320)
#define DEFAULT_PICTURE_HEIGHT		(240)
#define PICTURE_YUV420_SIZE (DEFAULT_PICTURE_WIDTH*DEFAULT_PICTURE_HEIGHT*3)/2

#define BIT_DEPTH					(8) // 1 byte
#define DEFAULT_SAMPLE_VALUE		(1 << (BIT_DEPTH - 1))

#define DEFAULT_QP_VALUE			100


/*** MACRO FUNCTIONS ***/

#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)


/*** FUNCTIONS ***/
int Clip3(const int minVal, const int maxVal, const int a);
void PrintBlock(unsigned char *inputBlock, int inputStride, int width, int height);
void FindMaxMinInt(int *inputBlock, int inputStride, int width, int height, int *max, int *min);
void PrintBlockSubtraction(unsigned char *leftBlock, int leftStride, unsigned char *rightBlock, int rightStride, int width, int height);
void PrintBlockInt(int *inputBlock, int inputStride, int width, int height);
void PrintReferenceBuffer(unsigned char *referenceBufferY, int cuX, int cuY, int size);
void SetInputPictureSamplesToValue(unsigned char *inputPicture, int width, int height, int value);
void SetInputPictureSamplesToArbitrary(unsigned char *inputPicture, int width, int height);

#endif
