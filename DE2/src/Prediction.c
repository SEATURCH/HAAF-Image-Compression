#include "../include/Prediction.h"

/*** PREDICTION ***/

// Creates an Average of the reference and assigns it to every sample in dst
// - Based off H264 16x16 DC prediction mode
void PredictionModeDC(
	// OUT
	unsigned char *dst, 
	int dstStride, 
	// IN
	unsigned char *referenceBuffer, 
	int size)
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
// - Based off H264 16x16 Vertical prediction mode
void PredictionModeVertical(
	// OUT
	unsigned char *dst, 
	int dstStride, 
	// IN
	unsigned char *referenceBuffer, 
	int size)
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
// - Based off H264 16x16 Horizontal prediction mode
void PredictionModeHorizontal(
	// OUT
	unsigned char *dst, 
	int dstStride, 
	// IN
	unsigned char *referenceBuffer, 
	int size)
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
// - Based off H264 16x16 Horizontal prediction mode
// Obtained from:
// http://wiki.multimedia.cx/index.php?title=H.264_Prediction#16x16_Prediction_Modes
void PredictionModePlanar(
	// OUT
	unsigned char *dst, 
	int dstStride, 
	// IN
	unsigned char *referenceBuffer, 
	int size)
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

// Calculates the reconstructed picture based on the residual and prediction
void CalculateRecon(
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

// Calculates the int reconstructed picture based on the residual and prediction
void CalculateRecon32Bit(
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

