#include "graphics.h"
#include <iostream>
#include <cmath>
#include "timePoints.h"
#include "immintrin.h"
#include <memory>
#include "SDLWrapper.h"

using namespace std;

enum PerfType { X86, SSE, AVX2 };

struct params {
	int windowWidth, windowHeight;
	float fractWidth, fractHeight;
	float fractPosX, fractPosY;
	int pitch; // distance between rows in !bytes!
	pixel* data; // pixel data
	Colormap colormap;
	PerfType perf;

	size_t nbPixels() { return windowWidth * windowHeight; }
};

void generateImageX86(params &p);
void generateImageAVX(params &p);
void generateImageSSE(params &p);

void saveImage(params &p)
{
	std::unique_ptr<pixel[]> buffer = make_unique<pixel[]>(p.windowWidth * p.windowHeight);
	p.data = buffer.get();
	p.pitch = p.windowWidth * 4;

	switch (p.perf) // use inheritance
	{
	case PerfType::AVX2:
		generateImageAVX(p);
		break;
	case PerfType::X86:
		generateImageX86(p);
		break;
	case PerfType::SSE:
		generateImageSSE(p);
		break;
	}
	writeImage(p.data, "out.bmp", p.windowWidth, p.windowHeight);
}

void renderImage(params& p, SDLWrapper& sdl)
{
	TimePoints t;

	p.data = sdl.startWorkingOnTexture(&p.pitch);

	switch (p.perf) // use inheritance
	{
	case PerfType::AVX2:
		generateImageAVX(p);
		break;
	case PerfType::SSE:
		generateImageSSE(p);
		break;
	case PerfType::X86:
		generateImageX86(p);
		break;
	}

	t.checkPoint("image generated in {TIME} ms.");
	sdl.renderTexture();
}


int main(int argc, char* argv[])
{
	TimePoints t;
	Colormap c;
	params p;
	p.perf = PerfType::X86;

	if (argc != 1 + 2)
	{
		cout << "usage:\n\t" << argv[0] << " width height" << endl;
		return 1;
	}

	p.windowWidth = atol(argv[1]), p.windowHeight = atol(argv[2]);

	if (p.nbPixels() == 0)
	{
		cout << "Invalid argument, width AND height must be non-null" << endl;
		return 2;
	}

	if (p.windowWidth % 8 != 0)
	{
		cout << "width must be a multiple of 8" << endl;
		return 3;
	}

	SDLWrapper sdl(p.windowWidth, p.windowHeight);

	std::unique_ptr<pixel[]> im = std::make_unique<pixel[]>(p.nbPixels());

	t.reset();
	p.fractPosX = -1.5;
	p.fractPosY = -0.5;
	p.colormap = c;

	p.fractHeight = 1.0;
	p.fractWidth = p.windowWidth * p.fractHeight / p.windowHeight;

	bool cont = true;

	sdl.onMouseMotion([&](size_t x, size_t y)
	{
		p.fractPosX = -p.fractWidth / 2.0f + (p.windowWidth / 2.0f - x) / (p.windowWidth);
		p.fractPosY = -p.fractHeight / 2.0f + (p.windowHeight / 2.0f - y) / (p.windowHeight);
	});

	sdl.onKeyPress(SDLK_o, [&]() {
		switch (p.perf)
		{
		case PerfType::X86:
			cout << "Using SSE" << endl;
			p.perf = PerfType::SSE; break;
		case PerfType::SSE:
			cout << "Using AVX2" << endl;
			p.perf = PerfType::AVX2; break;
		case PerfType::AVX2:
			cout << "Using X86" << endl;
			p.perf = PerfType::X86; break;
		}
	});


	sdl.onKeyPress(SDLK_p, [&]() {
		p.fractWidth *= 1.2; p.fractHeight *= 1.2;
	});

	sdl.onKeyPress(SDLK_m, [&]() {
		p.fractWidth *= 0.8; p.fractHeight *= 0.8;
	});

	sdl.onKeyPress(SDLK_s, [&]() {
		saveImage(p);
	});

	sdl.onKeyPress(SDLK_ESCAPE, [&]() { cont = false; });
	sdl.onQuitEvent([&]() { cont = false; });

	while (cont)
	{
		sdl.processEvents();
		renderImage(p, sdl);
	}

	return 0;
}


