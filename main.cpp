#include "graphics.h"
#include <iostream>
#include <cmath>
#include "timePoints.h"
#include "immintrin.h"
#include <memory>

using namespace std;
std::unique_ptr<pixel> generateImage(int width, int height, const Colormap& c);
std::unique_ptr<pixel> generateImageAVX(int width, int height, const Colormap& c);


int main(int argc, char* argv[])
{

	TimePoints t;
	Colormap c;
	if (argc != 1 + 2)
	{
		cout << "usage:\n\t" << argv[0] << " width height" << endl;
		return 1;
	}

	size_t w, h;
	w = atol(argv[1]), h = atol(argv[2]);

	if (w * h == 0)
	{
		cout << "Invalid argument, width AND height must be non-null" << endl;
		return 2;
	}

	if (w % 8 != 0)
	{
		cout << "width must be a multiple of 8" << endl;
		return 3;
	}

	t.reset();
	std::unique_ptr<pixel> imAVX = generateImageAVX(w, h, c);
	t.checkPoint("image generated using AVX intrinsics in {TIME} ms.");

	writeImage(imAVX.get(), "outAVX.bmp", w, h);

	t.reset();
	std::unique_ptr<pixel> im = generateImage(w, h, c);
	t.checkPoint("image generated in {TIME} ms.");

	writeImage(im.get(), "out.bmp", w, h);
	return 0;
}


std::unique_ptr<pixel> generateImage(int width, int height, const Colormap& c)
{
	pixel * im = new pixel[width * height];
	const pixel * colormap = c.getColorMap();

	float xMin = -2, xMax = 2, yMin = -1, yMax = 1;
	float xSpan = (xMax - xMin) / width;
	float ySpan = (yMax - yMin) / height;
	int maxIter = 255;

#pragma omp parallel for
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
		{
			// z0 = position
			float x0 = i * xSpan + xMin;
			float y0 = j * ySpan + yMin;

			// zp = 0
			double xp = 0, yp = 0;

			// while( ||zn|| < 2.0 ) zn = zp * zp + z0
			int iter = 0;
			while (xp*xp + yp*yp < 2.0 * 2.0 && iter < maxIter)
			{
				double tmp;
				tmp = (xp * xp - yp * yp) + x0;
				yp = (xp * yp + yp * xp) + y0;
				xp = tmp;
				iter++;
			}

			im[j * width + i] = colormap[iter];
		}

	return std::unique_ptr<pixel>(im);
}



std::unique_ptr<pixel> generateImageAVX(int width, int height, const Colormap& c)
{
	pixel * im = new pixel[width * height];
	const pixel * colormap = c.getColorMap();

	float xMin = -2, xMax = 2, yMin = -1, yMax = 1;

	__m256 basis = { 0, 1, 2, 3, 4, 5, 6, 7 };
	__m256 xSpan = _mm256_set1_ps((xMax - xMin) / width);
	float ySpan = (yMax - yMin) / height;
	__m256 xMin256 = _mm256_set1_ps(xMin);

#pragma omp parallel for
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i += 8)
		{
			// z0 = position
			__m256 offsets = _mm256_add_ps(basis, _mm256_set1_ps(static_cast<float>(i)));
			__m256 x0 = _mm256_fmadd_ps(xSpan, offsets, xMin256);
			__m256 y0 = _mm256_set1_ps(j * ySpan + yMin);

			// zp = 0
			__m256 xp = _mm256_setzero_ps();
			__m256 yp = _mm256_setzero_ps();

			__m256 iter = _mm256_setzero_ps();
			__m256 escapeVal = _mm256_set1_ps(4.0);
			__m256 ones = _mm256_set1_ps(1.0);

			__m256 underMaxNorm = _mm256_setzero_ps();
			int nbIter = 0;
			bool allUnderMaxNorm = true;

			// while( ||zn|| < 2.0 ) zn = zp * zp + z0
			while (nbIter < 255 && allUnderMaxNorm)
			{
				// tmp = (xp * xp - yp * yp) + x0;
				__m256 tmp = _mm256_add_ps(_mm256_fmsub_ps(xp, xp, _mm256_mul_ps(yp, yp)), x0);
				// yp = (xp * yp + yp * xp) + y0;
				yp = _mm256_add_ps(_mm256_fmadd_ps(xp, yp, _mm256_mul_ps(yp, xp)), y0);

				xp = tmp;

				// norm =  sqrt( || zp || )
				__m256 norm = _mm256_fmadd_ps(xp, xp, _mm256_mul_ps(yp, yp));

				// if (norm > 4) iter++
				underMaxNorm = _mm256_cmp_ps(norm, escapeVal, _CMP_LT_OQ);
				iter = _mm256_add_ps(iter, _mm256_and_ps(underMaxNorm, ones));
				nbIter++;

				allUnderMaxNorm = (0 == _mm256_testz_ps(underMaxNorm, underMaxNorm));
			}

			__m256i result = _mm256_cvtps_epi32(iter);

			for (int k = 0; k < 8; k++)
				im[j * width + i + k] = colormap[result.m256i_i32[k]];
		}

	return std::unique_ptr<pixel>(im);
}