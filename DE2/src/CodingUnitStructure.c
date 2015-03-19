// CodingUnitStructure.c

#include "stdio.h"
#include "stdlib.h"
#include "CodingUnitStructure.h"
#include "Transform.h"

/*** PREDICTION ***/

// Creates an Average of the reference and assigns it to every sample in dst
void PredictionModeDC(unsigned char *dst, int dstStride, unsigned char *referenceBuffer, int size)
{
	int puCursorX;
	int puCursorY;

	int refCursor = 0;
	int sum = 0;
	int average;

	// Obtain the average of the samples
	for(refCursor = 0; refCursor < CODING_UNIT_REF_BUFFER_SIZE_Y; refCursor++)
	{
		sum += referenceBuffer[refCursor];
	}
	average = (sum + (CODING_UNIT_REF_BUFFER_SIZE_Y >> 1)) / CODING_UNIT_REF_BUFFER_SIZE_Y;



	// Set everything to average
	for(puCursorY = 0; puCursorY < size; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < size; puCursorX++)
		{
			dst[(puCursorY * dstStride) + puCursorX] = average;
		}
	}
}

// Vertically copies the referenceBuffer onto dst
void PredictionModeVertical(unsigned char *dst, int dstStride, unsigned char *referenceBuffer, int size)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < size; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < size; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			dst[(puCursorY * dstStride) + puCursorX] = referenceBuffer[1 + puCursorX];
		}
	}
}

// Horizontally copies the referenceBuffer onto dst
void PredictionModeHorizontal(unsigned char *dst, int dstStride, unsigned char *referenceBuffer, int size)
{
	const int topRowOffset = size + 1;
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < size; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < size; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			dst[(puCursorY * dstStride) + puCursorX] = referenceBuffer[topRowOffset + puCursorY];
		}
	}
}

// Performs planar prediction
// Obtained from:
// http://wiki.multimedia.cx/index.php?title=H.264_Prediction#16x16_Prediction_Modes
void PredictionModePlanar(unsigned char *dst, int dstStride, unsigned char *referenceBuffer, int size)
{
	const int topRowOffset = size + 1;
	const int bitWidth = 5;
	int cursor;

	int puCursorX;
	int puCursorY;

	int hPrime = 0;
	int vPrime = 0;

	int h;
	int v;

	int a;
	int b;

	// Horizontal
	for(cursor = 0; cursor < (size >> 1); cursor++)
	{
		hPrime += ((size >> 1) - cursor) * (referenceBuffer[size - cursor] - referenceBuffer[cursor]);
	}

	// Vertical
	for(cursor = 0; cursor < ((size >> 1) - 1); cursor++)
	{
		vPrime += ((size >> 1) - cursor - 1) * (referenceBuffer[size - 2 - cursor + topRowOffset] - referenceBuffer[cursor + topRowOffset]);
	}
	vPrime += (size >> 1) * (referenceBuffer[size + topRowOffset - 1] - referenceBuffer[0]);

	// Final offsets
	h = (bitWidth * hPrime + (1 << bitWidth)) >> (bitWidth + 1);
	v = (bitWidth * vPrime + (1 << bitWidth)) >> (bitWidth + 1);
	a = ((size * (referenceBuffer[size + topRowOffset - 1]
								+ referenceBuffer[topRowOffset - 1] + 1))
							- 7 * (v + h));


	// Set everything to average
	for(puCursorY = 0; puCursorY < size; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < size; puCursorX++)
		{
			b = a + v*puCursorY + h*puCursorX;
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			dst[(puCursorY * dstStride) + puCursorX] = Clip3(0, 255, b >> bitWidth);
		}
	}
}

// Calculates the residual between specific pixels between the actual and prediction buffers
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
	int height)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			residual[puCursorY*residualStride + puCursorX] = (1 << BIT_DEPTH) + (actual[puCursorY*actualStride + puCursorX] - prediction[puCursorY*predictionStride + puCursorX]);
		}
	}
}


