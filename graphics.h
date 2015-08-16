#pragma once 

#include <cstddef>
#include <memory>
#include <algorithm>
#include <array>

typedef int pixel;

struct Pix {
	char B; char G; char R; char A;
};

void writeImage(pixel* data, const char* name, std::size_t width, std::size_t height);


class Colormap
{
protected:
	std::array<Pix, 256> cm;

public:
	Colormap();
	const pixel* getColorMap() const;
};