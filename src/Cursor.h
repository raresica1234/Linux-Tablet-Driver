#pragma once

#include <X11/Xlib.h>

#include "Enums.h"

class Cursor {
private:
	Display* m_Display = nullptr;

public:
	Cursor();
	~Cursor();

	void MoveTo(int x, int y);
	void Click(MouseButton button);
	void Release(MouseButton button);
}
