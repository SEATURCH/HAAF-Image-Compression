/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2015, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * intERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

// THIS CODE IS BORROWED FROM THE http://hevc.kw.bbc.co.uk/svn/jctvc-a124/trunk/source/Lib/TLibCommon/TComTrQuant.cpp FILE FROM THE HM H265 CODEC


#include "../include/Transform.h"
#include <string.h>

void InitConvertToBit(char *g_aucConvertToBit)
{
  int i, c;

  // g_aucConvertToBit[ x ]: log2(x/4), if x=4 -> 0, x=8 -> 1, x=16 -> 2, ...
  memset( g_aucConvertToBit,   -1, sizeof( g_aucConvertToBit ) );
  c=0;
  for ( i=4; i<=MAX_CU_SIZE; i*=2 )
  {
    g_aucConvertToBit[ i ] = c;
    c++;
  }
}

const short g_aiT8 [TRANSFORM_NUMBER_OF_DIRECTIONS][8][8]   =
{
  DEFINE_DCT8x8_MATRIX  (   64,    83,    36,    89,    75,    50,    18),
  DEFINE_DCT8x8_MATRIX  (   64,    83,    36,    89,    75,    50,    18)
};


const short g_aiT16[TRANSFORM_NUMBER_OF_DIRECTIONS][16][16] =
{
  DEFINE_DCT16x16_MATRIX(   64,    83,    36,    89,    75,    50,    18,    90,    87,    80,    70,    57,    43,    25,     9),
  DEFINE_DCT16x16_MATRIX(   64,    83,    36,    89,    75,    50,    18,    90,    87,    80,    70,    57,    43,    25,     9)
};




/** 8x8 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 *  \param line
 */
void partialButterfly8(int *src, int *dst, int shift, int line)
{
  int j,k;
  int E[4],O[4];
  int EE[2],EO[2];
  int add = (shift > 0) ? (1<<(shift-1)) : 0;

  for (j=0; j<line; j++)
  {
    /* E and O*/
    for (k=0;k<4;k++)
    {
      E[k] = src[k] + src[7-k];
      O[k] = src[k] - src[7-k];
    }
    /* EE and EO */
    EE[0] = E[0] + E[3];
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    dst[0]      = (g_aiT8[TRANSFORM_FORWARD][0][0]*EE[0] + g_aiT8[TRANSFORM_FORWARD][0][1]*EE[1] + add)>>shift;
    dst[4*line] = (g_aiT8[TRANSFORM_FORWARD][4][0]*EE[0] + g_aiT8[TRANSFORM_FORWARD][4][1]*EE[1] + add)>>shift;
    dst[2*line] = (g_aiT8[TRANSFORM_FORWARD][2][0]*EO[0] + g_aiT8[TRANSFORM_FORWARD][2][1]*EO[1] + add)>>shift;
    dst[6*line] = (g_aiT8[TRANSFORM_FORWARD][6][0]*EO[0] + g_aiT8[TRANSFORM_FORWARD][6][1]*EO[1] + add)>>shift;

    dst[line]   = (g_aiT8[TRANSFORM_FORWARD][1][0]*O[0] + g_aiT8[TRANSFORM_FORWARD][1][1]*O[1] + g_aiT8[TRANSFORM_FORWARD][1][2]*O[2] + g_aiT8[TRANSFORM_FORWARD][1][3]*O[3] + add)>>shift;
    dst[3*line] = (g_aiT8[TRANSFORM_FORWARD][3][0]*O[0] + g_aiT8[TRANSFORM_FORWARD][3][1]*O[1] + g_aiT8[TRANSFORM_FORWARD][3][2]*O[2] + g_aiT8[TRANSFORM_FORWARD][3][3]*O[3] + add)>>shift;
    dst[5*line] = (g_aiT8[TRANSFORM_FORWARD][5][0]*O[0] + g_aiT8[TRANSFORM_FORWARD][5][1]*O[1] + g_aiT8[TRANSFORM_FORWARD][5][2]*O[2] + g_aiT8[TRANSFORM_FORWARD][5][3]*O[3] + add)>>shift;
    dst[7*line] = (g_aiT8[TRANSFORM_FORWARD][7][0]*O[0] + g_aiT8[TRANSFORM_FORWARD][7][1]*O[1] + g_aiT8[TRANSFORM_FORWARD][7][2]*O[2] + g_aiT8[TRANSFORM_FORWARD][7][3]*O[3] + add)>>shift;

    src += 8;
    dst ++;
  }
}

