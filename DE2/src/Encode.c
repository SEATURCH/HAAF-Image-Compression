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


void DecodeCu(
	CodingUnitStructure_t *codingUnitStructure, 
	int cuX, 
	int cuY,
	int qpValue)
{
	// Encode Buffers
	CuIntBuffer transformBufferDWord;

	// Decode Buffers (Mode Decision)
	CuBuffer reconBuffer; // Will be compared to Actual Picture

	int cuIndex = (cuY * codingUnitStructure->numCusWidth) + cuX;

	// pointer to the input Picture at the CU's location
	CodingUnit_t *codingUnit = &codingUnitStructure->codingUnits[cuIndex];

	///BufferDescriptor_t *inputPicture = codingUnitStructure->inputPicture;
	BufferDescriptor_t *transformBestBuffer = &codingUnitStructure->transformBestBuffer;
	BufferDescriptor_t *reconBestBuffer = &codingUnitStructure->reconBestBuffer;

	// Recon
	unsigned char *reconBestY = &(reconBestBuffer->yBuffer[codingUnit->yBufferOffset]);
	unsigned char *reconBestU = &(reconBestBuffer->uBuffer[codingUnit->uvBufferOffset]);
	unsigned char *reconBestV = &(reconBestBuffer->vBuffer[codingUnit->uvBufferOffset]);
	int reconYStride = reconBestBuffer->yStride;
	int reconUStride = reconBestBuffer->uStride;
	int reconVStride = reconBestBuffer->vStride;

	// Transform Int Blocks (uses custom offset since it is in int space
	int transformStrideY = transformBestBuffer->yStride;
	int transformStrideU = transformBestBuffer->uStride;
	int transformStrideV = transformBestBuffer->vStride;
	unsigned char *transformBestY = &(transformBestBuffer->yBuffer[(transformStrideY * cuY * CODING_UNIT_HEIGHT) + (cuX * CODING_UNIT_WIDTH * transformBestBuffer->sampleSize)]);
	unsigned char *transformBestU = &(transformBestBuffer->uBuffer[(transformStrideU * cuY * (CODING_UNIT_HEIGHT >> 1)) + (cuX * (CODING_UNIT_WIDTH >> 1) * transformBestBuffer->sampleSize)]);
	unsigned char *transformBestV = &(transformBestBuffer->vBuffer[(transformStrideV * cuY * (CODING_UNIT_HEIGHT >> 1)) + (cuX * (CODING_UNIT_WIDTH >> 1) * transformBestBuffer->sampleSize)]);

	// Units are in bytes
	int transformWidth = CODING_UNIT_WIDTH * transformBestBuffer->sampleSize;

	// Will contain the reference for the CU
	// [0] : pixel offset (-1, -1) from CU start
	// [1-16] : pixels offset from (0, -1) to (15, -1)
	// [17-32] : pixels offset from (-1, 0) to (-1, 15)
	unsigned char referenceBuffer[CODING_UNIT_REF_BUFFER_SIZE_Y];

	int predictionMode = codingUnitStructure->bestPredictionModes[cuIndex];
	
	// Copy the samples into the reference buffer
	CopyReferenceSamples(
		referenceBuffer, 
		reconBestY,
		cuX, 
		cuY, 
		reconYStride, 
		CODING_UNIT_WIDTH, 
		CODING_UNIT_HEIGHT);

	///***** DECODING *****/
	CopyDWordToDWordBuffer(
		(int *)transformBestY,
		transformStrideY >> 2,  // >> 2 because transformStrideY is in bytes!
		transformBufferDWord,
		CODING_UNIT_WIDTH,
		CODING_UNIT_WIDTH,
		CODING_UNIT_HEIGHT);

	Decode(
		transformBufferDWord, 
		reconBuffer, 
		referenceBuffer, 
		predictionMode, 
		CODING_UNIT_WIDTH, 
		CODING_UNIT_HEIGHT,
		qpValue);

	// Copy recon buffer into recon best buffer
	CopyBlockByte(
		reconBuffer, 
		CODING_UNIT_WIDTH, 
		reconBestY, 
		reconYStride, 
		CODING_UNIT_WIDTH, 
		CODING_UNIT_HEIGHT);


	// Encode U and V according to bestPredictionModes (determined from Y in previous loop)
	/***** U *****/
	{
		// Copy the samples into the reference buffer
		CopyReferenceSamples(
			referenceBuffer, 
			reconBestU,
			cuX, 
			cuY, 
			reconUStride, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1);
		
		CopyDWordToDWordBuffer(
			(int *)transformBestU,
			transformStrideU >> 2,   // >> 2 because transformStrideU is in bytes!
			transformBufferDWord,
			CODING_UNIT_WIDTH >> 1,
			CODING_UNIT_WIDTH >> 1,
			CODING_UNIT_HEIGHT >> 1);
	
		Decode(
			transformBufferDWord, 
			reconBuffer, 
			referenceBuffer, 
			predictionMode, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			qpValue);
	
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
		// Copy the samples into the reference buffer
		CopyReferenceSamples(
			referenceBuffer, 
			reconBestV, 
			cuX, 
			cuY, 
			reconUStride, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1);
		
		CopyDWordToDWordBuffer(
			(int *)transformBestV,
			transformStrideV >> 2, // >> 2 because transformStrideV is in bytes, we need dword
			transformBufferDWord,
			CODING_UNIT_WIDTH >> 1,
			CODING_UNIT_WIDTH >> 1,
			CODING_UNIT_HEIGHT >> 1);
	
		Decode(
			transformBufferDWord, 
			reconBuffer, 
			referenceBuffer, 
			predictionMode, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			qpValue);
	
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


void Decode(
	CuIntBuffer transformBufferDWord,
	CuBuffer reconBuffer,
	unsigned char *referenceBuffer,
	int predictionMode, 
	int codingUnitWidth,
	int codingUnitHeight,
	int qp
	)
{
	CuIntBuffer invQuantizeBuffer;
	CuIntBuffer invTransformBufferDWord;
	CuIntBuffer reconBufferDWord;
	CuBuffer predictionBuffer;
	
	// Create prediction based off reference and predictionModeCursor
	PredictionFuncPtrTable[predictionMode](
		predictionBuffer, 
		codingUnitWidth, 
		referenceBuffer, 
		codingUnitWidth);

	/***** DECODE *****/
	// Copy transform coeffs into inv quant buffer
	CopyDWordToDWordBuffer(
		transformBufferDWord,
		codingUnitWidth,
		invQuantizeBuffer,
		codingUnitWidth,
		codingUnitWidth,
		codingUnitHeight);

	// Inverse Quantize
	QuantizationFuncPtrTable[QuantizeBackward](
		invQuantizeBuffer, 
		codingUnitWidth, 
		codingUnitWidth, 
		codingUnitHeight, 
		qp);

	// Inverse Transform
	xITrMxN(
		invQuantizeBuffer, 
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

	/***** DECODE *****/
	Decode(
		transformBufferDWord,
		reconBuffer,
		referenceBuffer,
		predictionMode,
		codingUnitWidth,
		codingUnitHeight,
		qp);
}

void EncodeCu(
	CodingUnitStructure_t *codingUnitStructure, 
	int cuX, 
	int cuY,
	int qpValue)
{
	// Encode Buffers
	CuIntBuffer transformBufferDWord[PredictionModeCount];

	// Decode Buffers (Mode Decision)
	CuBuffer reconBuffer[PredictionModeCount]; // Will be compared to Actual Picture

	int cuIndex = (cuY * codingUnitStructure->numCusWidth) + cuX;

	// pointer to the input Picture at the CU's location
	CodingUnit_t *codingUnit = &codingUnitStructure->codingUnits[cuIndex];


	// Unsigned Char Blocks
	BufferDescriptor_t *inputPicture = codingUnitStructure->inputPicture;
	unsigned char *inputY = &(inputPicture->yBuffer[codingUnit->yBufferOffset]);
	unsigned char *inputU = &(inputPicture->uBuffer[codingUnit->uvBufferOffset]);
	unsigned char *inputV = &(inputPicture->vBuffer[codingUnit->uvBufferOffset]);
	int yStride = inputPicture->yStride;
	int uStride = inputPicture->uStride;
	int vStride = inputPicture->vStride;

	// Transform Int Blocks (uses custom offset since it is in int space
	BufferDescriptor_t *transformBestBuffer = &codingUnitStructure->transformBestBuffer;
	int transformStrideY = transformBestBuffer->yStride;
	int transformStrideU = transformBestBuffer->uStride;
	int transformStrideV = transformBestBuffer->vStride;
	unsigned char *transformBestY = &(transformBestBuffer->yBuffer[(transformStrideY * cuY * CODING_UNIT_HEIGHT) + (cuX * CODING_UNIT_WIDTH * transformBestBuffer->sampleSize)]);
	unsigned char *transformBestU = &(transformBestBuffer->uBuffer[(transformStrideU * cuY * (CODING_UNIT_HEIGHT >> 1)) + (cuX * (CODING_UNIT_WIDTH >> 1) * transformBestBuffer->sampleSize)]);
	unsigned char *transformBestV = &(transformBestBuffer->vBuffer[(transformStrideV * cuY * (CODING_UNIT_HEIGHT >> 1)) + (cuX * (CODING_UNIT_WIDTH >> 1) * transformBestBuffer->sampleSize)]);
	
	// Units are in bytes
	int transformWidth = CODING_UNIT_WIDTH * transformBestBuffer->sampleSize;

	// Recon
	BufferDescriptor_t *reconBestBuffer = &codingUnitStructure->reconBestBuffer;
	unsigned char *reconBestY = &(reconBestBuffer->yBuffer[codingUnit->yBufferOffset]);
	unsigned char *reconBestU = &(reconBestBuffer->uBuffer[codingUnit->uvBufferOffset]);
	unsigned char *reconBestV = &(reconBestBuffer->vBuffer[codingUnit->uvBufferOffset]);
	int reconYStride = reconBestBuffer->yStride;
	int reconUStride = reconBestBuffer->uStride;
	int reconVStride = reconBestBuffer->vStride;

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

	// Prediction Mode Loop
	for(predictionModeCursor = 0; predictionModeCursor < PredictionModeCount; predictionModeCursor++)
	{
		int currentPredictionModeCost;

		///***** ENCODING/DECODING *****/
		EncodeDecode(
			transformBufferDWord[predictionModeCursor], 
			reconBuffer[predictionModeCursor], 
			inputY, 
			yStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH, 
			CODING_UNIT_HEIGHT,
			qpValue);

		/***** COST CALCULATION *****/
		// Determine the cost of this prediction mode, and update if necessary

		currentPredictionModeCost = ComputeCost(
										inputY, 
										yStride, 
										reconBuffer[predictionModeCursor], 
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
		}
	}

	// Copy over the best prediction mode for Y
	predictionModeCursor = codingUnitStructure->bestPredictionModes[cuIndex];
	{
		// Copy transform buffer into transform best buffer
		CopyDWordToDWordBuffer(
			transformBufferDWord[codingUnitStructure->bestPredictionModes[cuIndex]], 
			CODING_UNIT_WIDTH, 
			(int *)transformBestY, 
			transformStrideY >> 2,  // >> 2 because transformStrideY is in bytes!
			CODING_UNIT_WIDTH, 
			CODING_UNIT_HEIGHT);
	
		// Copy recon buffer into recon best buffer
		CopyBlockByte(
			reconBuffer[codingUnitStructure->bestPredictionModes[cuIndex]], 
			CODING_UNIT_WIDTH, 
			reconBestY, 
			reconYStride, 
			CODING_UNIT_WIDTH, 
			CODING_UNIT_HEIGHT);
	}

	// Encode U and V according to bestPredictionModes (determined from Y in previous loop)
	/***** U *****/
	{
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
			transformBufferDWord[predictionModeCursor], 
			reconBuffer[predictionModeCursor], 
			inputU, 
			uStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			qpValue);
	
		// Copy transform buffer into transform best buffer
		CopyDWordToDWordBuffer(
			transformBufferDWord[predictionModeCursor], 
			(CODING_UNIT_WIDTH >> 1), 
			(int *)transformBestU, 
			transformStrideU >> 2, // Stride is in Bytes, need dwords 
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));

		// Copy recon buffer into recon best buffer
		CopyBlockByte(
			reconBuffer[predictionModeCursor], 
			(CODING_UNIT_WIDTH >> 1), 
			reconBestU, 
			reconUStride, 
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));
	}
	
	/***** V *****/
	{
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
			transformBufferDWord[predictionModeCursor], 
			reconBuffer[predictionModeCursor], 
			inputV, 
			vStride, 
			referenceBuffer, 
			predictionModeCursor, 
			CODING_UNIT_WIDTH >> 1, 
			CODING_UNIT_HEIGHT >> 1,
			qpValue);
	
		// Copy transform buffer into transform best buffer
		CopyDWordToDWordBuffer(
			transformBufferDWord[predictionModeCursor], 
			(CODING_UNIT_WIDTH >> 1), 
			(int *)transformBestV, 
			transformStrideV >> 2, // >> 2 because stride is in Bytes, we need dword
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));

		// Copy recon buffer into recon best buffer
		CopyBlockByte(
			reconBuffer[predictionModeCursor], 
			(CODING_UNIT_WIDTH >> 1), 
			reconBestV, 
			reconVStride, 
			(CODING_UNIT_WIDTH >> 1), 
			(CODING_UNIT_HEIGHT >> 1));
	}
}


