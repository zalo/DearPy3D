#pragma once

#include "mvWindows.h"

// forward declarations
struct mvViewport;

void initialize_viewport    (mvViewport& viewport, int width, int height);
void cleanup_viewport       (mvViewport& viewport);
void process_viewport_events(mvViewport& viewport);

struct mvViewport
{
	int         width         = 500;
	int         height        = 500;
	HWND        handle        = nullptr;
	WNDCLASSEX  wc;
	HINSTANCE   hinstance     = nullptr;
	bool        running       = false;
	bool        cursorEnabled = true;
	double      lastX         = 0.0;
	double      lastY         = 0.0;
	double      deltaX        = 0.0;
	double      deltaY        = 0.0;
	bool        resized       = false;
};
