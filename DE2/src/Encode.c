#include "../include/Encode.h"
#include "../include/Prediction.h"
#include "../include/Transform.h"
#include "../include/Quantize.h"


// Copies the reference samples from inputY based on the CU's position.
// If a sample does not exist in the CU, the DEFAULT_SAMPLE_VALUE is used
void CopyReferenceSamples(
	unsigned char *referenceBuffer, 
	unsigned char *inputY, 
	int cuX, 
	int cuY, 
	int stride,
	int codingUnitWidth,
	int codingUnitHeight)
{
	int topRowOffset = codingUnitWidth + 1;
	int referenceBufferSize = codingUnitWidth + codingUnitHeight + 1;
	
	// Copy reference samples into reference buffer based off cuIndex
	// Make sure to account for out-of-bounds
	// Top Left
	if((!cuX) && (!cuY))
	{
		memset(referenceBuffer, DEFAULT_SAMPLE_VALUE, referenceBufferSize);
	}
	// Reference non-existant on left
	else if(!cuX)
	{
		// Point to (0, -1) for top line of Y samples
		unsigned char *samplePtr = inputY - stride;

		// Copy first value as default sample value
		referenceBuffer[0] = DEFAULT_SAMPLE_VALUE;
		// Copy 16 references above
		memcpy(&referenceBuffer[1], samplePtr, codingUnitWidth);
		// Set remaining 16 references to default
		memset(&referenceBuffer[topRowOffset], DEFAULT_SAMPLE_VALUE, codingUnitHeight);

	}
	// Reference non-existant on top
	else if(!cuY)
	{
		int yCursor = 0;

		// Start pointer to the left of the first sample
		unsigned char *samplePtr = inputY- 1;

		// Copy default luma sample value into first 17 samples as they are out of range
		memset(referenceBuffer, DEFAULT_SAMPLE_VALUE, topRowOffset);

		// Iterate by stride and copy pixels in one value at a time
		yCursor = 0;
		while(yCursor < codingUnitHeight)
		{
			referenceBuffer[topRowOffset + yCursor] = *samplePtr;

			yCursor++;
			samplePtr += stride;
		}
	}
	else
	{
		int yCursor = 0;

		// Point to cuStart offset by (-1, -1)
		// Since this CU CANNOT be on the top left due to the IF condition,
		// we can subtract this pointer without any issues.
		unsigned char *samplePtr = inputY - stride - 1;

		// Copy into [0..16] (horizontal row)
		memcpy(referenceBuffer, samplePtr, topRowOffset);

		// Iterate by stride and copy pixels in one value at a time
		samplePtr += stride;
		yCursor = 0;
		while(yCursor < codingUnitHeight)
		{
			referenceBuffer[topRowOffset + yCursor] = *samplePtr;

			yCursor++;
			samplePtr += stride;
		}
	}
}


int ComputeCost(
	unsigned char *input, 
	int inputStride, 
	unsigned char *recon, 
	int reconStride, 
	int width, 
	int height)
{
	int cursorX = 0;
	int cursorY = 0;

	int cost = 0;

	for(cursorY = 0; cursorY < height; cursorY++)
	{
		for(cursorX = 0; cursorX < width; cursorX++)
		{
			int difference = input[cursorY*inputStride + cursorX] - recon[cursorY*reconStride + cursorX];
			cost += (difference * difference);
		}
	}

	return cost;
}

void EncodeDecode(
	// OUT
	CuIntBuffer transformBufferDWord, 
	CuBuffer reconBuffer,
	// IN
	unsigned char *inputCU,
	int cuStride,
	unsigned char *referenceBuffer,
	int predictionMode, 
	int codingUnitWidth, 
	int codingUnitHeight,
	int qp)
{
	CuBuffer	predictionBuffer;
	CuIntBuffer residualBufferDWord;
	CuIntBuffer invTransformBufferDWord;
	CuIntBuffer reconBufferDWord;

	// Create prediction based off reference and predictionModeCursor
	PredictionFuncPtrTable[predictionMode](
		predictionBuffer, 
		codingUnitWidth, 
		referenceBuffer, 
		codingUnitWidth);

	// ***32bits rather than 8bits are used for each sample from this point on.

	// Calculate the residual from the prediction
	CalculateResidualDWord(
		residualBufferDWord, 
		codingUnitWidth, 
		inputCU, 
		cuStride, 
		predictionBuffer, 
		codingUnitWidth, 
		codingUnitWidth, 
		codingUnitHeight);

	// Transform
	xTrMxN(
		residualBufferDWord, 
		transformBufferDWord, 
		codingUnitWidth, 
		codingUnitHeight);

	// Quantize
	QuantizationFuncPtrTable[QuantizeForward](
		transformBufferDWord, 
		codingUnitWidth, 
		codingUnitWidth, 
		codingUnitHeight, 
		qp);


	// **At this point, Encoding is finished,we can harvest the transform buffer for use with Huffman Encoding**

	/***** DECODE *****/
	// Decode the transform to determine the cost for the prediction mode

	// Inverse Quantize
	QuantizationFuncPtrTable[QuantizeBackward](
		transformBufferDWord, 
		codingUnitWidth, 
		codingUnitWidth, 
		codingUnitHeight, 
		qp);

	// Inverse Transform
	xITrMxN(
		transformBufferDWord, 
		invTransformBufferDWord, 
		codingUnitWidth, 
		codingUnitHeight);
	
	// Add the prediction to the inverse transform to get the 'actual'
	CalculateReconDWord(
		reconBufferDWord, 
		codingUnitWidth, 
		invTransformBufferDWord, 
		codingUnitWidth, 
		predictionBuffer, 
		codingUnitWidth, 
		codingUnitWidth, 
		codingUnitHeight);
		
	// Saturate the 32bit recon into 8 bits
	CopyDWordToByteBuffer(
		reconBufferDWord,
		reconBuffer,
		(codingUnitWidth)*(codingUnitHeight));
		
}