void DecodeLoop(
	CodingUnitStructure_t *codingUnitStructure
	)
{
	int cuCursorX;
	int cuCursorY;
	
	{
		int i;
		FILE *outputCoeffs;
		outputCoeffs = fopen("Z:\\EncodedFiles\\InputDecodeLoop.txt", "w");
	
		for(i = 0; i < (codingUnitStructure->transformBestBuffer.yuvSize >> 2); i++)
		{
			fprintf(outputCoeffs, "transformCoeffs[%d]: %d\n", i, ((int *)codingUnitStructure->transformBestBuffer.fullPicturePointer)[i]);
		}
	}

	// Loop through each CU in raster scan.
	// We can do it in this Raster-Scan order since we are single-threaded.
	// We could parallelize by doing this loop in Z-Scan order.
	for(cuCursorY = 0; cuCursorY < codingUnitStructure->numCusHeight; cuCursorY++)
	{
		for(cuCursorX = 0; cuCursorX < codingUnitStructure->numCusWidth; cuCursorX++)
		{
			DecodeCu(
				codingUnitStructure, 
				cuCursorX, 
				cuCursorY,
				codingUnitStructure->qp);

			printf("Finished Decoding %d/%d CU's!\n", cuCursorY*codingUnitStructure->numCusWidth + cuCursorX + 1, codingUnitStructure->numCusWidth * codingUnitStructure->numCusHeight);
		}
	}
}


