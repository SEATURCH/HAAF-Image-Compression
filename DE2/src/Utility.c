#include "Utility.h"


int Clip3(const int minVal, const int maxVal, const int a)
{
	return MIN(MAX(minVal, a), maxVal);
}

// Prints a block of samples
void PrintBlock(unsigned char *inputBlock, int inputStride, int width, int height)
{
	int i, j;
	int inputCursor;

	//printf("Picture %dx%d:\n", width, height);
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			inputCursor = (j * inputStride) + i;
			printf("%3.1d ", inputBlock[inputCursor]);
		}
		printf("\n");
	}
	printf("\n");
}


// Prints a block of samples
void FindMaxMinInt(int *inputBlock, int inputStride, int width, int height, int *max, int *min)
{
	int i, j;
	int inputCursor;

	*max = 0x80000000;
	*min = 0x7FFFFFFF;
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			inputCursor = (j * inputStride) + i;

			if(inputBlock[inputCursor] > *max)
				*max = inputBlock[inputCursor];

			if(inputBlock[inputCursor] < *min)
				*min = inputBlock[inputCursor];
		}
	}
}

// Prints a block of samples
void PrintBlockSubtraction(unsigned char *leftBlock, int leftStride, unsigned char *rightBlock, int rightStride, int width, int height)
{
	int i, j;
	int leftCursor, rightCursor;

	//printf("Picture %dx%d:\n", width, height);
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			leftCursor = (j * leftStride) + i;
			rightCursor = (j * rightStride) + i;
			printf("%3.1d ", ((int)leftBlock[leftCursor]) - ((int)rightBlock[rightCursor]));
		}
		printf("\n");
	}
	printf("\n");
}

// Prints an int block of samples
void PrintBlockInt(int *inputBlock, int inputStride, int width, int height)
{
	int i, j;
	int inputCursor;

	//printf("Picture %dx%d:\n", width, height);
	for(j = 0; j < height; j++)
	{
		for(i = 0; i < width; i++)
		{
			inputCursor = (j * inputStride) + i;
			printf("%3.1d ", inputBlock[inputCursor]);
		}
		printf("\n");
	}
	printf("\n");
}

// Prints the reference buffer in a formatted way
void PrintReferenceBuffer(unsigned char *referenceBufferY, int cuX, int cuY, int size)
{
	//printf("Cu Index: (%d, %d)\n", cuX, cuY);
	// Print Reference Buffer
	{
		int i;
		for(i = 0; i < size + 1; i++)
		{
			printf("%d ", referenceBufferY[i]);
		}
		printf("\n");
		for(i = size + 1; i < CODING_UNIT_REF_BUFFER_SIZE_Y; i++)
		{
			printf("%d\n", referenceBufferY[i]);
		}

	}
}

// Self-explanatory
void SetInputPictureSamplesToValue(unsigned char *inputPicture, int width, int height, int value)
{
	int widthCursor;
	int heightCursor;

	for(heightCursor = 0; heightCursor < height; heightCursor++)
	{
		for(widthCursor = 0; widthCursor < width; widthCursor++)
		{
			inputPicture[(heightCursor * width) + widthCursor] = value;

		}
	}
}


// Sets the picture based on CU blocks to an slightly arbitrary number. 
// Each number symbolizes a 16x16 block
// 0 1 2 3 4 5 6 ....
// 1 2 3 4 5 6 7
// 2 3 4 5 6 7 8
void SetInputPictureSamplesToArbitrary(unsigned char *inputPicture, int width, int height)
{
	int widthCursor;
	int heightCursor;


	int pixelValue = 0;

	for(heightCursor = 0; heightCursor < height; heightCursor++)
	{
		for(widthCursor = 0; widthCursor < width; widthCursor++)
		{
			inputPicture[(heightCursor * width) + widthCursor] = (widthCursor / 16) + (heightCursor / 16); 

			pixelValue++;
			if(pixelValue > 92)
			{
				pixelValue = 0;
			}
		}
	}
	//PrintBlock(inputPicture, width, height);
}

