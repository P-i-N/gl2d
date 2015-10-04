#pragma once

#include <memory>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#ifdef UNICODE
#define _TEXT L
#else
#define _TEXT
#endif
#else
#endif

namespace gl2d {

class application;

class window : public std::enable_shared_from_this<window>
{
  friend class application;
  struct protection_tag { };
  application &_app;

public:
  enum flag
  {
    none = 0,
    resizable = 1,
    fullscreen = 2
  };

  static const unsigned default_flags = resizable;

  window(const protection_tag &tag, application &app, const std::string &title, int width, int height, unsigned flags = default_flags);

  virtual ~window();

  void close();

#ifdef _WIN32
  HWND handle() const { return _hWnd; }

private:
  int wnd_proc(UINT message, WPARAM wParam, LPARAM lParam);

  HWND _hWnd;
#else
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class application
{
public:
  application()
  {
#ifdef _WIN32
    _window_class = { 0 };
    _window_class.lpfnWndProc = wnd_proc_shared;
    _window_class.hInstance = GetModuleHandle(nullptr);
    _window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
    _window_class.lpszClassName = _TEXT"gl2d_window";
    _window_class.style = CS_OWNDC;
    RegisterClass(&_window_class);
#else

#endif
  }

  virtual ~application()
  {
#ifdef _WIN32
    UnregisterClass(_TEXT"gl2d_window", _window_class.hInstance);
#else
#endif
  }

  typedef std::vector<std::shared_ptr<window>> windows_t;

  windows_t &windows() { return _windows; }

  const windows_t &windows() const { return _windows; }

  std::shared_ptr<window> open_window(const std::string &title, int width, int height, unsigned flags = window::default_flags)
  {
    auto result = std::make_shared<window>(window::protection_tag(), *this, title, width, height, flags);
    _windows.push_back(result);

    return result;
  }

  bool close_window(const std::shared_ptr<window> ptr)
  {
    auto iter = std::find(_windows.begin(), _windows.end(), ptr);

    if (iter != _windows.end())
    {
      _windows.erase(iter);
      return true;
    }

    return false;
  }

  void run()
  {
#ifdef _WIN32
  MSG msg = { 0 };

	while (GetMessage(&msg, nullptr, 0, 0) > 0)
		DispatchMessage(&msg);
#else
#endif
  }

private:
  friend class window;

  bool _should_quit = false;

  windows_t _windows;

#ifdef _WIN32
  LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    for (auto w : _windows)
      if (w->handle() == hWnd)
        return w->wnd_proc(message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  static LRESULT CALLBACK wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    auto app = reinterpret_cast<application *>(GetWindowLongPtr(hWnd, GWL_USERDATA));
    return app ? app->wnd_proc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
  }

  WNDCLASS _window_class;
#else
#endif
};

inline window::window(const protection_tag &tag, application &app, const std::string &title, int width, int height, unsigned flags)
  : _app(app)
{
#ifdef _WIN32
  _hWnd = CreateWindow(_app._window_class.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
  SetWindowLongPtr(_hWnd, GWL_USERDATA, reinterpret_cast<LONG>(&app));

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
 
	HDC ourWindowHandleToDeviceContext = GetDC(_hWnd);

	int  letWindowsChooseThisPixelFormat;
	letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd); 
	SetPixelFormat(ourWindowHandleToDeviceContext,letWindowsChooseThisPixelFormat, &pfd);
 
	HGLRC ourOpenGLRenderingContext = wglCreateContext(ourWindowHandleToDeviceContext);
	wglMakeCurrent(ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);

#else
#endif
}

inline window::~window()
{

}

inline void window::close()
{
#ifdef _WIN32
  DestroyWindow(_hWnd);
#else
#endif
}

inline int window::wnd_proc(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_CLOSE:
      _app.close_window(shared_from_this());
      break;

    default:
      return DefWindowProc(_hWnd, message, wParam, lParam);
  }

  return 0;
}

}
