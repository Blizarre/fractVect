#include "graphics.h"
#include <iostream>
#include <cmath>

using namespace std;
pixel* generateImage(size_t width, size_t height);

int main(int argc, char* argv[])
{
   if(argc != 1 + 2)
   {
        cout << "usage:\n\t" << argv[0] << " width height" << endl;
        return 1;
   }

   size_t w, h;
   w = atol(argv[1]), h = atol(argv[2]);

   if(w * h == 0)
   {
       cout << "Invalid argument, width AND height must be non-null" << endl;
       return 2;
   }

   if(w % 8 != 0)
   {
        cout << "width must be a multiple of 8" <<endl;
        return 3;
   }

   pixel* im = generateImage(w, h);
   writeImage(im, "out.png", w, h);
   return 0;
}

float norm(float x, float y)
{
    float val = sqrt(x*x + y*y);
    return val;
}

pixel* generateImage(size_t width, size_t height)
{
    pixel * im = new pixel[width * height];

    int i, j;
    float x, y, val;
    double xMin = -2, xMax = 2, yMin = -1, yMax = 1;
    double xSpan = (xMax - xMin) / width;
    double ySpan = (yMax - yMin) / height;
    int maxIter = 255;

    #pragma omp parallel
    for(j = 0; j < height; j++)
        for(i = 0; i < width; i++)
        {
            // z0 = position
            float x0 = i * xSpan + xMin;
            float y0 = j * ySpan + yMin;

            // zp = 0
            double xp = 0, yp = 0;

            // while( |zn| < 4.0 ) zn = zp * zp + z0
            int iter = 0;
            while(xp*xp + yp*yp < 16.0 && iter < maxIter)
            {
                double tmp;
                tmp = (xp * xp - yp * yp) + x0;
                yp = (xp * yp + yp * xp) + y0;
                xp = tmp;
                iter ++;
            }

            im[j * width + i] = static_cast<unsigned char>( iter );//(maxIter/logf(maxIter)) * logf(iter) );
        }

    return im;
}
