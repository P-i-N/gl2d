#ifndef __GL3D_WIN32_H__
#define __GL3D_WIN32_H__

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl3d_2d.h"
#include "gl3d_input.h"

namespace gl3d {

window_id_t window_open(const std::string &title, int width, int height, unsigned flags = default_window_flags);

bool window_close(window_id_t id = main_window_id);

void set_window_title(const std::string &text, window_id_t id = main_window_id);

const std::string &window_title(window_id_t id = main_window_id);

void set_window_size(int width, int height, window_id_t = main_window_id);

ivec2 get_window_size(window_id_t id = main_window_id);

inline ivec2 get_window_center(window_id_t id = main_window_id) { return get_window_size(id) / 2; }

void run();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif // __GL3D_WIN32_H__

#ifdef GL3D_IMPLEMENTATION
#ifndef __GL3D_WIN32_H_IMPL__
#define __GL3D_WIN32_H_IMPL__

#if defined(WIN32)
#include <windowsx.h>
#include <hidsdi.h>
#include <Xinput.h>
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "xinput.lib")
#else
#endif

#define GL3D_WINDOW_CLASS "gl3d_window"

namespace gl3d {

bool g_should_quit = false;
window_id_t g_next_id = 0;

uint64_t g_timer_offset = 0;
uint64_t g_timer_frequency = 0;
uint64_t g_last_timer_counter = 0;

//---------------------------------------------------------------------------------------------------------------------
struct window
{
	window_id_t id;
	std::string title;
	HWND handle;
	HDC hdc;
	HGLRC hglrc;
	DWORD style;
	int width, height;
	context2d ctx2d;
	context3d ctx3d;
	int mouse_x = 0, mouse_y = 0;
	int mouse_dx = 0, mouse_dy = 0;

	window(window_id_t win_id, const std::string &win_title, int win_width, int win_height, unsigned flags = default_window_flags);

	virtual ~window();

	void make_current();
	void flip();
	void set_title(const std::string &text);
	void set_size(int w, int h);

	void fill_mouse_event(event &e)
	{
		e.mouse.x = mouse_x;
		e.mouse.y = mouse_y;
		e.mouse.dx = mouse_dx;
		e.mouse.dy = mouse_dy;
	}
};

//---------------------------------------------------------------------------------------------------------------------
typedef std::map<window_id_t, std::unique_ptr<window>> windows_t;
windows_t g_windows;

/* Forward declaration */
LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------------------------------------------------
struct window_class
{
	WNDCLASS wndclass;

	window_class()
	{
		wndclass = { 0 };
		wndclass.lpfnWndProc = wnd_proc;
		wndclass.hInstance = GetModuleHandle(nullptr);
		wndclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
		wndclass.lpszClassName = TEXT(GL3D_WINDOW_CLASS);
		wndclass.style = CS_OWNDC;
		wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		RegisterClass(&wndclass);
	}

	~window_class()
	{
		UnregisterClass(TEXT(GL3D_WINDOW_CLASS), wndclass.hInstance);
	}
} g_window_class;

//---------------------------------------------------------------------------------------------------------------------
void update_timer()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	li.QuadPart -= g_timer_offset;

	state.time = static_cast<float>(li.QuadPart / static_cast<double>(g_timer_frequency));
	state.delta = static_cast<float>((li.QuadPart - g_last_timer_counter) / static_cast<double>(g_timer_frequency));
	g_last_timer_counter = li.QuadPart;
}

//---------------------------------------------------------------------------------------------------------------------
std::map<int, int> g_xinput_port_map;

