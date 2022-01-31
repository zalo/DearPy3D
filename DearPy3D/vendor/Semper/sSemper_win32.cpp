#include "sSemper.h"
#include "sSemper_Internal.h"

#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include <objbase.h> // For COM headers
#include <tchar.h>
#include <dwmapi.h>

#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

#if defined SEMPER_INCLUDE_IMGUI
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

static
bool is_vk_down(int vk)
{
	return (::GetKeyState(vk) & 0x8000) != 0;
}

static int
get_horizontal_shift(const HWND window_handle)
{
	RECT window_rectangle, frame_rectangle;
	GetWindowRect(window_handle, &window_rectangle);
	DwmGetWindowAttribute(window_handle,
		DWMWA_EXTENDED_FRAME_BOUNDS, &frame_rectangle, sizeof(RECT));

	return frame_rectangle.left - window_rectangle.left;
}

static sKey
VirtualKeyToSemperKey(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_TAB: return sKey_Tab;
	case VK_LEFT: return sKey_LeftArrow;
	case VK_RIGHT: return sKey_RightArrow;
	case VK_UP: return sKey_UpArrow;
	case VK_DOWN: return sKey_DownArrow;
	case VK_PRIOR: return sKey_PageUp;
	case VK_NEXT: return sKey_PageDown;
	case VK_HOME: return sKey_Home;
	case VK_END: return sKey_End;
	case VK_INSERT: return sKey_Insert;
	case VK_DELETE: return sKey_Delete;
	case VK_BACK: return sKey_Backspace;
	case VK_SPACE: return sKey_Space;
	case VK_RETURN: return sKey_Enter;
	case VK_ESCAPE: return sKey_Escape;
	case VK_OEM_7: return sKey_Apostrophe;
	case VK_OEM_COMMA: return sKey_Comma;
	case VK_OEM_MINUS: return sKey_Minus;
	case VK_OEM_PERIOD: return sKey_Period;
	case VK_OEM_2: return sKey_Slash;
	case VK_OEM_1: return sKey_Semicolon;
	case VK_OEM_PLUS: return sKey_Equal;
	case VK_OEM_4: return sKey_LeftBracket;
	case VK_OEM_5: return sKey_Backslash;
	case VK_OEM_6: return sKey_RightBracket;
	case VK_OEM_3: return sKey_GraveAccent;
	case VK_CAPITAL: return sKey_CapsLock;
	case VK_SCROLL: return sKey_ScrollLock;
	case VK_NUMLOCK: return sKey_NumLock;
	case VK_SNAPSHOT: return sKey_PrintScreen;
	case VK_PAUSE: return sKey_Pause;
	case VK_NUMPAD0: return sKey_Keypad0;
	case VK_NUMPAD1: return sKey_Keypad1;
	case VK_NUMPAD2: return sKey_Keypad2;
	case VK_NUMPAD3: return sKey_Keypad3;
	case VK_NUMPAD4: return sKey_Keypad4;
	case VK_NUMPAD5: return sKey_Keypad5;
	case VK_NUMPAD6: return sKey_Keypad6;
	case VK_NUMPAD7: return sKey_Keypad7;
	case VK_NUMPAD8: return sKey_Keypad8;
	case VK_NUMPAD9: return sKey_Keypad9;
	case VK_DECIMAL: return sKey_KeypadDecimal;
	case VK_DIVIDE: return sKey_KeypadDivide;
	case VK_MULTIPLY: return sKey_KeypadMultiply;
	case VK_SUBTRACT: return sKey_KeypadSubtract;
	case VK_ADD: return sKey_KeypadAdd;
	case IM_VK_KEYPAD_ENTER: return sKey_KeypadEnter;
	case VK_LSHIFT: return sKey_LeftShift;
	case VK_LCONTROL: return sKey_LeftCtrl;
	case VK_LMENU: return sKey_LeftAlt;
	case VK_LWIN: return sKey_LeftSuper;
	case VK_RSHIFT: return sKey_RightShift;
	case VK_RCONTROL: return sKey_RightCtrl;
	case VK_RMENU: return sKey_RightAlt;
	case VK_RWIN: return sKey_RightSuper;
	case VK_APPS: return sKey_Menu;
	case '0': return sKey_0;
	case '1': return sKey_1;
	case '2': return sKey_2;
	case '3': return sKey_3;
	case '4': return sKey_4;
	case '5': return sKey_5;
	case '6': return sKey_6;
	case '7': return sKey_7;
	case '8': return sKey_8;
	case '9': return sKey_9;
	case 'A': return sKey_A;
	case 'B': return sKey_B;
	case 'C': return sKey_C;
	case 'D': return sKey_D;
	case 'E': return sKey_E;
	case 'F': return sKey_F;
	case 'G': return sKey_G;
	case 'H': return sKey_H;
	case 'I': return sKey_I;
	case 'J': return sKey_J;
	case 'K': return sKey_K;
	case 'L': return sKey_L;
	case 'M': return sKey_M;
	case 'N': return sKey_N;
	case 'O': return sKey_O;
	case 'P': return sKey_P;
	case 'Q': return sKey_Q;
	case 'R': return sKey_R;
	case 'S': return sKey_S;
	case 'T': return sKey_T;
	case 'U': return sKey_U;
	case 'V': return sKey_V;
	case 'W': return sKey_W;
	case 'X': return sKey_X;
	case 'Y': return sKey_Y;
	case 'Z': return sKey_Z;
	case VK_F1: return sKey_F1;
	case VK_F2: return sKey_F2;
	case VK_F3: return sKey_F3;
	case VK_F4: return sKey_F4;
	case VK_F5: return sKey_F5;
	case VK_F6: return sKey_F6;
	case VK_F7: return sKey_F7;
	case VK_F8: return sKey_F8;
	case VK_F9: return sKey_F9;
	case VK_F10: return sKey_F10;
	case VK_F11: return sKey_F11;
	case VK_F12: return sKey_F12;
	default: return sKey_None;
	}
}

