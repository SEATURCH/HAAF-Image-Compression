#include "../include/Bitstream.h"


void BitstreamConstructor(
	Bitstream_t *bitstream, 
	int maxWidth, 
	int maxHeight)
{
	const int LZ4HeaderLen = 7;
	int numCUs = ((maxWidth * maxHeight) / CODING_UNIT_WIDTH) / CODING_UNIT_HEIGHT;
	int maxSampleTotalSize = (((maxWidth) * (maxHeight) * 3) / 2) * sizeof(int);

	int bytesTakenByPredModes = numCUs >> 2;
	int bytesTakenByHeader = BITSTREAM_HEADER_LEN;
	int bytesNeededForLZ4 = maxSampleTotalSize + (2 * LZ4HeaderLen);

	bitstream->maxSize = bytesNeededForLZ4 + bytesTakenByHeader + bytesTakenByPredModes;
	bitstream->data = (unsigned char *) malloc(bitstream->maxSize * sizeof(unsigned char));
	bitstream->size = 0;
}

void BitstreamConstructorMaxSize(
	Bitstream_t *bitstream, 
	int maxSize)
{
	bitstream->maxSize = maxSize;
	bitstream->data = (unsigned char *) malloc(bitstream->maxSize * sizeof(unsigned char));
	bitstream->size = 0;
}


void BitstreamDeconstructor(Bitstream_t *bitstream)
{
	free(bitstream->data);
}

void GetPictureInfoFromHeader(
	Bitstream_t *bitstream,
	int *width,
	int *height,
	int *qp)
{
	*width = *((unsigned short *)(bitstream->data + HEADER_WIDTH_OFFSET));
	*height = *((unsigned short *)(bitstream->data + HEADER_HEIGHT_OFFSET));
	*qp = *((unsigned char *)(bitstream->data + HEADER_QP_OFFSET));

	//printf("Bitstream Picture Resolution: %dx%d\n", *width, *height);
}

void GetLZ4LengthFromHeader(
	Bitstream_t *bitstream,
	unsigned int *lz4Length)
{
	*lz4Length = *((unsigned int *)(bitstream->data + HEADER_LZ4_OFFSET));
}

void WriteBitstreamToFile(
	Bitstream_t *bitstream,
	const char *filename)
{
	FILE* file_handler;
	int bytesWritten;
	
	file_handler = fopen(filename,"wb");
	if(file_handler == NULL)
	{
		printf("Could not open file for output!!\n");
		return;
	}

	// No data in bitstream?
	if(bitstream->size == 0)
	{
		printf("Warning: empty bitstream!\n");
	}
		
	// Write Bitstream to file
	bytesWritten = fwrite(bitstream->data, 1, bitstream->size, file_handler);

	if(bytesWritten != bitstream->size)
	{
		printf("error, size written does not equal bitstream size!\n");
	}

	//Write the quantized output back to the PI via a serial stream
	fclose(file_handler);
}

void OpenBitstreamFromFile(
	const char *filename,
	Bitstream_t *inputBitstream,
	int *pictureWidth,
	int *pictureHeight,
	int *qp)
{
	FILE *bitstreamFile = fopen(filename, "rb");
	int fileSize;

	if(bitstreamFile == NULL)
	{
		printf("Couldn't open bitstream!\n");
	}

	// Get total file size

	// Iterate cursor to end of file
	fseek(bitstreamFile, 0, SEEK_END);
	// Get cursor position
	fileSize = ftell(bitstreamFile);

	// Put bitstreamFile cursor back to beginning
	fseek(bitstreamFile, 0, SEEK_SET);

	// Construct the bitstream based off the filesize
	BitstreamConstructorMaxSize(
		inputBitstream, 
		fileSize);

	// Read the file into inputBitstream
	fread(inputBitstream->data, 1, fileSize, bitstreamFile);
	inputBitstream->size = fileSize;

	// Get picture width/ picture height
	GetPictureInfoFromHeader(
		inputBitstream, 
		pictureWidth, 
		pictureHeight,
		qp);
}



