#pragma once

#include <X11/Xlib.h>
#include <iostream>

#include "Enums.h"

class CursorHelper {
private:
	Display* m_Display = nullptr;

public:
	CursorHelper();
	~CursorHelper();

	void MoveTo(int x, int y);
	void Click(MouseButton button);
	void Release(MouseButton button);
};
