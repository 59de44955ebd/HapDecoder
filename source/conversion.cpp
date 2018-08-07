/*
Based on Hap code by Tom Butterworth
Copyright (c) 2012-2013, Tom Butterworth and Vidvox LLC.

Valentin Schmidt 2018: implemented OpenMP support for ImageMath_MatrixMultiply8888 and Blend32bppTo32bppChecker 
*/

#include <stdint.h>

//######################################
// OMP
//######################################
#include <omp.h>

#define CLAMP_UINT8( x ) ( (x) < 0 ? (0) : ( (x) > 255 ? 255 : (x) ) )

//######################################
// VS: mac code removed, converted to using OMP
//######################################
void ImageMath_MatrixMultiply8888(const void *src,
	size_t src_bytes_per_row,
	void *dst,
	size_t dst_bytes_per_row,
	unsigned long width,
	unsigned long height,
	const int16_t matrix[4 * 4],
	int32_t divisor,          // Applied after the matrix op
	const int16_t *pre_bias,	// An array of 4 int16_t or NULL, added before matrix op
	const int32_t *post_bias,
	bool bUseOMP
)
{
	const uint8_t *pixel_src;
	uint8_t *pixel_dst;

	size_t src_bytes_extra_per_row = src_bytes_per_row - (width * 4);
	size_t dst_bytes_extra_per_row = dst_bytes_per_row - (width * 4);

	if (bUseOMP) {
		int numThreads = omp_get_max_threads();

		for (int y = 0; y < height; y++) {

			//######################################
			#pragma omp parallel num_threads(numThreads)
			#pragma omp for
			//######################################
			for (int x = 0; x < width; x++) {

				pixel_src = (const uint8_t *)src + 4 * x + y * (4 * width + src_bytes_extra_per_row);
				pixel_dst = (uint8_t *)dst + 4 * x + y * (4 * width + dst_bytes_extra_per_row);

				int32_t result[4];
				int32_t source[4] = { pixel_src[0], pixel_src[1], pixel_src[2], pixel_src[3] };

				// Pre-bias
				if (pre_bias != NULL)
				{
					source[0] += pre_bias[0];
					source[1] += pre_bias[1];
					source[2] += pre_bias[2];
					source[3] += pre_bias[3];
				}

				// MM
				result[0] = (matrix[0] * source[0]) + (matrix[4] * source[1]) + (matrix[8] * source[2]) + (matrix[12] * source[3]);
				result[1] = (matrix[1] * source[0]) + (matrix[5] * source[1]) + (matrix[9] * source[2]) + (matrix[13] * source[3]);
				result[2] = (matrix[2] * source[0]) + (matrix[6] * source[1]) + (matrix[10] * source[2]) + (matrix[14] * source[3]);
				result[3] = (matrix[3] * source[0]) + (matrix[7] * source[1]) + (matrix[11] * source[2]) + (matrix[15] * source[3]);

				// Post-bias
				if (post_bias != NULL)
				{
					result[0] += post_bias[0];
					result[1] += post_bias[1];
					result[2] += post_bias[2];
					result[3] += post_bias[3];
				}

				// Divisor
				result[0] /= divisor;
				result[1] /= divisor;
				result[2] /= divisor;
				result[3] /= divisor;

				// Clamp
				pixel_dst[0] = CLAMP_UINT8(result[0]);
				pixel_dst[1] = CLAMP_UINT8(result[1]);
				pixel_dst[2] = CLAMP_UINT8(result[2]);
				pixel_dst[3] = CLAMP_UINT8(result[3]);

			}//x

		}//y
	}
	else {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {

				int32_t result[4];
				int32_t source[4] = { pixel_src[0], pixel_src[1], pixel_src[2], pixel_src[3] };

				// Pre-bias
				if (pre_bias != NULL)
				{
					source[0] += pre_bias[0];
					source[1] += pre_bias[1];
					source[2] += pre_bias[2];
					source[3] += pre_bias[3];
				}

				// MM
				result[0] = (matrix[0] * source[0]) + (matrix[4] * source[1]) + (matrix[8] * source[2]) + (matrix[12] * source[3]);
				result[1] = (matrix[1] * source[0]) + (matrix[5] * source[1]) + (matrix[9] * source[2]) + (matrix[13] * source[3]);
				result[2] = (matrix[2] * source[0]) + (matrix[6] * source[1]) + (matrix[10] * source[2]) + (matrix[14] * source[3]);
				result[3] = (matrix[3] * source[0]) + (matrix[7] * source[1]) + (matrix[11] * source[2]) + (matrix[15] * source[3]);

				// Post-bias
				if (post_bias != NULL)
				{
					result[0] += post_bias[0];
					result[1] += post_bias[1];
					result[2] += post_bias[2];
					result[3] += post_bias[3];
				}

				// Divisor
				result[0] /= divisor;
				result[1] /= divisor;
				result[2] /= divisor;
				result[3] /= divisor;

				// Clamp
				pixel_dst[0] = CLAMP_UINT8(result[0]);
				pixel_dst[1] = CLAMP_UINT8(result[1]);
				pixel_dst[2] = CLAMP_UINT8(result[2]);
				pixel_dst[3] = CLAMP_UINT8(result[3]);

				pixel_src += 4;
				pixel_dst += 4;
			}
			pixel_src += src_bytes_extra_per_row;
			pixel_dst += dst_bytes_extra_per_row;
		}
	}
}

