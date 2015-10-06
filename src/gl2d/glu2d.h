#pragma once

#include <map>
#include <memory>
#include <thread>

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

typedef unsigned window_id_t;

struct window_flag
{
  enum 
  {
    none = 0,
    resizable = 1,
    fullscreen = 2
  };
};

static const unsigned default_flags = window_flag::resizable;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class application
{
public:
  application();
  ~application();

  void run();

  window_id_t open_window(const std::string &title, int width, int height, unsigned flags = default_flags)
  {
    auto id = _next_id++;
    _windows[id] = std::make_unique<window>(this, id, title, width, height, flags);

    return id;
  }

  bool close_window(window_id_t id)
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
    {
      _windows.erase(iter);
      _should_quit |= _windows.empty();      
      return true;
    }

    return false;
  }

private:
  struct basic_window
  {
    application *app;
    window_id_t id;

    basic_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags = default_flags)
      : app(a)
      , id(win_id)
    {
      
    }

    virtual ~basic_window() { }
  };

#ifdef _WIN32
  struct window : basic_window
  {
    HWND handle;
    HDC hdc;
    HGLRC hglrc;

    window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags = default_flags);
    virtual ~window();
  };

  LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  WNDCLASS _window_class;
#endif

  bool _should_quit = false;
  unsigned _next_id = 1;

  typedef std::map<window_id_t, std::unique_ptr<window>> windows_t;
  windows_t _windows;

  void tick()
  {
    std::this_thread::yield();
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

//---------------------------------------------------------------------------------------------------------------------
inline application::application()
{
  _window_class = { 0 };
  _window_class.lpfnWndProc = wnd_proc_shared;
  _window_class.hInstance = GetModuleHandle(nullptr);
  _window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
  _window_class.lpszClassName = _TEXT"gl2d_window";
  _window_class.style = CS_OWNDC;
  RegisterClass(&_window_class);
}

//---------------------------------------------------------------------------------------------------------------------
inline application::~application()
{
  UnregisterClass(_TEXT"gl2d_window", _window_class.hInstance);
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::run()
{
  MSG msg = { 0 };
  while (!_should_quit)
  {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
      tick();
  }
}

//---------------------------------------------------------------------------------------------------------------------
inline application::window::window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags)
  : basic_window(a, win_id, title, width, height, flags)
{
  handle = CreateWindow(app->_window_class.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
  SetWindowLongPtr(handle, GWL_USERDATA, reinterpret_cast<LONG>(app));

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

	int pf = ChoosePixelFormat(hdc, &pfd); 
	SetPixelFormat(hdc, pf, &pfd);
 
	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);
}

//---------------------------------------------------------------------------------------------------------------------
inline application::window::~window()
{
  wglDeleteContext(hglrc);
  DestroyWindow(handle);
}

//---------------------------------------------------------------------------------------------------------------------
inline LRESULT CALLBACK application::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  for (auto &kvp : _windows)
  {
    if (kvp.second->handle == hWnd)
    {
      switch (message)
      {
        case WM_CLOSE:
          close_window(kvp.first);
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
inline LRESULT CALLBACK application::wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  auto app = reinterpret_cast<application *>(GetWindowLongPtr(hWnd, GWL_USERDATA));
  return app ? app->wnd_proc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else

#endif

}
