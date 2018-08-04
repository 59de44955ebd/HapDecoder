/*
 YCoCgDXT.c
 Hap Codec

 Copyright (c) 2012-2013, Tom Butterworth and Vidvox LLC. All rights reserved. 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


    Based on code by J.M.P. van Waveren / id Software, Inc.
    and changes by Chris Sidhall / Electronic Arts

    My changes are trivial:
      - Remove dependencies on other EAWebKit files
      - Mark unexported functions as static
      - Refactor to eliminate use of a global variable
      - Correct spelling of NVIDIA_7X_HARDWARE_BUG_FIX macro
      - Remove single usage of an assert macro


 Copyright (C) 2009-2011 Electronic Arts, Inc.  All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1.  Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2.  Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
 its contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///////////////////////////////////////////////////////////////////////////////
// BCImageCompressionEA.cpp
// File created by Chrs Sidhall
// Also please see Copyright (C) 2007 Id Software, Inc used in this file.
///////////////////////////////////////////////////////////////////////////////

#include "YCoCgDXT.h"
#include <string.h>
#include <stdlib.h>

/* ALWAYS_INLINE */
/* Derived from EAWebKit's AlwaysInline.h, losing some of its support for other compilers */

#ifndef ALWAYS_INLINE
#if (defined(__GNUC__) || defined(__clang__)) && !defined(DEBUG)
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined(_MSC_VER) && defined(NDEBUG)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif
#endif

// CSidhall Note: The compression code is directly from http://developer.nvidia.com/object/real-time-ycocg-dxt-compression.html
// It was missing some Emit functions but have tried to keep it as close as possible to the orignal version.
// Also removed some alpha handling which was never used and added a few overloaded functions (like ExtractBlock).

/*
 Real-Time YCoCg DXT Compression
 Copyright (C) 2007 Id Software, Inc.
 Written by J.M.P. van Waveren
 
 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 */

/*
 * This code was modified by Electronic Arts Inc Copyright � 2009
 */

// Compression code removed for HapDecoder by Valentin Schmidt 2018

#ifndef word
typedef unsigned short  word;
#endif 

#ifndef dword
typedef unsigned int    dword;
#endif

#define INSET_COLOR_SHIFT       4       // inset color bounding box
#define INSET_ALPHA_SHIFT       5       // inset alpha bounding box

#define C565_5_MASK             0xF8    // 0xFF minus last three bits
#define C565_6_MASK             0xFC    // 0xFF minus last two bits

#define NVIDIA_G7X_HARDWARE_BUG_FIX     // keep the colors sorted as: max, min

#if defined(__LITTLE_ENDIAN__) || defined(_WIN32)
#define EA_SYSTEM_LITTLE_ENDIAN
#endif

//--- YCoCgDXT5 Decompression ---
static void RestoreLumaAlphaBlock(  const void * pSource, byte * colorBlock){
    byte *pS=(unsigned char *) pSource;
    byte luma[8];     
    
    // Grabbed this standard table building from undxt.cpp UnInterpolatedAlphaBlock() 
    luma[0] = *pS++;                                                      
    luma[1] = *pS++;                                                      
    luma[2] = (byte)((6 * luma[0] + 1 * luma[1] + 3) / 7);    
    luma[3] = (byte)((5 * luma[0] + 2 * luma[1] + 3) / 7);    
    luma[4] = (byte)((4 * luma[0] + 3 * luma[1] + 3) / 7);    
    luma[5] = (byte)((3 * luma[0] + 4 * luma[1] + 3) / 7);    
    luma[6] = (byte)((2 * luma[0] + 5 * luma[1] + 3) / 7);    
    luma[7] = (byte)((1 * luma[0] + 6 * luma[1] + 3) / 7);    
    
    int rawIndexes;
    int raw;
    int colorIndex=3;
    // We have 6 bytes of indexes (3 bits * 16 texels)
    // Easier to process in 2 groups of 8 texels... 
    for(int j=0; j < 2; j++) {
        // Pack the indexes so we can shift out the indexes as a group 
        rawIndexes = *pS++;
        raw = *pS++;
        rawIndexes |= raw << 8;
        raw = *pS++;
        rawIndexes |= raw << 16;  
        
        // Since we still have to operate on the texels, just store it in a linear array workspace     
        for(int i=0; i < 8; i++) {       
            static const int LUMA_INDEX_FILTER = 0x7;   // To isolate the 3 bit luma index
            
            byte index = (byte)(rawIndexes & LUMA_INDEX_FILTER);
            colorBlock[colorIndex] = luma[index];
            colorIndex += 4;                                            
            rawIndexes  >>=3;      
        }
    }   
}

