#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include "CursorHelper.h"


CursorHelper::CursorHelper() {	
	// Picking default display
	m_Display = XOpenDisplay(NULL);
	if(m_Display == NULL) {
		printf("Could not open display!");
		exit(-1);
	}
}

CursorHelper::~CursorHelper() {
	// Closing default display
	XCloseDisplay(m_Display);
}

void CursorHelper::MoveTo(int x, int y) {
	// Creating a dummy event
	XEvent event;

	// Getting the cursor position
	XQueryPointer(m_Display, DefaultRootWindow(m_Display), &event.xbutton.root,
			&event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root,
			&event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

	int lastX, lastY;
	lastX = event.xbutton.x;
	lastY = event.xbutton.y;
	
	// Changing absolute movement to relative movement
	x -= lastX;
	y -= lastY;
	
	// Moving the cursor
	XWarpPointer(m_Display, None, None, 0, 0, 0, 0, x, y);

	// Flushing changes
	XFlush(m_Display);
}

void CursorHelper::Click(MouseButton button) {
	XTestFakeButtonEvent(m_Display, static_cast<unsigned int>(button), True, 0);	
}

void CursorHelper::Release(MouseButton button) {
	XTestFakeButtonEvent(m_Display, static_cast<unsigned int>(button), False, 0);	
}
