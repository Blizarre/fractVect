#include <cstddef>
#include <memory>
#include <algorithm>

typedef int pixel;

struct Pix {
	char R; char G; char B; char A;
};

void writeImage(pixel* data, const char* name, std::size_t width, std::size_t height);

class Colormap
{
protected:
	std::unique_ptr<Pix[]> cm;

public:
	Colormap();
	const pixel* getColorMap() const;
};