/** 8x8 inverse transform implemented using partial butterfly structure (1D)
 *  \param src   input data (transform coefficients)
 *  \param dst   output data (residual)
 *  \param shift specifies right shift after 1D transform
 *  \param line
 *  \param outputMinimum  minimum for clipping
 *  \param outputMaximum  maximum for clipping
 */
void partialButterflyInverse8(int *src, int *dst, int shift, int line, const int outputMinimum, const int outputMaximum)
{
  int j,k;
  int E[4],O[4];
  int EE[2],EO[2];
  int add = (shift > 0) ? (1<<(shift-1)) : 0;

  for (j=0; j<line; j++)
  {
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<4;k++)
    {
      O[k] = g_aiT8[TRANSFORM_INVERSE][ 1][k]*src[line]   + g_aiT8[TRANSFORM_INVERSE][ 3][k]*src[3*line] +
             g_aiT8[TRANSFORM_INVERSE][ 5][k]*src[5*line] + g_aiT8[TRANSFORM_INVERSE][ 7][k]*src[7*line];
    }

    EO[0] = g_aiT8[TRANSFORM_INVERSE][2][0]*src[ 2*line ] + g_aiT8[TRANSFORM_INVERSE][6][0]*src[ 6*line ];
    EO[1] = g_aiT8[TRANSFORM_INVERSE][2][1]*src[ 2*line ] + g_aiT8[TRANSFORM_INVERSE][6][1]*src[ 6*line ];
    EE[0] = g_aiT8[TRANSFORM_INVERSE][0][0]*src[ 0      ] + g_aiT8[TRANSFORM_INVERSE][4][0]*src[ 4*line ];
    EE[1] = g_aiT8[TRANSFORM_INVERSE][0][1]*src[ 0      ] + g_aiT8[TRANSFORM_INVERSE][4][1]*src[ 4*line ];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      dst[ k   ] = Clip3( outputMinimum, outputMaximum, (E[k] + O[k] + add)>>shift );
      dst[ k+4 ] = Clip3( outputMinimum, outputMaximum, (E[3-k] - O[3-k] + add)>>shift );
    }
    src ++;
    dst += 8;
  }
}



/** 16x16 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 *  \param line
 */
void partialButterfly16(int *src, int *dst, int shift, int line)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = (shift > 0) ? (1<<(shift-1)) : 0;

  for (j=0; j<line; j++)
  {
    /* E and O*/
    for (k=0;k<8;k++)
    {
      E[k] = src[k] + src[15-k];
      O[k] = src[k] - src[15-k];
    }
    /* EE and EO */
    for (k=0;k<4;k++)
    {
      EE[k] = E[k] + E[7-k];
      EO[k] = E[k] - E[7-k];
    }
    /* EEE and EEO */
    EEE[0] = EE[0] + EE[3];
    EEO[0] = EE[0] - EE[3];
    EEE[1] = EE[1] + EE[2];
    EEO[1] = EE[1] - EE[2];

    dst[ 0      ] = (g_aiT16[TRANSFORM_FORWARD][ 0][0]*EEE[0] + g_aiT16[TRANSFORM_FORWARD][ 0][1]*EEE[1] + add)>>shift;
    dst[ 8*line ] = (g_aiT16[TRANSFORM_FORWARD][ 8][0]*EEE[0] + g_aiT16[TRANSFORM_FORWARD][ 8][1]*EEE[1] + add)>>shift;
    dst[ 4*line ] = (g_aiT16[TRANSFORM_FORWARD][ 4][0]*EEO[0] + g_aiT16[TRANSFORM_FORWARD][ 4][1]*EEO[1] + add)>>shift;
    dst[ 12*line] = (g_aiT16[TRANSFORM_FORWARD][12][0]*EEO[0] + g_aiT16[TRANSFORM_FORWARD][12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      dst[ k*line ] = (g_aiT16[TRANSFORM_FORWARD][k][0]*EO[0] + g_aiT16[TRANSFORM_FORWARD][k][1]*EO[1] +
                       g_aiT16[TRANSFORM_FORWARD][k][2]*EO[2] + g_aiT16[TRANSFORM_FORWARD][k][3]*EO[3] + add)>>shift;
    }

    for (k=1;k<16;k+=2)
    {
      dst[ k*line ] = (g_aiT16[TRANSFORM_FORWARD][k][0]*O[0] + g_aiT16[TRANSFORM_FORWARD][k][1]*O[1] +
                       g_aiT16[TRANSFORM_FORWARD][k][2]*O[2] + g_aiT16[TRANSFORM_FORWARD][k][3]*O[3] +
                       g_aiT16[TRANSFORM_FORWARD][k][4]*O[4] + g_aiT16[TRANSFORM_FORWARD][k][5]*O[5] +
                       g_aiT16[TRANSFORM_FORWARD][k][6]*O[6] + g_aiT16[TRANSFORM_FORWARD][k][7]*O[7] + add)>>shift;
    }

    src += 16;
    dst ++;

  }
}

