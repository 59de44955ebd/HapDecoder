/*
 YCoCg.c
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
 */

 // Valentin Schmidt 2018: Stripped to minimal needs for HapDecoder

#include "YCoCg.h"
#include <stdint.h>
#include "conversion.h"

void ConvertCoCgAY8888ToBGRA( uint8_t *src, uint8_t *dst, unsigned long width, unsigned long height, size_t src_rowbytes, size_t dst_rowbytes, int allow_tile )
{
    const int16_t pre_bias[4] = { -128, -128, 0, 0 };
    
    const int16_t matrix[16] = {
        -1,  0,  1,  0, // Co
        -1,  1, -1,  0, // Cg
         0,  0,  0,  1, // A
         1,  1,  1,  0  // Y
    };
    
    ImageMath_MatrixMultiply8888(src, src_rowbytes, dst, dst_rowbytes, width, height, matrix, 1, pre_bias, NULL, allow_tile);
}