// Converts a 5.6.5 short back into 3 bytes 
static ALWAYS_INLINE void Convert565ToColor( const unsigned short value , byte *pOutColor ) 
{
    int c = value >> (5+6);
    pOutColor[0] = c << 3;  // Was a 5 bit so scale back up 
    
    c = value >> 5;
    c &=0x3f;               // Filter out the top value
    pOutColor[1] = c << 2;  // Was a 6 bit 
    
    c = value & 0x1f;         // Filter out the top values
    pOutColor[2] = c << 3;  // was a 5 bit so scale back up 
}

// Flip around the 2 bytes in a short
static ALWAYS_INLINE short ShortFlipBytes( short raw ) 
{
    return ((raw >> 8) & 0xff) | (raw << 8);
}

static void RestoreChromaBlock( const void * pSource, byte *colorBlock)
{
    unsigned short *pS =(unsigned short *) pSource;
    pS +=4;  // Color info stars after 8 bytes (first 8 is the Y/alpha channel info)
    
    unsigned short rawColor = *pS++;     
#ifndef EA_SYSTEM_LITTLE_ENDIAN  
    rawColor = ShortFlipBytes(rawColor);
#endif
  
    byte color[4][4];   // Color workspace 
    
    // Build the color lookup table 
    // The luma should have already been extracted and sitting at offset[3]
    Convert565ToColor( rawColor , &color[0][0] );     
    rawColor = *pS++;     
#ifndef EA_SYSTEM_LITTLE_ENDIAN  
    rawColor = ShortFlipBytes(rawColor);
#endif
    
    Convert565ToColor( rawColor , &color[1][0] ); 
    
    // EA/Alex Mole: mixing float & int operations is horrifyingly slow on some platforms, so we do it different!
#if defined(__PPU__) || defined(_XBOX)
    color[2][0] = (byte) ( ( ((int)color[0][0] * 3) + ((int)color[1][0]    ) ) >> 2 );
    color[2][1] = (byte) ( ( ((int)color[0][1] * 3) + ((int)color[1][1]    ) ) >> 2 );
    color[3][0] = (byte) ( ( ((int)color[0][0]    ) + ((int)color[1][0] * 3) ) >> 2 );
    color[3][1] = (byte) ( ( ((int)color[0][1]    ) + ((int)color[1][1] * 3) ) >> 2 );
#else
    color[2][0] = (byte) ( (color[0][0] * 0.75f) + (color[1][0] * 0.25f) );
    color[2][1] = (byte) ( (color[0][1] * 0.75f) + (color[1][1] * 0.25f) );
    color[3][0] = (byte) ( (color[0][0] * 0.25f) + (color[1][0] * 0.75f) );
    color[3][1] = (byte) ( (color[0][1] * 0.25f) + (color[1][1] * 0.75f) );
#endif
    
    byte scale = ((color[0][2] >> 3) + 1) >> 1; // Adjust for shifts instead of divide
    
    // Scale back values here so we don't have to do it for all 16 texels
    // Note: This is really only for the software version.  In hardware, the scale would need to be restored during the YCoCg to RGB conversion.
    for(int i=0; i < 4; i++) {
        color[i][0] = ((color[i][0] - 128) >> scale) + 128;
        color[i][1] = ((color[i][1] - 128) >> scale) + 128;
    }
    
    // Rebuild the color block using the indexes (2 bits per texel)
    int rawIndexes;
    int colorIndex=0;
    
    // We have 2 shorts of indexes (2 bits * 16 texels = 32 bits). (If can confirm 4x alignment, can grab it as a word with single loop) 
    for(int j=0; j < 2; j++) {
        rawIndexes = *pS++;
#ifndef EA_SYSTEM_LITTLE_ENDIAN  
        rawIndexes = ShortFlipBytes(rawIndexes);
#endif
        
        // Since we still have to operate on block, just store it in a linear array workspace     
        for(int i=0; i < 8; i++) {       
            static const int COCG_INDEX_FILTER = 0x3;   // To isolate the 2 bit chroma index
            
            unsigned char index = (unsigned char)(rawIndexes & COCG_INDEX_FILTER);
            colorBlock[colorIndex] = color[index][0];
            colorBlock[colorIndex+1] = color[index][1];
            colorBlock[colorIndex+2] = 255;  
            colorIndex += 4;                                            
            rawIndexes  >>=2;      
        }
    }   
}

// This stores a 4x4 texel block but can overflow the output rectangle size if it is not 4 texels aligned in size
static int ALWAYS_INLINE StoreBlock( const byte *colorBlock, const int stride, byte *outPtr ) {
    
    for ( int j = 0; j < 4; j++ ) {
        memcpy( (void*) outPtr,&colorBlock[j*4*4], 4*4 );
        outPtr += stride;
    }
    return 64;
}

