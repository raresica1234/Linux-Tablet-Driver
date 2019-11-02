#include "Area.h"


Area::Area(float x, float y, float width, float height)
	: x(x), y(y), width(width), height(height) {
		
}


void Area::map(float &xVal, float &yVal, Area *from, Area *to) {
	xVal = ((float)xVal - from->x) / from->width * to->width + to->x;
	yVal = ((float)yVal - from->y) / from->height * to->height + to->y;
}