/** 16x16 inverse transform implemented using partial butterfly structure (1D)
 *  \param src            input data (transform coefficients)
 *  \param dst            output data (residual)
 *  \param shift          specifies right shift after 1D transform
 *  \param line
 *  \param outputMinimum  minimum for clipping
 *  \param outputMaximum  maximum for clipping
 */
void partialButterflyInverse16(int *src, int *dst, int shift, int line, const int outputMinimum, const int outputMaximum)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = (shift > 0) ? (1<<(shift-1)) : 0;

  for (j=0; j<line; j++)
  {
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<8;k++)
    {
      O[k] = g_aiT16[TRANSFORM_INVERSE][ 1][k]*src[ line]   + g_aiT16[TRANSFORM_INVERSE][ 3][k]*src[ 3*line] +
             g_aiT16[TRANSFORM_INVERSE][ 5][k]*src[ 5*line] + g_aiT16[TRANSFORM_INVERSE][ 7][k]*src[ 7*line] +
             g_aiT16[TRANSFORM_INVERSE][ 9][k]*src[ 9*line] + g_aiT16[TRANSFORM_INVERSE][11][k]*src[11*line] +
             g_aiT16[TRANSFORM_INVERSE][13][k]*src[13*line] + g_aiT16[TRANSFORM_INVERSE][15][k]*src[15*line];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[TRANSFORM_INVERSE][ 2][k]*src[ 2*line] + g_aiT16[TRANSFORM_INVERSE][ 6][k]*src[ 6*line] +
              g_aiT16[TRANSFORM_INVERSE][10][k]*src[10*line] + g_aiT16[TRANSFORM_INVERSE][14][k]*src[14*line];
    }
    EEO[0] = g_aiT16[TRANSFORM_INVERSE][4][0]*src[ 4*line ] + g_aiT16[TRANSFORM_INVERSE][12][0]*src[ 12*line ];
    EEE[0] = g_aiT16[TRANSFORM_INVERSE][0][0]*src[ 0      ] + g_aiT16[TRANSFORM_INVERSE][ 8][0]*src[ 8*line  ];
    EEO[1] = g_aiT16[TRANSFORM_INVERSE][4][1]*src[ 4*line ] + g_aiT16[TRANSFORM_INVERSE][12][1]*src[ 12*line ];
    EEE[1] = g_aiT16[TRANSFORM_INVERSE][0][1]*src[ 0      ] + g_aiT16[TRANSFORM_INVERSE][ 8][1]*src[ 8*line  ];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    for (k=0;k<2;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+2] = EEE[1-k] - EEO[1-k];
    }
    for (k=0;k<4;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+4] = EE[3-k] - EO[3-k];
    }
    for (k=0;k<8;k++)
    {
      dst[k]   = Clip3( outputMinimum, outputMaximum, (E[k] + O[k] + add)>>shift);
      dst[k+8] = Clip3( outputMinimum, outputMaximum, (E[7-k] - O[7-k] + add)>>shift);

    }
    src ++;
    dst += 16;
  }
}

