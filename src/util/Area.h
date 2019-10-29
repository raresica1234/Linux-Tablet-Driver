#pragma once

struct Area {
	float x;
	float y;
	float width;
	float height;

	Area(float x, float y, float width, float height);

	static void map(int &xVal, int &yVal, Area *from, Area *to);
};