// This store only the texels that are within the width and height boundaries so does not overflow
static int StoreBlock( const byte *colorBlock , const int stride, const int widthRemain, const int heightRemain,  byte *outPtr) 
{
    int outCount =0;
    int width = stride >> 2;    // Convert to int offsets
    
    int *pBlock32 = (int *) colorBlock;  // Since we are using ARGB, we assume 4 byte alignment is already being used
    int *pOutput32 = (int*) outPtr; 
    
    int widthMax = 4;
    if(widthRemain < 4) {
        widthMax = widthRemain;
    }
    
    int heightMax = 4;
    if(heightRemain < 4) {
        heightMax = heightRemain;    
    }
    
    for(int j =0; j < heightMax; j++) {
        for(int i=0; i < widthMax; i++) {
            pOutput32[i] = pBlock32[i];    
            outCount +=4;       
        }
        
        // Set up offset for next texel row source (keep existing if we are at the end)
        pBlock32 +=4;    
        pOutput32 +=width;
    }
    return outCount;
}

/**************************************************************************************************/
/*!
 \Function    DeCompressYCoCgDXT5( const byte *inBuf, byte *outBuf, const int width, const int height, const int stride ) 
 
 \Description  Decompression for YCoCgDXT5  
 Bascially does the reverse order of he compression.  
 
 Ouptut data still needs to be converted from YCoCg to ARGB after this function has completed
 (probably more efficient to convert it inside here but have not done so to stay closer to the orginal
 sample code and just make it easier to follow).
 
 16 bytes get unpacked into a 4x4 texel block (64 bytes output).
 
 The compressed format:
 2 bytes of min and max Y luma values (these are used to rebuild an 8 element Luma table)
 6 bytes of indexes into the luma table
 3 bits per index so 16 indexes total 
 2 shorts of min and max color values (these are used to rebuild a 4 element chroma table)
 5 bits Co
 6 bits Cg
 5 bits Scale. The scale can only be 1, 2 or 4. 
 4 bytes of indexes into the Chroma CocG table 
 2 bits per index so 16 indexes total
 
 
 \Input          const byte *inBuf
 \Input          byte *outBuf, 
 \Input          const int width
 \input          const int height 
 \input          const int stride for inBuf
 
 \Output         int size output in bytes
 
 
 \Version    1.0        01/12/09 Created
 1.1        12/21/09 Alex Mole: removed branches from tight inner loop
 1.2        11/10/10 CSidhall: Added stride for textures with different image and canvas sizes.
 */
/*************************************************************************************************F*/
extern "C" int DeCompressYCoCgDXT5( const byte *inBuf, byte *outBuf, const int width, const int height, const int stride )
{
    byte colorBlock[64];    // 4x4 texel work space a linear array 
    int outByteCount =0;
    const byte *pCurInBuffer = inBuf;
    
    int blockLineSize = stride * 4;  // 4 lines per loop
    for( int j = 0; j < ( height & ~3 ); j += 4, outBuf += blockLineSize )
    {
        int i;
        for( i = 0; i < ( width & ~3 ); i += 4 )
        {
            RestoreLumaAlphaBlock(pCurInBuffer, colorBlock);
            RestoreChromaBlock(pCurInBuffer, colorBlock);           
            outByteCount += StoreBlock(colorBlock, stride, outBuf + i * 4);
            pCurInBuffer += 16; // 16 bytes per block of compressed data
        }
        
        // Do we have some leftover columns?
        if( width & 3 )
        {
            int widthRemain = width & 3;
            
            RestoreLumaAlphaBlock(pCurInBuffer, colorBlock);
            RestoreChromaBlock(pCurInBuffer, colorBlock);
            
            outByteCount += StoreBlock(colorBlock , stride, widthRemain, 4 /* heightRemain >= 4 */, outBuf + i * 4);
            
            pCurInBuffer += 16; // 16 bytes per block of compressed data
        }
    }
    
    // Do we have some leftover lines?
    if( height & 3 )
    {
        int heightRemain = height & 3;
        
        for( int i = 0; i < width; i += 4 )
        {
            RestoreLumaAlphaBlock(pCurInBuffer, colorBlock);
            RestoreChromaBlock(pCurInBuffer, colorBlock);
            
            int widthRemain = width - i;
            outByteCount += StoreBlock(colorBlock , stride, widthRemain, heightRemain,  outBuf + i * 4);
            
            pCurInBuffer += 16; // 16 bytes per block of compressed data
        }
    }
    
    return outByteCount;
}