void DecodeBitstream(
	CodingUnitStructure_t *codingUnitStructure,
	Bitstream_t *inputBitstream)
{
	unsigned char *headerPtr = inputBitstream->data;
	unsigned char *encodedPMPtr = &(headerPtr[BITSTREAM_HEADER_LEN]);
	unsigned char *lz4Ptr; // Determined after encoded predictionModesLen is found

	unsigned int lz4Length;
	int numPredictionModes;
	int numCUs = codingUnitStructure->numCusHeight * codingUnitStructure->numCusWidth;

	// Short buffer holds our coefficients
	int saturatedCoeffsSize = (codingUnitStructure->transformBestBuffer.yuvSize >> 1);
	short *saturatedCoeffs = (short *)malloc(sizeof(short) * saturatedCoeffsSize);
	
	// Decode Prediction Modes 
	DecodePredictionModes(
		codingUnitStructure->bestPredictionModes,
		&numPredictionModes,
		encodedPMPtr,
		numCUs >> NUMBITSPERMODE, 
		PredictionModeCount);

	// Decode LZ4
	lz4Ptr = &(encodedPMPtr[numCUs >> NUMBITSPERMODE]);
	GetLZ4LengthFromHeader(
		inputBitstream, 
		&lz4Length);

	// Decompress LZ4 into transform coefficients
	LZ4IO_decompressArray(
		lz4Ptr, 
		lz4Length,
		(unsigned char *) saturatedCoeffs,
		saturatedCoeffsSize);

	// Saturate short coeffs to int
	{
		int bufferCursor;
		for(bufferCursor = 0; bufferCursor < (codingUnitStructure->transformBestBuffer.yuvSize >> 2); bufferCursor++)
		{
			((int *)codingUnitStructure->transformBestBuffer.fullPicturePointer)[bufferCursor] = saturatedCoeffs[bufferCursor];
		}

	}


	free(saturatedCoeffs);
}


void EncodePredictionModes(
	// OUT
	unsigned char *outputBuffer,
	int *outputBufferLength,
	// IN
	PredictionMode_t *predictionModes,
	int predictionModesLen,
	int numPredictionModes)
{
	// This function was designed for maximum 4 prediction modes
	int outputBufferCursor = 0;
	int predictionModeCursor = 0;
	int innerCursor = 0;

	if(numPredictionModes > MAXPREDMODES)
	{
		printf("FATAL ERROR: Number of Prediction Modes exceeds max!\n");
		return;
	}

	// How this works:
	// Since each prediction mode can be encoded with 2 bits, 
	// we can store four prediction modes into 1 byte.

	// Example (Pn: Prediction Mode 'n'):
	// P0: 0b00000000
	// P1: 0b00000001
	// P2: 0b00000010
	// P3: 0b00000011
	//
	// After Shifting: (PSn: Prediction Mode Pn Shifted Left by n*2):
	// PS0: 0b00000000
	// PS1: 0b00000100
	// PS2: 0b00100000
	// PS3: 0b11000000
	//
	// Compressed (C: Sum of PSn from [0:3]):
	// C:  0b11100100
	for(predictionModeCursor = 0; predictionModeCursor + (MAXPREDMODES - 1) < predictionModesLen; predictionModeCursor += MAXPREDMODES)
	{
		outputBufferCursor = predictionModeCursor >> NUMBITSPERMODE;
		outputBuffer[outputBufferCursor] = 0;
		for(innerCursor = 0; innerCursor < MAXPREDMODES; innerCursor++)
		{
			// Obtain the first two bits and shift it left into position
			outputBuffer[outputBufferCursor] |= ((predictionModes[predictionModeCursor + innerCursor] & 0x3) << (innerCursor << 1));
			//printf("Best Prediction Mode [%d]: 0x%x\n", predictionModeCursor + innerCursor, predictionModes[predictionModeCursor + innerCursor]);
		}
		//printf("outputBuffer[%d] : 0x%x\n", outputBufferCursor, outputBuffer[outputBufferCursor]);
	}
	// Inherent to 2 bits per prediction mode
	*outputBufferLength = predictionModesLen >> 2;
}

void DecodePredictionModes(
	// OUT
	PredictionMode_t *predictionModes,
	int *predictionModesLen,
	// IN
	unsigned char *inputBuffer,
	int inputBufferLen,
	int numPredictionModes)
{
	int inputBufferCursor = 0;
	int predictionModeCursor = 0;
	int innerCursor = 0;

	// Performs an inverse of EncodePredictionModes
	// Compressed (C: Sum of PSn from [0:3]):
	// C:   0b11100100
	//
	// Copied, Shifted (CSn: C >> (n*2)):
	// CI0: 0b11100100
	// CI1: 0b00111001
	// CI2: 0b00001110
	// CI3: 0b00000011
	//
	// Prediction Mode: (Pn: (CSn & 0x3)):
	// P0: 0b00000000
	// P1: 0b00000001
	// P2: 0b00000010
	// P3: 0b00000011
	
	for(inputBufferCursor = 0; inputBufferCursor < inputBufferLen; inputBufferCursor++)
	{
		predictionModeCursor = inputBufferCursor * MAXPREDMODES;
		for(innerCursor = 0; innerCursor < MAXPREDMODES; innerCursor++)
		{
			predictionModes[predictionModeCursor + innerCursor] = (PredictionMode_t)((inputBuffer[inputBufferCursor] >> (innerCursor << 1)) & 0x3);
			//printf("Decoded Prediction Mode [%d]: 0x%x\n", predictionModeCursor + innerCursor, predictionModes[predictionModeCursor + innerCursor]);
		}
	}

	*predictionModesLen = inputBufferLen * MAXPREDMODES;
}


