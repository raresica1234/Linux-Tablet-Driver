#include "Area.h"


Area::Area(float x, float y, float width, float height)
	: x(x), y(y), width(width), height(height) {
		
}


void Area::map(int &xVal, int &yVal, Area *from, Area *to) {
	float xval = ((float)xVal - from->x) / from->width * to->width + to->x;
	float yval = ((float)yVal - from->y) / from->height * to->height + to->y;
	xVal = (int)xval;
	yVal = (int)yval;
}