void EncodeLoop(
	CodingUnitStructure_t *codingUnitStructure,
	int qpValue
	)
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
			EncodeCu(
				codingUnitStructure, 
				cuCursorX, 
				cuCursorY,
				qpValue);
			printf("Finished %d/%d CU's!\n", cuCursorY*codingUnitStructure->numCusWidth + cuCursorX + 1, codingUnitStructure->numCusWidth * codingUnitStructure->numCusHeight);
		}
	}

	// Update the QP value after encoding is finished
	codingUnitStructure->qp = qpValue;
}
// Requires EncodeLoop has run successfully on codingUnitStructure
void GenerateBitstream(
	CodingUnitStructure_t *codingUnitStructure,
	Bitstream_t *outputBitstream)
{
	int numCUs = codingUnitStructure->numCusHeight * codingUnitStructure->numCusWidth;
	int saturatedCoeffsSize = (codingUnitStructure->transformBestBuffer.yuvSize >> 1);
	short *saturatedCoeffs = (short *)malloc(sizeof(short) * saturatedCoeffsSize);
	
	{
		int bufferCursor;
		for(bufferCursor = 0; bufferCursor < (codingUnitStructure->transformBestBuffer.yuvSize >> 2); bufferCursor++)
		{
			saturatedCoeffs[bufferCursor] = ((int *)codingUnitStructure->transformBestBuffer.fullPicturePointer)[bufferCursor];
		}

	}

	EncodeBitstream(
		outputBitstream,
		(unsigned char *)saturatedCoeffs,//(unsigned char *) codingUnitStructure->transformBestBuffer.fullPicturePointer,
		saturatedCoeffsSize,//codingUnitStructure->transformBestBuffer.yuvSize,
		codingUnitStructure->bestPredictionModes,
		numCUs,
		codingUnitStructure->widthPicture,
		codingUnitStructure->heightPicture,
		codingUnitStructure->qp);

	free(saturatedCoeffs);
}