// Header Component Descriptions:
// Width (W): 16 bits, describes width of picture
// Height (H): 16 bits, describes height of picture
// Block Width (BW): 8 bits, describes width of blocks used to encode
// Block Height (BH): 8 bits, describes height of blocks used to encode
// Quantization Parameter: 8 bits, describes the quantization level of every single CU
// Transform Coefficients Length (TCL): 32 bits, describes total size taken up by encoded LZ4 transform coeffs

// Header (in order, 1 byte per line):
// - bn: byte n
// - Header Len: 10 bytes
// START:
// W.b1
// W.b0
// H.b1
// H.b0
// BW
// BH
// QP
// TCL.b3
// TCL.b2
// TCL.b1
// TCL.b0
// HEADER END
void WriteHeaderInfo(
	unsigned char *outputBuffer, // Assume this has enough memory
	int *bytesWritten, // Keep track of all the bytes written by the fn
	unsigned short width,
	unsigned short height,
	unsigned char blockWidth,
	unsigned char blockHeight,
	unsigned char qp,
	unsigned int transformCoeffLen)
{
	unsigned char *outputBufferPtr = outputBuffer;

	// Initialize bytesWritten to 0
	*bytesWritten = 0;

	// Write width to bytes 1, 0
	*((unsigned short *)outputBufferPtr) = width;
	outputBufferPtr += sizeof(unsigned short);
	*bytesWritten += sizeof(unsigned short);
	
	// Write height to bytes 3, 2
	*((unsigned short *)outputBufferPtr) = height;
	outputBufferPtr += sizeof(unsigned short);
	*bytesWritten += sizeof(unsigned short);

	// Write BlockWidth to bytes 4
	*outputBufferPtr = blockWidth;
	outputBufferPtr += sizeof(unsigned char);
	*bytesWritten += sizeof(unsigned char);
	
	// Write BlockHeight to bytes 5
	*outputBufferPtr = blockHeight;
	outputBufferPtr += sizeof(unsigned char);
	*bytesWritten += sizeof(unsigned char);

	// Write QP to bytes 6
	*outputBufferPtr = qp;
	outputBufferPtr += sizeof(unsigned char);
	*bytesWritten += sizeof(unsigned char);

	// Write transformCoeffLen to bytes 10, 9, 8, 7
	*((unsigned int*)outputBufferPtr) = transformCoeffLen;
	outputBufferPtr += sizeof(unsigned int);
	*bytesWritten += sizeof(unsigned int);

	if(*bytesWritten != BITSTREAM_HEADER_LEN)
	{
		printf("WARNING! HEADER BYTES WRITTEN DO NOT MATCH HEADER LENGTH SPEC: %d, %d!\n", *bytesWritten, BITSTREAM_HEADER_LEN);
	}
}



// BITSTREAM SPEC
// This is a rough description of how the output bitstream will look.

// **HEADER** <fixedlen>
// **PREDICTION MODES** <fixedlen, based off header components>
// **TRANSFORM COEFFS** <dynamiclen, impossible to predict>

// ENCODE BITSTREAM
void EncodeBitstream(
	// OUTPUT
	Bitstream_t *outputBitstream,
	// IN
	unsigned char *transformCoeffs,
	int transformCoeffsSize, // Total size of transformCoeffs (remember, they are ints, not chars)
	PredictionMode_t *predictionModes,
	int predictionModesLen,
	int width,
	int height,
	int qp)
{
	unsigned char *headerPtr = outputBitstream->data;
	unsigned char *encodedPMPtr = &(headerPtr[BITSTREAM_HEADER_LEN]);
	unsigned char *lz4Ptr; // Determined after encoded predictionModesLen is found

	int encodedHeaderSize = 0;
	int encodedPMSize = 0;
	int encodedLZ4Size;

	int compressionLevel = LZ4_COMPRESSION_LEVEL;

	// Encode Prediction Modes
	EncodePredictionModes(
		encodedPMPtr,
		&encodedPMSize,
		predictionModes,
		predictionModesLen,
		PredictionModeCount// CONST
		);

	// Determine lz4Ptr based off encodedPMLen
	lz4Ptr = &(encodedPMPtr[encodedPMSize]);
	encodedLZ4Size = outputBitstream->maxSize - encodedPMSize - BITSTREAM_HEADER_LEN;
	
	// Encode transformCoeffs into lz4Ptr
	LZ4IO_compressArray(
		transformCoeffs, 
		transformCoeffsSize,
		lz4Ptr,
		&encodedLZ4Size,
		compressionLevel
		);

	// With encodedLZ4 len now available, write header
	WriteHeaderInfo(
		headerPtr,
		&encodedHeaderSize,
		(unsigned short) width,
		(unsigned short) height,
		(unsigned char) CODING_UNIT_WIDTH,
		(unsigned char) CODING_UNIT_HEIGHT,
		(unsigned char) qp,
		(unsigned int) encodedLZ4Size);


	// Write total length
	outputBitstream->size = encodedHeaderSize + encodedPMSize + encodedLZ4Size;
}
