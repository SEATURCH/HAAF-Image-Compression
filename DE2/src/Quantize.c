#include "../include/Quantize.h"

void Quantize(
	int *coeff, 
	int coeffStride, 
	int width, 
	int height,
	int *qp
	)
{
	int puCursorX;
	int puCursorY;

	// Lossless Compression
	if(qp == NULL)
		return;

	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] /= qp[puCursorY * width + puCursorX];

		}
	}
}


void InverseQuantize(
	int *coeff, 
	int coeffStride, 
	int width, 
	int height, 
	int *qp
	)
{
	int puCursorX;
	int puCursorY;
	
	// Lossless Compression
	if(qp == NULL)
		return;

	// Set everything to average
	for(puCursorY = 0; puCursorY < height; puCursorY++)
	{
		for(puCursorX = 0; puCursorX < width; puCursorX++)
		{
			// The '1' is the offset from the (-1, -1) start of referenceBuffer
			coeff[(puCursorY * coeffStride) + puCursorX] *= qp[puCursorY * width + puCursorX];
		}
	}
}

