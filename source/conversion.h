#pragma once

void ImageMath_MatrixMultiply8888(const void *src,
	size_t src_bytes_per_row,
	void *dst,
	size_t dst_bytes_per_row,
	unsigned long width,
	unsigned long height,
	const int16_t matrix[4 * 4],
	int32_t divisor,          // Applied after the matrix op
	const int16_t	*pre_bias,	// An array of 4 int16_t or NULL, added before matrix op
	const int32_t *post_bias,	// An array of 4 int32_t or NULL, added after matrix op
	int allow_tile); // if non-zero, operation may be tiled and multithreaded

void ConvertBGRAtoRGBA(int width, int height, const unsigned char* a, unsigned char* b);
void AlphaBlend(unsigned char* result, const unsigned char* fg, const unsigned char* bg);
void Blend32bppTo32bppChecker(int width, int height, bool swapRedBlue, const unsigned char* a, unsigned char* b);