#define DYNAMIC_RANGE	15

/** MxN forward transform (2D)
*  \param bitDepth              [in]  bit depth
*  \param block                 [in]  residual block
*  \param coeff                 [out] transform coefficients
*  \param iWidth                [in]  width of transform
*  \param iHeight               [in]  height of transform
*  \param useDST                [in]

*/
void xTrMxN(int *block, int *coeff, int iWidth, int iHeight)
{
  char g_aucConvertToBit  [ MAX_CU_SIZE+1 ];


  const int TRANSFORM_MATRIX_SHIFT = 6;
  const int bitDepth = 8;
  const int maxLog2TrDynamicRange = DYNAMIC_RANGE;

  int shift_1st;// = ((g_aucConvertToBit[iWidth] + 2) +  bitDepth + TRANSFORM_MATRIX_SHIFT) - maxLog2TrDynamicRange;
  int shift_2nd;// = (g_aucConvertToBit[iHeight] + 2) + TRANSFORM_MATRIX_SHIFT;

  int tmp[ 16*16 ];

  
  InitConvertToBit(g_aucConvertToBit);

  shift_1st = ((g_aucConvertToBit[iWidth] + 2) + bitDepth + TRANSFORM_MATRIX_SHIFT) - maxLog2TrDynamicRange;
  shift_2nd = (g_aucConvertToBit[iHeight] + 2) + TRANSFORM_MATRIX_SHIFT;

  switch (iWidth)
  {
    case 8:     partialButterfly8 ( block, tmp, shift_1st, iHeight );  break;
    case 16:    partialButterfly16( block, tmp, shift_1st, iHeight );  break;
  }

  switch (iHeight)
  {
    case 8:     partialButterfly8 ( tmp, coeff, shift_2nd, iWidth );    break;
    case 16:    partialButterfly16( tmp, coeff, shift_2nd, iWidth );    break;
  }
}


/** MxN inverse transform (2D)
*  \param bitDepth              [in]  bit depth
*  \param coeff                 [in]  transform coefficients
*  \param block                 [out] residual block
*  \param iWidth                [in]  width of transform
*  \param iHeight               [in]  height of transform
*  \param useDST                [in]
*  \param maxLog2TrDynamicRange [in]
*/
void xITrMxN(int *coeff, int *block, int iWidth, int iHeight)
{
  char g_aucConvertToBit  [ MAX_CU_SIZE+1 ];
  const int TRANSFORM_MATRIX_SHIFT = 6;
  const int bitDepth = 8;
  const int maxLog2TrDynamicRange = DYNAMIC_RANGE;

  const int clipMinimum = -(1 << maxLog2TrDynamicRange);
  const int clipMaximum =  (1 << maxLog2TrDynamicRange) - 1;
  
  int shift_1st;// = TRANSFORM_MATRIX_SHIFT + 1; //1 has been added to shift_1st at the expense of shift_2nd
  int shift_2nd;// = (TRANSFORM_MATRIX_SHIFT + maxLog2TrDynamicRange - 1) - bitDepth;
  
  int tmp[16*16];

  
  InitConvertToBit(g_aucConvertToBit);

  shift_1st = TRANSFORM_MATRIX_SHIFT + 1; //1 has been added to shift_1st at the expense of shift_2nd
  shift_2nd = (TRANSFORM_MATRIX_SHIFT + maxLog2TrDynamicRange - 1) - bitDepth;


  switch (iHeight)
  {
    
    case  8: partialButterflyInverse8 ( coeff, tmp, shift_1st, iWidth, clipMinimum, clipMaximum); break;
    case 16: partialButterflyInverse16( coeff, tmp, shift_1st, iWidth, clipMinimum, clipMaximum); break;
  }

  switch (iWidth)
  {
    // Clipping here is not in the standard, but is used to protect the "Pel" data type into which the inverse-transformed samples will be copied
    case  8: partialButterflyInverse8 ( tmp, block, shift_2nd, iHeight, -32768, 32767); break;
    case 16: partialButterflyInverse16( tmp, block, shift_2nd, iHeight, -32768, 32767); break;
  }
}