static bool
is_window_focused(sWindow& window)
{
	return window.platform.handle == ::GetActiveWindow();
}

static void
enable_mouse_raw_motion(sWindow& window)
{
	if (window.rawMouseMotion)
		return;
	window.rawMouseMotion = true;

	const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window.platform.handle };

	if (!::RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		S_ASSERT(false && "Win32: Failed to register raw input device");
	}
}

static void
disable_mouse_raw_motion(sWindow& window)
{
	if (!window.rawMouseMotion)
		return;
	window.rawMouseMotion = false;

	const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };
	if (!::RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		S_ASSERT(false && "Win32: Failed to register raw input device");
	}
}

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined SEMPER_INCLUDE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
#endif

	sIO& io = GContext->IO;
	sWindow* window;
	if (msg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		window = reinterpret_cast<sWindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	else
	{
		LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		window = reinterpret_cast<sWindow*>(ptr);
	}

	switch (msg)
	{

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			RECT actual_rect;
			RECT client_rect;
			int actual_width = 0;
			int actual_height = 0;
			int client_width = 0;
			int client_height = 0;
			if (GetWindowRect(hWnd, &actual_rect))
			{
				actual_width = actual_rect.right - actual_rect.left;
				actual_height = actual_rect.bottom - actual_rect.top;
			}

			if (GetClientRect(hWnd, &client_rect))
			{
				client_width = client_rect.right - client_rect.left;
				client_height = client_rect.bottom - client_rect.top;
			}

			//if (window->width != cwidth || window->height != cheight)
			window->sizeChanged = true;

			window->width = actual_width;
			window->height = actual_height;
			window->client_width = client_width;
			window->client_height = client_height;

		}
		return 0;

	case WM_MOVING:
	{
		int horizontal_shift = get_horizontal_shift(window->platform.handle);
		RECT rect = *(RECT*)(lParam);
		window->xpos = rect.left + horizontal_shift;
		window->ypos = rect.top;
		break;
	}

	case WM_MOUSEMOVE:

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		window->platform.mouseHandle = window->platform.handle;
		if (!window->mouseTracked)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, window->platform.handle, 0 };
			::TrackMouseEvent(&tme);
			window->mouseTracked = true;
		}
		io.pushMousePosEvent((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSELEAVE:

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		if (window->platform.mouseHandle == window->platform.handle)
			window->platform.mouseHandle = NULL;
		window->mouseTracked = false;
		io.pushMousePosEvent(-FLT_MAX, -FLT_MAX);
		break;
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		if (window->mouseButtonsDown == 0 && ::GetCapture() == NULL)
			::SetCapture(window->platform.handle);
		window->mouseButtonsDown |= 1 << button;
		io.pushMouseButtonEvent(button, true);
		return 0;
	}

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		int button = 0;
		if (msg == WM_LBUTTONUP) { button = 0; }
		if (msg == WM_RBUTTONUP) { button = 1; }
		if (msg == WM_MBUTTONUP) { button = 2; }
		if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		window->mouseButtonsDown &= ~(1 << button);
		if (window->mouseButtonsDown == 0 && ::GetCapture() == window->platform.handle)
			::ReleaseCapture();
		io.pushMouseButtonEvent(button, false);
		return 0;
	}

	case WM_MOUSEWHEEL:
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif
		io.pushMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;
	case WM_MOUSEHWHEEL:
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif
		io.pushMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureKeyboard) break;
		#endif
		const bool is_key_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
		if (wParam < 256)
		{
			// submit modifiers
			sKeyModFlags key_mods =
				((is_vk_down(VK_CONTROL)) ? sKeyModFlags_Ctrl : 0) |
				((is_vk_down(VK_SHIFT)) ? sKeyModFlags_Shift : 0) |
				((is_vk_down(VK_MENU)) ? sKeyModFlags_Alt : 0) |
				((is_vk_down(VK_APPS)) ? sKeyModFlags_Super : 0);
			io.pushKeyModsEvent(key_mods);

			// obtain virtual key code
			// (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
			int vk = (int)wParam;
			if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
				vk = IM_VK_KEYPAD_ENTER;

			// submit key event
			const sKey key = VirtualKeyToSemperKey(vk);
			const int scancode = (int)LOBYTE(HIWORD(lParam));
			if (key != sKey_None)
				io.pushKeyEvent(key, is_key_down);

			// submit individual left/right modifier events
			if (vk == VK_SHIFT)
			{
				// Important: shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
				if (is_vk_down(VK_LSHIFT) == is_key_down) { io.pushKeyEvent(sKey_LeftShift, is_key_down); }
				if (is_vk_down(VK_RSHIFT) == is_key_down) { io.pushKeyEvent(sKey_RightShift, is_key_down); }
			}
			else if (vk == VK_CONTROL)
			{
				if (is_vk_down(VK_LCONTROL) == is_key_down) { io.pushKeyEvent(sKey_LeftCtrl, is_key_down); }
				if (is_vk_down(VK_RCONTROL) == is_key_down) { io.pushKeyEvent(sKey_RightCtrl, is_key_down); }
			}
			else if (vk == VK_MENU)
			{
				if (is_vk_down(VK_LMENU) == is_key_down) { io.pushKeyEvent(sKey_LeftAlt, is_key_down); }
				if (is_vk_down(VK_RMENU) == is_key_down) { io.pushKeyEvent(sKey_RightAlt, is_key_down); }
			}
		}
		return 0;
	}

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		io.pushFocusEvent(msg == WM_SETFOCUS);
		return 0;

	case WM_CHAR:

		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureKeyboard) break;
		#endif

		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.pushInputCharacterUTF16((unsigned short)wParam);
		return 0;

	case WM_DESTROY:
	{
		window->running = false;
		PostQuitMessage(0);
		break;
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


namespace Semper
{
	void
	process_windows_workarounds()
	{
		sIO& io = get_io();

		// Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
		if (is_key_down(sKey_LeftShift) && !is_vk_down(VK_LSHIFT))
			io.pushKeyEvent(sKey_LeftShift, false);
		if (is_key_down(sKey_RightShift) && !is_vk_down(VK_RSHIFT))
			io.pushKeyEvent(sKey_RightShift, false);

		// Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
		if (is_key_down(sKey_LeftSuper) && !is_vk_down(VK_LWIN))
			io.pushKeyEvent(sKey_LeftSuper, false);
		if (is_key_down(sKey_RightSuper) && !is_vk_down(VK_RWIN))
			io.pushKeyEvent(sKey_RightSuper, false);
	}

	sPlatformSpecifics
	initialize_platform(sWindow& window, int cwidth, int cheight)
	{

		sPlatformSpecifics specifics;

		specifics.wc = { 0 };
		specifics.wc.cbSize = sizeof(WNDCLASSEX);
		specifics.wc.style = CS_HREDRAW | CS_VREDRAW;
		specifics.wc.lpfnWndProc = HandleMsgSetup;
		specifics.wc.cbClsExtra = 0;
		specifics.wc.cbWndExtra = 0;
		specifics.wc.hInstance = GetModuleHandle(nullptr);
		specifics.wc.hIcon = nullptr;
		specifics.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		specifics.wc.hbrBackground = nullptr;
		specifics.wc.lpszMenuName = nullptr;
		specifics.wc.lpszClassName = "Semper Window";
		specifics.wc.hIconSm = nullptr;
		RegisterClassEx(&specifics.wc);

		// calculate window size based on desired client region size
		RECT wr;
		wr.left = 100;
		wr.right = cwidth + wr.left;
		wr.top = 100;
		wr.bottom = cheight + wr.top;
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

		// create window & get hWnd
		specifics.handle = CreateWindowA(
			"Semper Window",
			"Semper Sandbox",
			WS_OVERLAPPEDWINDOW,
			0, 0, wr.right - wr.left, wr.bottom - wr.top,
			nullptr,
			nullptr,
			GetModuleHandle(nullptr),
			&window // app data
		);

		window.width = wr.right - wr.left;
		window.height = wr.bottom - wr.top;

		ShowWindow(specifics.handle, SW_SHOWDEFAULT);

		return specifics;
	}

	void
	cleanup_platform(sWindow& window)
	{
		CoUninitialize();
		DestroyWindow(window.platform.handle);
		UnregisterClass(window.platform.wc.lpszClassName, window.platform.wc.hInstance);
		window.platform.handle = nullptr;
		window.platform.mouseHandle = nullptr;
		window.platform.wc = { 0 };
	}

	void
	process_platform_events(sWindow& window)
	{
		MSG msg = {};

		// while queue has messages, remove and dispatch them (but do not block on empty queue)
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// check for quit because peekmessage does not signal this via return val
			if (msg.message == WM_QUIT)
			{
				window.running = false;
				return;
			}

			// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void
	enable_platform_cursor(sWindow& window)
	{
		sIO& io = get_io();

		if (window.rawMouseMotion)
			disable_mouse_raw_motion(window);

		::ClipCursor(nullptr);

		::SetCursorPos((int)io.restoreCursorPos[0], (int)io.restoreCursorPos[1]);

		::ShowCursor(true);
		::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
	}

	void
	disable_platform_cursor(sWindow& window)
	{
		sIO& io = get_io();

		if (is_window_focused(window))
		{
			POINT pos;

			if (::GetCursorPos(&pos))
			{
				//::ScreenToClient(window.platform.handle, &pos);
				io.restoreCursorPos[0] = pos.x;
				io.restoreCursorPos[1] = pos.y;
			}
		}

		::ShowCursor(false);
		::SetCursor(NULL);

		{
			int width, height;

			RECT area;
			::GetClientRect(window.platform.handle, &area);

			POINT pos = { (int)area.right / 2.0, (int)area.bottom / 2.0 };

			// Store the new position so it can be recognized later
			io.lastCursorPos[0] = pos.x;
			io.lastCursorPos[1] = pos.y;

			::ClientToScreen(window.platform.handle, &pos);
			::SetCursorPos(pos.x, pos.y);
		}

		// update clip rect
		{
			RECT clipRect;
			::GetClientRect(window.platform.handle, &clipRect);
			::ClientToScreen(window.platform.handle, (POINT*)&clipRect.left);
			::ClientToScreen(window.platform.handle, (POINT*)&clipRect.right);
			::ClipCursor(&clipRect);
		}

		if (window.rawMouseMotion)
			enable_mouse_raw_motion(window);
	}
}