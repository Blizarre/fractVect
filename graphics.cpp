#include "graphics.h"
#include "cImg.h"
using namespace cimg_library;

// May throw
void writeImage(pixel* data, const char* name, size_t width, size_t height)
{
	Pix * p = reinterpret_cast<Pix*>(data);

	// from RGBARGBARGBARGBA to RRRRGGGGBBBB
	std::unique_ptr<char[]> cImgformat = std::make_unique<char[]>(width * height * 3);
	for (size_t i = 0; i < width * height; i++)
	{
		cImgformat[i * 1] = p[i].R;
		cImgformat[width * height + i] = p[i].G;
		cImgformat[width * height * 2 + i] = p[i].B;
	}

	CImg<char> image(cImgformat.get(), static_cast<int>(width), static_cast<int>(height), 1, 3, false);
	image.save(name);
}


Colormap::Colormap()
{
	for (int i = 0; i < 255; i++)
	{
		Pix& val = cm[i];
		val.R = static_cast<char>(255.0 * std::log(i) / std::log(255.0));
		val.G = i >> 1;
		val.B = i >> 1;
		val.A = 255;
	}

	// Island in the middle of mandelbrot is best left completely dark
	Pix& val = cm[255];
	val.R = 0; val.G = 0; val.B = 0; val.A = 255;
}

const pixel* Colormap::getColorMap() const
{
	return reinterpret_cast<const pixel*>(cm.data());
}