void update_xinput()
{
	for (int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		auto iter = g_xinput_port_map.find(i);
		int port = iter != g_xinput_port_map.end() ? iter->second : -1;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			if (port == -1)
			{
				port = detail::gamepad_state::allocate_port();
				g_xinput_port_map[i] = port;
				event e(event_type::gamepad_connect, invalid_window_id);
				e.gamepad.port = port;
				on_event.call(e);
			}
			else
				port = iter->second;

			auto &g = gamepad[port];

			g.change_button_state(gamepad_button::a, (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
			g.change_button_state(gamepad_button::b, (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
			g.change_button_state(gamepad_button::x, (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
			g.change_button_state(gamepad_button::y, (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
			g.change_button_state(gamepad_button::up, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
			g.change_button_state(gamepad_button::down, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
			g.change_button_state(gamepad_button::left, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
			g.change_button_state(gamepad_button::right, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
			g.change_button_state(gamepad_button::thumb_left, (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
			g.change_button_state(gamepad_button::thumb_right, (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);
			g.change_button_state(gamepad_button::shoulder_left, (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
			g.change_button_state(gamepad_button::shoulder_right, (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);

			g.change_axis_state(gamepad_axis::thumb_left, state.Gamepad.sThumbLX / 32767.0f, state.Gamepad.sThumbLY / 32767.0f);
			g.change_axis_state(gamepad_axis::thumb_right, state.Gamepad.sThumbRX / 32767.0f, state.Gamepad.sThumbRY / 32767.0f);
			g.change_axis_state(gamepad_axis::trigger_left, state.Gamepad.bLeftTrigger / 255.0f, 0.0f);
			g.change_axis_state(gamepad_axis::trigger_right, state.Gamepad.bRightTrigger / 255.0f, 0.0f);
		}
		else
		{
			if (port != -1)
			{
				event e(event_type::gamepad_disconnect, invalid_window_id);
				e.gamepad.port = port;
				on_event.call(e);
				detail::gamepad_state::release_port(port);
				g_xinput_port_map.erase(iter);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void update()
{
	update_timer();
	update_xinput();

	// Call global tick
	{
		auto iter = g_windows.find(main_window_id);
		if (iter != g_windows.end())
			state.ctx2d = &(iter->second->ctx2d);
		else
			state.ctx2d = nullptr;

		state.ctx3d = nullptr;
		state.current_window_id = main_window_id;
		on_tick.call();
	}

	for (auto &kvp : g_windows)
	{
		auto &w = *kvp.second;
		
		w.make_current();
		state.ctx2d = &(w.ctx2d);
		state.ctx3d = &(w.ctx3d);
		state.current_window_id = w.id;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		state.ctx3d->clear();
		on_event.call(event(event_type::paint, w.id));
		w.ctx2d.render(w.ctx3d, w.width, w.height);
		w.flip();
	}

	state.ctx2d = nullptr;
	state.ctx3d = nullptr;
	state.current_window_id = invalid_window_id;
}

//---------------------------------------------------------------------------------------------------------------------
window_id_t window_open(const std::string &title, int width, int height, unsigned flags)
{
	auto id = g_next_id++;
	auto w = std::make_unique<window>(id, title, width, height, flags);

	if (!w->ctx2d.init())
	{
		window_close(id);
		return invalid_window_id;
	}

	g_windows[id] = std::move(w);

	on_event.call(event(event_type::open, id));
	return id;
}

//---------------------------------------------------------------------------------------------------------------------
bool window_close(window_id_t id)
{
	auto iter = g_windows.find(id);
	if (iter != g_windows.end())
	{
		on_event.call(event(event_type::close, iter->second->id));
		g_windows.erase(iter);
		g_should_quit |= id == main_window_id || g_windows.empty();
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------------------------------------
void set_window_title(const std::string &text, window_id_t id)
{
	auto iter = g_windows.find(id);
	if (iter != g_windows.end())
		iter->second->set_title(text);
}

//---------------------------------------------------------------------------------------------------------------------
const std::string &window_title(window_id_t id)
{
	auto iter = g_windows.find(id);
	if (iter != g_windows.end())
		return iter->second->title;

	static const std::string empty_title = "";
	return empty_title;
}

//---------------------------------------------------------------------------------------------------------------------
void set_window_size(int width, int height, window_id_t id)
{
	auto iter = g_windows.find(id);
	if (iter != g_windows.end())
		iter->second->set_size(width, height);
}

//---------------------------------------------------------------------------------------------------------------------
ivec2 get_window_size(window_id_t id)
{
	auto iter = g_windows.find(id);
	if (iter != g_windows.end())
		return ivec2(iter->second->width, iter->second->height);

	return ivec2();
}

//---------------------------------------------------------------------------------------------------------------------
window::window(window_id_t win_id, const std::string &title, int width, int height, unsigned flags)
	: id(win_id)
	, title(title)
	, width(width)
	, height(height)
{
	style =  WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	RECT adjustedRect;
	adjustedRect.top = adjustedRect.left = 0;
	adjustedRect.right = width;
	adjustedRect.bottom = height;
	AdjustWindowRectEx(&adjustedRect, style & ~WS_OVERLAPPED, FALSE, 0);

	handle = CreateWindow(TEXT(GL3D_WINDOW_CLASS), title.c_str(), style, 0, 0, adjustedRect.right - adjustedRect.left, adjustedRect.bottom - adjustedRect.top, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	if (win_id == main_window_id)
	{
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 1;
		rid.usUsage = 5;
		rid.dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
		rid.hwndTarget = handle;
		RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		24, 8,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
 
	hdc = GetDC(handle);

	auto pf = ChoosePixelFormat(hdc, &pfd); 
	SetPixelFormat(hdc, pf, &pfd);
 
	hglrc = wglCreateContext(hdc);
	make_current();

	ctx2d.init();
}

//---------------------------------------------------------------------------------------------------------------------
window::~window()
{
	wglDeleteContext(hglrc);
	DestroyWindow(handle);
}

//---------------------------------------------------------------------------------------------------------------------
void window::make_current()
{
	if (wglGetCurrentContext() != hglrc)
		wglMakeCurrent(hdc, hglrc);
}

//---------------------------------------------------------------------------------------------------------------------
void window::flip()
{
	SwapBuffers(hdc);
}

//---------------------------------------------------------------------------------------------------------------------
void window::set_title(const std::string &text)
{
	SetWindowTextA(handle, text.c_str());
}

//---------------------------------------------------------------------------------------------------------------------
void window::set_size(int w, int h)
{
	if (width != w || height != h)
	{
		width = w;
		height = h;
		RECT rect;
		GetClientRect(handle, &rect);
		int x = rect.left;
		int y = rect.top;
		rect.right = rect.left + w;
		rect.bottom = rect.top + h;
		AdjustWindowRectEx(&rect, style & ~WS_OVERLAPPED, FALSE, 0);
		SetWindowPos(handle, handle, x, y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOREPOSITION);
	}
}

//---------------------------------------------------------------------------------------------------------------------
key vk_to_key(WPARAM vk)
{
	static const std::map<WPARAM, key> vkToKeyMap =
	{
		{ VK_RETURN, key::enter }, { VK_TAB, key::tab }, { VK_BACK, key::backspace }, { VK_ESCAPE, key::escape }, { VK_SPACE, key::space },
		{ 0x41, key::a }, { 0x42, key::b }, { 0x43, key::c }, { 0x44, key::d }, { 0x45, key::e }, { 0x46, key::f }, { 0x47, key::g }, { 0x48, key::h },
		{ 0x49, key::i }, { 0x4a, key::j }, { 0x4b, key::k }, { 0x4c, key::l }, { 0x4d, key::m }, { 0x4e, key::n }, { 0x4f, key::o }, { 0x50, key::p },
		{ 0x51, key::q }, { 0x52, key::r }, { 0x53, key::s }, { 0x54, key::t }, { 0x55, key::u }, { 0x56, key::v }, { 0x57, key::w }, { 0x58, key::x },
		{ 0x59, key::y }, { 0x5a, key::z },
		{ 0x30, key::num_0 }, { 0x31, key::num_1 }, { 0x32, key::num_2 }, { 0x33, key::num_3 }, { 0x34, key::num_4 },
		{ 0x35, key::num_5 }, { 0x36, key::num_6 }, { 0x37, key::num_7 }, { 0x38, key::num_8 }, { 0x39, key::num_9 },
		{ VK_NUMPAD0, key::numpad_0 }, { VK_NUMPAD1, key::numpad_1 }, { VK_NUMPAD2, key::numpad_2 }, { VK_NUMPAD3, key::numpad_3 }, { VK_NUMPAD4, key::numpad_4 },
		{ VK_NUMPAD5, key::numpad_5 }, { VK_NUMPAD6, key::numpad_6 }, { VK_NUMPAD7, key::numpad_7 }, { VK_NUMPAD8, key::numpad_8 }, { VK_NUMPAD9, key::numpad_9 },
		{ VK_UP, key::up }, { VK_RIGHT, key::right }, { VK_DOWN, key::down }, { VK_LEFT, key::left },
		{ VK_INSERT, key::insert }, { VK_DELETE, key::del }, { VK_HOME, key::home }, { VK_END, key::end },
		{ VK_PRIOR, key::page_up }, { VK_NEXT, key::page_down },
		{ VK_F1, key::f1 }, { VK_F2, key::f2 }, { VK_F3, key::f3 }, { VK_F4, key::f4 }, { VK_F5, key::f5 }, { VK_F6, key::f6 },
		{ VK_F7, key::f7 }, { VK_F8, key::f8 }, { VK_F9, key::f9 }, { VK_F10, key::f10 }, { VK_F11, key::f11 }, { VK_F12, key::f12 },
		{ VK_CONTROL, key::ctrl }, { VK_MENU, key::alt }, { VK_SHIFT, key::shift }
	};

	auto iter = vkToKeyMap.find(vk);
	if (iter != vkToKeyMap.end())
		return iter->second;

	return key::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
mouse_button mbutton_to_mouse_button(UINT msg)
{
	if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
		return mouse_button::left;
	if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
		return mouse_button::right;
	if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
		return mouse_button::middle;

	return mouse_button::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
mouse_button xbutton_to_mouse_button(WPARAM xb)
{
	auto lo = LOWORD(xb);
	if (lo == 32) return mouse_button::back;
	if (lo == 64) return mouse_button::forward;

	return mouse_button::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> g_raw_input_buffer;

void parse_raw_input(RAWINPUT *raw)
{
	// We can use statics here, this should be called from one thread only anyway
	static std::vector<uint8_t> preparsedDataBuffer;
	static std::vector<uint8_t> buttonCapsBuffer;
	static std::vector<uint8_t> valueCapsBuffer;
	static std::vector<USAGE> usages;
	UINT bufferSize;

	if (GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, nullptr, &bufferSize)) return;
	if (!bufferSize) return;
	preparsedDataBuffer.resize(bufferSize);
	GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, preparsedDataBuffer.data(), &bufferSize);
	PHIDP_PREPARSED_DATA preparsedData = reinterpret_cast<PHIDP_PREPARSED_DATA>(preparsedDataBuffer.data());
	HIDP_CAPS caps;
	HidP_GetCaps(preparsedData, &caps);

	buttonCapsBuffer.resize(sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps);
	PHIDP_BUTTON_CAPS buttonCaps = reinterpret_cast<PHIDP_BUTTON_CAPS>(buttonCapsBuffer.data());
	HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, preparsedData);

	valueCapsBuffer.resize(sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
	PHIDP_VALUE_CAPS valueCaps = reinterpret_cast<PHIDP_VALUE_CAPS>(valueCapsBuffer.data());
	HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, preparsedData);

	// Check buttons
	ULONG numButtons = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
	usages.resize(numButtons);
	HidP_GetUsages(HidP_Input,
		buttonCaps->UsagePage, 0, usages.data(), &numButtons, preparsedData,
		reinterpret_cast<PCHAR>(raw->data.hid.bRawData), raw->data.hid.dwSizeHid);

	for (size_t i = 0; i < usages.size(); ++i)
	{
		// TODO
		int index = usages[i];// - buttonCaps->Range.UsageMin;
	}
}

//---------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	for (auto &kvp : g_windows)
	{
		if (kvp.second->handle == hWnd)
		{
			switch (message)
			{
				case WM_INPUT:
				{
					UINT bufferSize;
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &bufferSize, sizeof(RAWINPUTHEADER));
					g_raw_input_buffer.resize(bufferSize);
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, g_raw_input_buffer.data(), &bufferSize, sizeof(RAWINPUTHEADER));
					parse_raw_input(reinterpret_cast<RAWINPUT *>(g_raw_input_buffer.data()));
				}
				return 0;

				case WM_MOUSEMOVE:
					mouse.change_position(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), kvp.second->id);
					return 0;

				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
					mouse.change_button_state(mbutton_to_mouse_button(message), true, kvp.second->id);
					return 0;

				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
					mouse.change_button_state(mbutton_to_mouse_button(message), false, kvp.second->id);
					return 0;

				case WM_XBUTTONDOWN:
				{
					auto button = xbutton_to_mouse_button(wParam);
					if (button != mouse_button::unknown)
						mouse.change_button_state(button, true, kvp.second->id);
				}
				return 0;

				case WM_XBUTTONUP:
				{
					auto button = xbutton_to_mouse_button(wParam);
					if (button != mouse_button::unknown)
						mouse.change_button_state(button, false, kvp.second->id);
				}
				return 0;

				case WM_MOUSEWHEEL:
				{
					event e(event_type::mouse_wheel, kvp.second->id);
					e.wheel.dx = 0;
					e.wheel.dy = GET_WHEEL_DELTA_WPARAM(wParam);
					on_event.call(e);
				}
				return 0;

				case WM_MOUSEHWHEEL:
				{
					event e(event_type::mouse_wheel, kvp.second->id);
					e.wheel.dx = GET_WHEEL_DELTA_WPARAM(wParam);
					e.wheel.dy = 0;
					on_event.call(e);
				}
				return 0;

				case WM_KEYDOWN:
				{
					bool isKeyPress = (lParam & (1 << 30)) != 0;
					if (!isKeyPress)
						keyboard.change_key_state(vk_to_key(wParam), true, kvp.second->id);
				}
				return 0;

				case WM_KEYUP:
					keyboard.change_key_state(vk_to_key(wParam), false, kvp.second->id);
					return 0;

				case WM_CHAR:
				{
					event e(event_type::key_press, kvp.second->id);
					e.keyboard.key = key::unknown;
					e.keyboard.key_char = static_cast<int>(wParam);
					on_event.call(e);
				}
				return 0;

				case WM_SIZE:
				{
					event e(event_type::resize, kvp.second->id);
					e.resize.width = LOWORD(lParam);
					e.resize.height = HIWORD(lParam);
					on_event.call(e);
					kvp.second->width = LOWORD(lParam);
					kvp.second->height = HIWORD(lParam);
				}
				break;

				case WM_SIZING:
					update();
					break;

				case WM_CLOSE:
					window_close(kvp.first);
					return 0;

				default:
					break;
			}
			
			break;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------------------------------------------------
void run()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	g_timer_offset = li.QuadPart;

	QueryPerformanceFrequency(&li);
	g_timer_frequency = li.QuadPart;

	while (true)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		if (!g_should_quit)
		{
			update();
			std::this_thread::yield();
		}
		else
			break;
	}
}

}

#endif // __GLU3D_WIN32_H_IMPL__
#endif // GL2D_IMPLEMENTATION