void EncodeCu(
	CodingUnitStructure_t *codingUnitStructure, 
	int cuX, 
	int cuY)
{
	// Encode Buffers
	CuIntBuffer transformBufferDWord;

	// Decode Buffers (Mode Decision)
	CuBuffer reconBuffer; // Will be compared to Actual Picture

	int cuIndex = (cuY * codingUnitStructure->numCusWidth) + cuX;

	// pointer to the input Picture at the CU's location
	CodingUnit_t *codingUnit = &codingUnitStructure->codingUnits[cuIndex];

	BufferDescriptor_t *inputPicture = &codingUnitStructure->inputPicture;
	BufferDescriptor_t *transformBestBuffer = &codingUnitStructure->transformBestBuffer;
	BufferDescriptor_t *reconBestBuffer = &codingUnitStructure->reconBestBuffer;

	// Unsigned Char Blocks
	unsigned char *inputY = &inputPicture->yBuffer[codingUnit->yBufferOffset];
	unsigned char *inputU = &inputPicture->uBuffer[codingUnit->uvBufferOffset];
	unsigned char *inputV = &inputPicture->vBuffer[codingUnit->uvBufferOffset];
	int yStride = inputPicture->yStride;
	int uStride = inputPicture->uStride;
	int vStride = inputPicture->vStride;

	// Recon
	unsigned char *reconBestY = &reconBestBuffer->yBuffer[codingUnit->yBufferOffset];
	unsigned char *reconBestU = &reconBestBuffer->uBuffer[codingUnit->uvBufferOffset];
	unsigned char *reconBestV = &reconBestBuffer->vBuffer[codingUnit->uvBufferOffset];
	int reconYStride = reconBestBuffer->yStride;
	int reconUStride = reconBestBuffer->uStride;
	int reconVStride = reconBestBuffer->vStride;

	// Transform Int Blocks (uses custom offset since it is in int space
	int transformStrideY = transformBestBuffer->yStride;
	int transformStrideU = transformBestBuffer->uStride;
	int transformStrideV = transformBestBuffer->vStride;
	unsigned char *transformBestY = &transformBestBuffer->yBuffer[(yStride * cuY) + (cuX * transformBestBuffer->sampleSize)];
	unsigned char *transformBestU = &transformBestBuffer->uBuffer[(uStride * cuY) + (cuX * transformBestBuffer->sampleSize)];
	unsigned char *transformBestV = &transformBestBuffer->vBuffer[(vStride * cuY) + (cuX * transformBestBuffer->sampleSize)];

	// Units are in bytes
	int transformWidth = CODING_UNIT_WIDTH * transformBestBuffer->sampleSize;

	// Will contain the reference for the CU
	// [0] : pixel offset (-1, -1) from CU start
	// [1-16] : pixels offset from (0, -1) to (15, -1)
	// [17-32] : pixels offset from (-1, 0) to (-1, 15)
	unsigned char referenceBuffer[CODING_UNIT_REF_BUFFER_SIZE_Y];

	int predictionModeCursor = 0;
	
	// Copy the samples into the reference buffer
	CopyReferenceSamples(
		referenceBuffer, 
		reconBestY,//inputY,
		cuX, 
		cuY, 
		reconYStride,//yStride, 
		CODING_UNIT_WIDTH, 
		CODING_UNIT_HEIGHT);

	//printf("Position: (%d, %d)\n", cuX, cuY);

	// Prediction Mode Loop
	for(predictionModeCursor = 0; predictionModeCursor < PredictionModeCount; predictionModeCursor++)
	{
		int currentPredictionModeCost;

		///***** ENCODING/DECODING *****/
		EncodeDecode(
			transformBufferDWord, 
			reconBuffer, 
			inputY, 
			yStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH, 
			CODING_UNIT_HEIGHT,
			DEFAULT_QP_VALUE);

		/***** COST CALCULATION *****/
		// Determine the cost of this prediction mode, and update if necessary

		currentPredictionModeCost = ComputeCost(
										inputY, 
										yStride, 
										reconBuffer, 
										CODING_UNIT_WIDTH, 
										CODING_UNIT_WIDTH, 
										CODING_UNIT_HEIGHT);

		// Check new cost with current best cost
		codingUnit->predictionModeCost[predictionModeCursor] = currentPredictionModeCost;
		if(codingUnit->predictionModeCost[predictionModeCursor] < codingUnit->predictionModeCost[ codingUnitStructure->bestPredictionModes[cuIndex] ]
			|| !predictionModeCursor)
		{
			// Update the best prediction mode and copy the transform and recon into the buffers
			codingUnitStructure->bestPredictionModes[cuIndex] = (PredictionMode_t) predictionModeCursor;
			// Copy transform buffer into transform best buffer
			CopyBlockByte(
				(unsigned char *)transformBufferDWord, 
				CODING_UNIT_WIDTH * sizeof(int), 
				transformBestY, 
				transformStrideY, 
				CODING_UNIT_WIDTH * sizeof(int), 
				CODING_UNIT_HEIGHT);

			// Copy recon buffer into recon best buffer
			CopyBlockByte(
				reconBuffer, 
				CODING_UNIT_WIDTH, 
				reconBestY, 
				reconYStride, 
				CODING_UNIT_WIDTH, 
				CODING_UNIT_HEIGHT);
		}
	}


	// Encode U and V according to bestPredictionModes (determined from Y in previous loop)
	/***** U *****/
	{
		predictionModeCursor = codingUnitStructure->bestPredictionModes[cuIndex];
		// Copy the samples into the reference buffer
		CopyReferenceSamples(
			referenceBuffer, 
			inputU,
			cuX, 
			cuY, 
			uStride, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1);

		EncodeDecode(
			transformBufferDWord, 
			reconBuffer, 
			inputU, 
			uStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			DEFAULT_QP_VALUE);

		// Copy transform buffer into transform best buffer
		CopyBlockByte(
			(unsigned char *)transformBufferDWord, 
			(CODING_UNIT_WIDTH >> 1) * sizeof(int), 
			transformBestU, 
			transformStrideU, 
			(CODING_UNIT_WIDTH >> 1) * sizeof(int), 
			(CODING_UNIT_HEIGHT >> 1));

		// Copy recon buffer into recon best buffer
		CopyBlockByte(
			reconBuffer, 
			(CODING_UNIT_WIDTH >> 1), 
			reconBestU, 
			reconUStride, 
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));
	}
	
	/***** V *****/
	{
		predictionModeCursor = codingUnitStructure->bestPredictionModes[cuIndex] ;
		// Copy the samples into the reference buffer
		CopyReferenceSamples(
			referenceBuffer, 
			inputV,
			cuX, 
			cuY, 
			vStride, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1);

		EncodeDecode(
			transformBufferDWord, 
			reconBuffer, 
			inputV, 
			vStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			DEFAULT_QP_VALUE);

		// Copy transform buffer into transform best buffer
		CopyBlockByte(
			(unsigned char *)transformBufferDWord, 
			(CODING_UNIT_WIDTH >> 1) * sizeof(int), 
			transformBestV, 
			transformStrideV, 
			(CODING_UNIT_WIDTH >> 1) * sizeof(int), 
			(CODING_UNIT_HEIGHT >> 1));

		// Copy recon buffer into recon best buffer
		CopyBlockByte(
			reconBuffer, 
			(CODING_UNIT_WIDTH >> 1), 
			reconBestV, 
			reconVStride, 
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));
	}
}





