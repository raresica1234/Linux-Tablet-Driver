#pragma once

struct Area {
	float x;
	float y;
	float width;
	float height;

	Area(float x, float y, float width, float height);

	static void map(float &xVal, float &yVal, Area *from, Area *to);
};