// Calculates the integer residual between specific pixels between the actual and prediction buffers
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
	int height)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			residual[puCursorY*residualStride + puCursorX] = actual[puCursorY*actualStride + puCursorX] - prediction[puCursorY*predictionStride + puCursorX];
		}
	}
}




// Calculates the reconstructed picture based on the residual and prediction
void CalculateActual(
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
	int height)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			 actual[puCursorY*actualStride + puCursorX] = residual[puCursorY*residualStride + puCursorX] + prediction[puCursorY*predictionStride + puCursorX];
		}
	}
}

// Calculates the int reconstructed picture based on the residual and prediction
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
	int height)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			 recon[puCursorY*reconStride + puCursorX] = residual[puCursorY*residualStride + puCursorX] + prediction[puCursorY*predictionStride + puCursorX];
		}
	}
}


// Returns 1 if buffers are identical
// Returns 0 if buffers are NOT identical
int CheckIdenticalBuffers(
	unsigned char *left,
	int leftStride,
	unsigned char *right,
	int rightStride,
	int width,
	int height)
{
	int puCursorX;
	int puCursorY;
	

#define ABS_DIFF(a, b) (a>b?a-b:b-a)
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			 if(left[puCursorY*leftStride + puCursorX] != right[puCursorY*rightStride + puCursorX])
			 {
				 return 0;
			 }
		}
	}
	return 1;
}

// Copies a block of unsigned char from src to dest
void CopyBlockByte(
	unsigned char *src,
	int srcStride,
	unsigned char *dest,
	int destStride,
	int width,
	int height)
{
	int puCursorX;
	int puCursorY;
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			 dest[puCursorY*destStride + puCursorX] = src[puCursorY*srcStride + puCursorX];
		}
	}
}

/*** FUNCTIONS ***/

// Picture Buffer Constructor
// Since input format is restricted to YUV420, we can easily compute Y and UV strides
void BufferDescriptorConstructor(BufferDescriptor_t *pictureBuffer,
	int width,
	int height,
	int sampleSize)
{
	int uvStride = width >> 1;
	int uvHeight = height >> 1;

	// Set stride/height
	// Y
	pictureBuffer->yStride = width * sampleSize;
	pictureBuffer->yHeight = height;
	palloc(&pictureBuffer->yBuffer, pictureBuffer->yStride, pictureBuffer->yHeight);

	// U
	pictureBuffer->uStride = uvStride * sampleSize;
	pictureBuffer->uHeight = uvHeight;
	palloc(&pictureBuffer->uBuffer, pictureBuffer->uStride, pictureBuffer->uHeight);

	// V
	pictureBuffer->vStride = uvStride * sampleSize;
	pictureBuffer->vHeight = uvHeight;
	palloc(&pictureBuffer->vBuffer, pictureBuffer->vStride, pictureBuffer->vHeight);

	// Default is 1 (unsigned char), but 4 (int) is used for transform buffer
	pictureBuffer->sampleSize = sampleSize;
}

void BufferDescriptorDeconstructor(BufferDescriptor_t *pictureBuffer)
{
	free(pictureBuffer->yBuffer);
	free(pictureBuffer->uBuffer);
	free(pictureBuffer->vBuffer);
}