//######################################
// 
//######################################
void ConvertBGRAtoRGBA(int width, int height, const unsigned char* a, unsigned char* b, bool bUseOMP) {
	int numPixels = width * height;
	if (bUseOMP) {
		int numThreads = omp_get_max_threads();
		//######################################
		#pragma omp parallel num_threads(numThreads)
		//######################################
		{
			int thread_id = omp_get_thread_num();
			const unsigned char* aa = a + (numPixels * 4 / numThreads) * thread_id;
			unsigned char* bb = b + (numPixels * 4 / numThreads) * thread_id;
			//######################################
			#pragma omp for
			//######################################
			for (int i = 0; i < numPixels; i++) {
				bb[0] = aa[2];
				bb[1] = aa[1];
				bb[2] = aa[0];
				bb[3] = aa[3];
				aa += 4;
				bb += 4;
			}
		}
	}
	else {
		for (int i = 0; i < numPixels; i++)
		{
			b[0] = a[2];
			b[1] = a[1];
			b[2] = a[0];
			b[3] = a[3];
			a += 4;
			b += 4;
		}
	}
}


//######################################
//
//######################################
void AlphaBlend (unsigned char* result, const unsigned char* fg, const unsigned char* bg) {
	unsigned int alpha = fg[3] + 1;
	unsigned int inv_alpha = 256 - fg[3];
	result[0] = (unsigned char)((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
	result[1] = (unsigned char)((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
	result[2] = (unsigned char)((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
}

//######################################
// 
//######################################
void Blend32bppTo32bppChecker (int width, int height, bool swapRedBlue, const unsigned char* a, unsigned char* b, bool bUseOMP) {

	unsigned int checkerA = 0x00666666;
	unsigned int checkerB = 0x00999999;
	int numPixels = width * height;

	if (bUseOMP) {

		int numThreads = omp_get_max_threads();
		//######################################
		#pragma omp parallel num_threads(numThreads)
		//######################################
		{
			int thread_id = omp_get_thread_num();
			const unsigned char* aa = a + (numPixels * 4 / numThreads) * thread_id;
			unsigned char* bb = b + (numPixels * 4 / numThreads) * thread_id;
			//######################################
			#pragma omp for
			//######################################
			for (int i = 0; i < numPixels; i++) {
				bb[0] = aa[0];
				bb[1] = aa[1];
				bb[2] = aa[2];
				bb[3] = aa[3];

				int x = (i % width) / 64;
				int y = i / (64 * width);
				unsigned int checker = checkerA;
				if (((x + y) & 1) == 0)
					checker = checkerB;

				AlphaBlend(bb, aa, (const unsigned char*)&checker);

				if (swapRedBlue) {
					unsigned char temp = bb[0];
					bb[0] = bb[2];
					bb[2] = temp;
				}

				aa += 4;
				bb += 4;
			}
		}
	}
	else {
		for (int i = 0; i < numPixels; i++) {
			b[0] = a[0];
			b[1] = a[1];
			b[2] = a[2];
			b[3] = a[3];

			int x = (i % width) / 64;
			int y = i / (64 * width);
			unsigned int checker = checkerA;
			if (((x + y) & 1) == 0)
				checker = checkerB;

			AlphaBlend(b, a, (const unsigned char*)&checker);

			if (swapRedBlue) {
				unsigned char temp = b[0];
				b[0] = b[2];
				b[2] = temp;
			}

			a += 4;
			b += 4;
		}
	}
}
