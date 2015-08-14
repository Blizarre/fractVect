#include "graphics.h"

#define cimg_use_png
#include "CImg.h"
using namespace cimg_library;

// May throw
void writeImage(pixel* data, const char* name, size_t width, size_t height)
{
  CImg<pixel> image(data, width,height,1,1,false);
  image.save_png(name);
}