void CodingUnitStructureConstructor(
	CodingUnitStructure_t *codingUnitStructure, 
	int pictureWidth, 
	int pictureHeight)
{
	int xCuIndex = 0;
	int yCuIndex = 0;
	int cuIndex = 0;

	/** Picture Buffers **/
	// Input Picture Set-up
	codingUnitStructure->widthPicture = pictureWidth;
	codingUnitStructure->heightPicture = pictureHeight;

	// Initialize Picture Buffers
	// Normal Dims
	BufferDescriptorConstructor(
		&codingUnitStructure->inputPicture,
		codingUnitStructure->widthPicture,
		codingUnitStructure->heightPicture,
		sizeof(unsigned char));
	BufferDescriptorConstructor(
		&codingUnitStructure->transformBestBuffer,
		codingUnitStructure->widthPicture,
		codingUnitStructure->heightPicture,
		sizeof(int));
	BufferDescriptorConstructor(
		&codingUnitStructure->reconBestBuffer,
		codingUnitStructure->widthPicture,
		codingUnitStructure->heightPicture,
		sizeof(unsigned char));

	/** Coding Units **/
	codingUnitStructure->numCusWidth = codingUnitStructure->widthPicture / CODING_UNIT_WIDTH;
	codingUnitStructure->numCusHeight = codingUnitStructure->heightPicture / CODING_UNIT_HEIGHT;

	// Allocate memory for coding units
	codingUnitStructure->codingUnits = (CodingUnit_t *) malloc((codingUnitStructure->numCusWidth * codingUnitStructure->numCusHeight) * sizeof(CodingUnit_t));

	for(yCuIndex = 0; yCuIndex < codingUnitStructure->numCusHeight; ++yCuIndex)
	{
		for(xCuIndex = 0; xCuIndex < codingUnitStructure->numCusWidth; ++xCuIndex)
		{
			int predictionModeCursor = 0;
			cuIndex = (yCuIndex * codingUnitStructure->numCusWidth) + xCuIndex;

			// Initialize base variables
			codingUnitStructure->codingUnits[cuIndex].blockIndex = cuIndex;
			codingUnitStructure->codingUnits[cuIndex].bestPredictionMode = DC;

			// Set all prediction mode costs to infinite
			for(predictionModeCursor = 0; predictionModeCursor < PredictionModeCount; predictionModeCursor++)
			{
				codingUnitStructure->codingUnits[cuIndex].predictionModeCost[predictionModeCursor] = 0xFFFFFFFF;
			}

			// Determine normal offsets
			codingUnitStructure->codingUnits[cuIndex].yBufferOffset = 
				((yCuIndex * CODING_UNIT_HEIGHT) * codingUnitStructure->widthPicture)
				+ ((xCuIndex * CODING_UNIT_WIDTH));
			codingUnitStructure->codingUnits[cuIndex].uvBufferOffset = 
				((yCuIndex * (CODING_UNIT_HEIGHT >> 1))) * (codingUnitStructure->widthPicture >> 1)
				+ (xCuIndex * (CODING_UNIT_WIDTH >> 1));

		}
	}
	//InitConvertToBit();
}

void CodingUnitStructureDeconstructor(CodingUnitStructure_t *codingUnitStructure)
{
	BufferDescriptorDeconstructor(&codingUnitStructure->inputPicture);
	BufferDescriptorDeconstructor(&codingUnitStructure->transformBestBuffer);
	BufferDescriptorDeconstructor(&codingUnitStructure->reconBestBuffer);

	free(codingUnitStructure->codingUnits);
}

void CopyCharToIntBuffer(CuBuffer src, CuIntBuffer dst, int size) {
	int bufferCursor;
	for(bufferCursor=0; bufferCursor<size; bufferCursor++) {
		dst[bufferCursor] = (int) src[bufferCursor];
	}
}

void CopyIntToCharBuffer(CuIntBuffer src, CuBuffer dst,  int size) {
	int bufferCursor;
	for(bufferCursor=0; bufferCursor<size; bufferCursor++) {
		dst[bufferCursor] = (unsigned char) src[bufferCursor];
	}
}


void Quantize(int *coeff, int coeffStride, int width, int height, int qp)
{
	int puCursorX;
	int puCursorY;

	// Lossless Compression
	if(qp == 0)
		return;

	if(qp < 0 || qp > 50000)
	{
		printf("INVALID QP!!!!!\n");
		return;
	}
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] /= qp;
		}
	}
}


void InverseQuantize(int *coeff, int coeffStride, int width, int height, int qp)
{
	int puCursorX;
	int puCursorY;

	// Lossless Compression
	if(qp == 0)
		return;

	if(qp < 0 || qp > 50000)
	{
		printf("INVALID QP!!!!!\n");
		return;
	}
	
	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] *= qp;
		}
	}
}


/*** HELPER FUNCTIONS ***/

// Picture Allocator
void palloc(
	unsigned char **picturePointer, 
	int width, 
	int height)
{
	*picturePointer = (unsigned char *) malloc(width * height);
}