void EncodeLoop(CodingUnitStructure_t *codingUnitStructure)
{
	int cuCursorX;
	int cuCursorY;

	// Loop through each CU in raster scan.
	// We can do it in this Raster-Scan order since we are single-threaded.
	// We could parallelize by doing this loop in Z-Scan order.
	for(cuCursorY = 0; cuCursorY < codingUnitStructure->numCusHeight; cuCursorY++)
	{
		for(cuCursorX = 0; cuCursorX < codingUnitStructure->numCusWidth; cuCursorX++)
		{
			EncodeCu(codingUnitStructure, cuCursorX, cuCursorY);

			printf("Finished %d/%d CU's!\n", cuCursorY*codingUnitStructure->numCusWidth + cuCursorX + 1, codingUnitStructure->numCusWidth * codingUnitStructure->numCusHeight);
		}
	}

	// Binary Code the transformBestBuffer and predictionModes
	


	//// TODO: Import LZ4 library into codebase
	//{
	//	int numCUs = codingUnitStructure->numCusHeight * codingUnitStructure->numCusWidth;
	//	// 300: Total number of prediction modes for 320x240
	//	unsigned char outputBuffer[300];
	//	int outputBufferLen;
	//
	//	PredictionMode_t predictionModeBuffer[300];
	//	int numPredictionModes;
	//
	//	int predCursor = 0;
	//
	//	// Testing encoding Prediction Modes
	//	EncodePredictionModes(
	//		outputBuffer,
	//		&outputBufferLen,
	//		codingUnitStructure->bestPredictionModes,
	//		numCUs,
	//		PredictionModeCount);
	//
	//	// Test decoding of Prediction Modes
	//	DecodePredictionModes(
	//		predictionModeBuffer,
	//		&numPredictionModes,
	//		outputBuffer,
	//		outputBufferLen,
	//		PredictionModeCount);
	//
	//	for(predCursor = 0; predCursor < numCUs; predCursor++)
	//	{
	//		if(codingUnitStructure->bestPredictionModes[predCursor] != predictionModeBuffer[predCursor])
	//		{
	//			printf("F*CKKKKKK!\n");
	//		}
	//	}
	//
	//}




//#define TESTBUFFERSIZE (((320*240)*3)/2)*4
//	// this will probably not work...
//	{
//		int uncompressedBufferSize = TESTBUFFERSIZE;
//		unsigned char *transformCoeffs = codingUnitStructure->transformBestBuffer.fullPicturePointer;
//		unsigned char binaryCodingBuffer[TESTBUFFERSIZE];
//		unsigned char uncompressedBuffer[TESTBUFFERSIZE];
//		int binaryCodingBufferSize;
//		int i;
//		int numCUs = codingUnitStructure->numCusHeight * codingUnitStructure->numCusWidth;
//
//		
//		PredictionMode_t *bcPredictionModes;
//		int bcPredictionModesSize;
//		PredictionMode_t *uncompressedPredictionModes;
//		
//		bcPredictionModes = (PredictionMode_t *) malloc(numCUs * sizeof(PredictionMode_t));
//		uncompressedPredictionModes = (PredictionMode_t *) malloc(numCUs * sizeof(PredictionMode_t));
//
//
//		// Transform Coeffs
//		LZ4IO_compressArray(
//			codingUnitStructure->transformBestBuffer.fullPicturePointer, 
//			uncompressedBufferSize,
//			binaryCodingBuffer,
//			&binaryCodingBufferSize,
//			0 // I don't know how to use this input
//			);
//
//		LZ4IO_decompressArray(
//			binaryCodingBuffer, 
//			binaryCodingBufferSize,
//			uncompressedBuffer,
//			uncompressedBufferSize);
//		
//		// Check uncompressed transform coeffs
//		for(i = 0; i < uncompressedBufferSize; i++)
//		{
//			if(transformCoeffs[i] != uncompressedBuffer[i])
//			{
//				printf("Fuck!!!\n");
//			} 
//		}
//
//		// Prediction Modes
//		LZ4IO_compressArray(
//			(unsigned char *) codingUnitStructure->bestPredictionModes, 
//			numCUs * sizeof(PredictionMode_t),
//			(unsigned char *) bcPredictionModes,
//			&bcPredictionModesSize,
//			0 // I don't know how to use this input
//			);
//
//		LZ4IO_decompressArray(
//			(unsigned char *) bcPredictionModes, 
//			bcPredictionModesSize,
//			(unsigned char *) uncompressedPredictionModes,
//			numCUs * sizeof(PredictionMode_t));
//		
//		for(i = 0; i < numCUs; i++)
//		{
//			if(codingUnitStructure->bestPredictionModes[i] != uncompressedPredictionModes[i])
//			{
//				printf("Fuck!!!\n");
//			} 
//		}
//
//
//		printf("done encoding!\n");
//		codingUnitStructure->transformBestBuffer;
//
//		free(bcPredictionModes);
//		free(uncompressedPredictionModes);
//}

}


// Requires EncodeLoop has run successfully on codingUnitStructure
void GenerateBitstream(
	CodingUnitStructure_t *codingUnitStructure,
	Bitstream_t *outputBitstream)
{
	int numCUs = codingUnitStructure->numCusHeight * codingUnitStructure->numCusHeight;

	EncodeBitstream(
		outputBitstream,
		(unsigned char *) codingUnitStructure->transformBestBuffer.fullPicturePointer,
		codingUnitStructure->transformBestBuffer.yuvSize,
		codingUnitStructure->bestPredictionModes,
		numCUs,
		codingUnitStructure->widthPicture,
		codingUnitStructure->heightPicture);

}