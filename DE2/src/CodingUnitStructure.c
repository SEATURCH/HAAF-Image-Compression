#include "../include/CodingUnitStructure.h"
#include "../include/Prediction.h"
#include "../include/Transform.h"
#include "../include/Quantize.h"


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

	int ySize = (width * sampleSize) * height;
	int uSize = (uvStride * sampleSize) * uvHeight;
	int vSize = (uvStride * sampleSize) * uvHeight;

	// Y Buffer starts at beginning
	int yOffset = 0;
	int uOffset = yOffset + ySize;
	int vOffset = uOffset + uSize;

	pictureBuffer->yuvSize = ySize + uSize + vSize;

	// Allocate memory for the entire picture (YUV420)
	pictureBuffer->fullPicturePointer = (unsigned char *) malloc( pictureBuffer->yuvSize * sizeof(unsigned char));

	// Set stride/height
	// Y
	pictureBuffer->yStride = width * sampleSize;
	pictureBuffer->yHeight = height;
	pictureBuffer->yBuffer = &pictureBuffer->fullPicturePointer[yOffset];

	// U
	pictureBuffer->uStride = uvStride * sampleSize;
	pictureBuffer->uHeight = uvHeight;
	pictureBuffer->uBuffer = &pictureBuffer->fullPicturePointer[uOffset];

	// V
	pictureBuffer->vStride = uvStride * sampleSize;
	pictureBuffer->vHeight = uvHeight;
	pictureBuffer->vBuffer = &pictureBuffer->fullPicturePointer[vOffset];

	// Default is 1 (unsigned char), but 4 (int) is used for transform buffer
	pictureBuffer->sampleSize = sampleSize;
}

void BufferDescriptorDeconstructor(BufferDescriptor_t *pictureBuffer)
{
	free(pictureBuffer->fullPicturePointer);
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
	//BufferDescriptorConstructor(
	//	&codingUnitStructure->inputPicture,
	//	codingUnitStructure->widthPicture,
	//	codingUnitStructure->heightPicture,
	//	sizeof(unsigned char));
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
	// Allocate memory for prediction modes
	codingUnitStructure->bestPredictionModes = (PredictionMode_t *) malloc( (codingUnitStructure->numCusWidth * codingUnitStructure->numCusHeight) * sizeof(PredictionMode_t));

	for(yCuIndex = 0; yCuIndex < codingUnitStructure->numCusHeight; ++yCuIndex)
	{
		for(xCuIndex = 0; xCuIndex < codingUnitStructure->numCusWidth; ++xCuIndex)
		{
			int predictionModeCursor = 0;
			cuIndex = (yCuIndex * codingUnitStructure->numCusWidth) + xCuIndex;

			// Initialize base variables
			codingUnitStructure->codingUnits[cuIndex].blockIndex = cuIndex;
			
			codingUnitStructure->bestPredictionModes[cuIndex] = DC;
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
	
#if USE_REAL_QUANTIZATION
	// Initialize Quantization Tables
	{

		// Quantization Multipliers
		int QPT0 = QPP[QP_BASE][0] / QP_MULT;
		int QPT1 = QPP[QP_BASE][1] / QP_MULT;
		int QPT2 = QPP[QP_BASE][2] / QP_MULT;

		// Inverse Quantization Multipliers
		int IQPT0 = IQPP[QP_BASE][0] * QP_MULT;
		int IQPT1 = IQPP[QP_BASE][1] * QP_MULT;
		int IQPT2 = IQPP[QP_BASE][2] * QP_MULT;

		int quantCursor = 0;

		
		printf("QPT[0]: %d\nQPT[1]: %d\nQPT[2]: %d\n\n", QPT0, QPT1, QPT2);
		printf("IQPT[0]: %d\nIQPT[1]: %d\nIQPT[2]: %d\n\n", IQPT0, IQPT1, IQPT2);
		printf("QBITS: %d\nQBITS_ROUND: %d\n\n", QBITS, QBITS_ROUND);

		// 0th row, even vals
		for(quantCursor = 0; quantCursor < CODING_UNIT_WIDTH; quantCursor += 2)
		{
			codingUnitStructure->QuantTable[0][quantCursor] = QPT0;
			codingUnitStructure->IQuantTable[0][quantCursor] = IQPT0;

			codingUnitStructure->QuantTable[1][quantCursor] = QPT2;
			codingUnitStructure->IQuantTable[1][quantCursor] = IQPT2;
		}

		for(quantCursor = 1; quantCursor < CODING_UNIT_WIDTH; quantCursor += 2)
		{
			codingUnitStructure->QuantTable[0][quantCursor] = QPT2;
			codingUnitStructure->IQuantTable[0][quantCursor] = IQPT2;

			codingUnitStructure->QuantTable[1][quantCursor] = QPT1;
			codingUnitStructure->IQuantTable[1][quantCursor] = IQPT1;
		}

	}
#endif
	
}

void CodingUnitStructureDeconstructor(CodingUnitStructure_t *codingUnitStructure)
{
	//BufferDescriptorDeconstructor(&codingUnitStructure->inputPicture);
	BufferDescriptorDeconstructor(&codingUnitStructure->transformBestBuffer);
	BufferDescriptorDeconstructor(&codingUnitStructure->reconBestBuffer);
	
	free(codingUnitStructure->bestPredictionModes);
	free(codingUnitStructure->codingUnits);
}


void SetInputPicture(
	CodingUnitStructure_t *codingUnitStructure,
	BufferDescriptor_t *inputPicture)
{
	codingUnitStructure->inputPicture = inputPicture;
}