void generateImageX86(params &p)
{
	const pixel * colormap = p.colormap.getColorMap();

	float xMin = p.fractPosX;
	float xMax = xMin + p.fractWidth;

	float yMin = p.fractPosY;
	float yMax = yMin + p.fractHeight;

	float xSpan = xSpan = (xMax - xMin) / p.windowWidth;
	float ySpan = (yMax - yMin) / p.windowHeight;

#pragma omp parallel for
	for (int j = 0; j < p.windowHeight; j++)
		for (int i = 0; i < p.windowWidth; i++)
		{
			// z0 = position
			float x0 = i * xSpan + xMin;
			float y0 = j * ySpan + yMin;

			// zp = 0
			double xp = 0, yp = 0;

			// while( ||zn|| < 2.0 ) zn = zp * zp + z0
			int iter = 0;
			while (xp*xp + yp*yp < 2.0 * 2.0 && iter < 255)
			{
				double tmp;
				tmp = (xp * xp - yp * yp) + x0;
				yp = (xp * yp + yp * xp) + y0;
				xp = tmp;
				iter++;
			}

			p.data[j * p.pitch / 4 + i] = colormap[iter];
		}
}


void generateImageAVX(params &p)
{
	const pixel * colormap = p.colormap.getColorMap();

	float xMin = p.fractPosX;
	float xMax = xMin + p.fractWidth;

	float yMin = p.fractPosY;
	float yMax = yMin + p.fractHeight;

	__m256 basis = { 0, 1, 2, 3, 4, 5, 6, 7 };
	__m256 xSpan = _mm256_set1_ps((xMax - xMin) / p.windowWidth);
	float ySpan = (yMax - yMin) / p.windowHeight;
	__m256 xMin256 = _mm256_set1_ps(xMin);

#pragma omp parallel for
	for (int j = 0; j < p.windowHeight; j++)
		for (int i = 0; i < p.windowWidth; i += 8)
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
				p.data[j * (p.pitch / 4) + i + k] = colormap[result.m256i_i32[k]];
		}
}

void generateImageSSE(params &p)
{
	const pixel * colormap = p.colormap.getColorMap();

	float xMin = p.fractPosX;
	float xMax = xMin + p.fractWidth;

	float yMin = p.fractPosY;
	float yMax = yMin + p.fractHeight;

	__m128 basis = { 0, 1, 2, 3 };
	__m128 xSpan = _mm_set1_ps((xMax - xMin) / p.windowWidth);
	float ySpan = (yMax - yMin) / p.windowHeight;
	__m128 xMin128 = _mm_set1_ps(xMin);

#pragma omp parallel for
	for (int j = 0; j < p.windowHeight; j++)
		for (int i = 0; i < p.windowWidth; i += 4)
		{
			// z0 = position
			__m128 offsets = _mm_add_ps(basis, _mm_set1_ps(static_cast<float>(i)));
			__m128 x0 = _mm_fmadd_ps(xSpan, offsets, xMin128);
			__m128 y0 = _mm_set1_ps(j * ySpan + yMin);

			// zp = 0
			__m128 xp = _mm_setzero_ps();
			__m128 yp = _mm_setzero_ps();

			__m128 iter = _mm_setzero_ps();
			__m128 escapeVal = _mm_set1_ps(4.0);
			__m128 ones = _mm_set1_ps(1.0);

			__m128 underMaxNorm = _mm_setzero_ps();
			int nbIter = 0;
			bool allUnderMaxNorm = true;

			// while( ||zn|| < 2.0 ) zn = zp * zp + z0
			while (nbIter < 255 && allUnderMaxNorm)
			{
				// tmp = (xp * xp - yp * yp) + x0;
				__m128 tmp = _mm_add_ps(_mm_fmsub_ps(xp, xp, _mm_mul_ps(yp, yp)), x0);
				// yp = (xp * yp + yp * xp) + y0;
				yp = _mm_add_ps(_mm_fmadd_ps(xp, yp, _mm_mul_ps(yp, xp)), y0);

				xp = tmp;

				// norm =  sqrt( || zp || )
				__m128 norm = _mm_fmadd_ps(xp, xp, _mm_mul_ps(yp, yp));

				// if (norm > 4) iter++
				underMaxNorm = _mm_cmp_ps(norm, escapeVal, _CMP_LT_OQ);
				iter = _mm_add_ps(iter, _mm_and_ps(underMaxNorm, ones));
				nbIter++;

				allUnderMaxNorm = (0 == _mm_testz_ps(underMaxNorm, underMaxNorm));
			}

			__m128i result = _mm_cvtps_epi32(iter);

			for (int k = 0; k < 4; k++)
				p.data[j * (p.pitch / 4) + i + k] = colormap[result.m128i_i32[k]];
		}
}