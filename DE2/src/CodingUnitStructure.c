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
	int pictureHeight,
	int qp)
{
	int xCuIndex = 0;
	int yCuIndex = 0;
	int cuIndex = 0;

	/** Picture Buffers **/
	// Input Picture Set-up
	codingUnitStructure->widthPicture = pictureWidth;
	codingUnitStructure->heightPicture = pictureHeight;

	// Initialize Picture Buffers
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

	// Initialize Quantization Tables based off qp
	{
		int BandPassFilter[CODING_UNIT_WIDTH + CODING_UNIT_HEIGHT - 1];

		int yCursor = 0;
		int xCursor = 0;

		// Initialize Band Pass Filter
		BandPassFilter[0]  = GetQuantizationParam(AzA_MAX, AzA_MIN, QP_MAX, qp);
		BandPassFilter[1]  = GetQuantizationParam(BzB_MAX, BzB_MIN, QP_MAX, qp);
		BandPassFilter[2]  = GetQuantizationParam(CzC_MAX, CzC_MIN, QP_MAX, qp);
		BandPassFilter[3]  = GetQuantizationParam(DzD_MAX, DzD_MIN, QP_MAX, qp);
		BandPassFilter[4]  = GetQuantizationParam(EzE_MAX, EzE_MIN, QP_MAX, qp);
		BandPassFilter[5]  = GetQuantizationParam(FzF_MAX, FzF_MIN, QP_MAX, qp);
		BandPassFilter[6]  = GetQuantizationParam(GzG_MAX, GzG_MIN, QP_MAX, qp);
		BandPassFilter[7]  = GetQuantizationParam(HzH_MAX, HzH_MIN, QP_MAX, qp);
		BandPassFilter[8]  = GetQuantizationParam(IzI_MAX, IzI_MIN, QP_MAX, qp);
		BandPassFilter[9]  = GetQuantizationParam(JzJ_MAX, JzJ_MIN, QP_MAX, qp);
		BandPassFilter[10] = GetQuantizationParam(KzK_MAX, KzK_MIN, QP_MAX, qp);
		BandPassFilter[11] = GetQuantizationParam(LzL_MAX, LzL_MIN, QP_MAX, qp);
		BandPassFilter[12] = GetQuantizationParam(MzM_MAX, MzM_MIN, QP_MAX, qp);
		BandPassFilter[13] = GetQuantizationParam(NzN_MAX, NzN_MIN, QP_MAX, qp);
		BandPassFilter[14] = GetQuantizationParam(OzO_MAX, OzO_MIN, QP_MAX, qp);
		BandPassFilter[15] = GetQuantizationParam(PzP_MAX, PzP_MIN, QP_MAX, qp);
		BandPassFilter[16] = GetQuantizationParam(QzQ_MAX, QzQ_MIN, QP_MAX, qp);
		BandPassFilter[17] = GetQuantizationParam(RzR_MAX, RzR_MIN, QP_MAX, qp);
		BandPassFilter[18] = GetQuantizationParam(SzS_MAX, SzS_MIN, QP_MAX, qp);
		BandPassFilter[19] = GetQuantizationParam(TzT_MAX, TzT_MIN, QP_MAX, qp);
		BandPassFilter[20] = GetQuantizationParam(UzU_MAX, UzU_MIN, QP_MAX, qp);
		BandPassFilter[21] = GetQuantizationParam(VzV_MAX, VzV_MIN, QP_MAX, qp);
		BandPassFilter[22] = GetQuantizationParam(WzW_MAX, WzW_MIN, QP_MAX, qp);
		BandPassFilter[23] = GetQuantizationParam(XzX_MAX, XzX_MIN, QP_MAX, qp);
		BandPassFilter[24] = GetQuantizationParam(YzY_MAX, YzY_MIN, QP_MAX, qp);
		BandPassFilter[25] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);
		BandPassFilter[26] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);
		BandPassFilter[27] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);
		BandPassFilter[28] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);
		BandPassFilter[29] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);
		BandPassFilter[30] = GetQuantizationParam(ZzZ_MAX, ZzZ_MIN, QP_MAX, qp);

		// Luma Filter
		for(yCursor = 0; yCursor < CODING_UNIT_HEIGHT; yCursor++)
		{
			for(xCursor = 0; xCursor < CODING_UNIT_WIDTH; xCursor++)
			{
				codingUnitStructure->lumaQuantizationBuffer[(yCursor * CODING_UNIT_WIDTH) + xCursor] = BandPassFilter[yCursor + xCursor];
			}
		}

		// Chroma Filter
		for(yCursor = 0; yCursor < (CODING_UNIT_HEIGHT >> 1); yCursor++)
		{
			for(xCursor = 0; xCursor < (CODING_UNIT_WIDTH >> 1); xCursor++)
			{
				codingUnitStructure->chromaQuantizationBuffer[(yCursor * (CODING_UNIT_WIDTH >> 1)) + xCursor] = BandPassFilter[(yCursor << 1) + (xCursor << 1)];
			}
		}

		codingUnitStructure->qp = qp;

	}
	
}

void CodingUnitStructureDeconstructor(CodingUnitStructure_t *codingUnitStructure)
{
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
