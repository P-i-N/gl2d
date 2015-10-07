#pragma once

#include <functional>
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

class application;

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

static const unsigned default_window_flags = window_flag::resizable;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct window
{
  application *app;
  window_id_t id;
  std::string title;
  int width, height;

  window(application *a, window_id_t win_id, const std::string &win_title, int win_width, int win_height, unsigned flags = default_window_flags)
    : app(a)
    , id(win_id)
    , title(win_title)
    , width(win_width)
    , height(win_height)
  {
      
  }

  virtual ~window() { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class key
{
  unknown = 0,
  enter = 13,
  tab = 8, backspace = 9,
  escape = 27,
  space = ' ',
  a = 'A', b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
  num_0 = '0', num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,
  numpad_0, numpad_1, numpad_2, numpad_3, numapad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
  up, right, down, left,
  insert, del,
  home, end,
  page_up, page_down,
  f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class mouse_button
{
  none = 0,
  left, right, middle,
  wheel_up, wheel_down,
  wheel_left, wheel_right,
  special_0, special_1,
  back, forward
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class event_type
{
  unknown = 0,
  tick,
  open, close, resize,
  key_down, key_up,
  mouse_down, mouse_up, mouse_move
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct event
{
  event_type type;
  const gl2d::window &window;
  double time;
  double delta;
  bool used = false;

  union
  {
    struct { unsigned mods; key key; char key_char; } keyboard;
    struct { int width, height; } resize;
    struct { int x, y, dx, dy; mouse_button button; } mouse;
  };

  event(event_type et, const gl2d::window &w, double t, double d)
    : type(et)
    , window(w)
    , time(t)
    , delta(d)
  {
    
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class application
{
public:
  application();
  virtual ~application();

  void run();

  window_id_t open_window(const std::string &title, int width, int height, unsigned flags = default_window_flags)
  {
    auto id = _next_id++;
    _windows[id] = std::make_unique<platform_window>(this, id, title, width, height, flags);
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

  typedef std::function<void(event &)> event_handler_t;

  void event_handler(const event_handler_t &handler) { _event_handler = handler; }
  const event_handler_t &event_handler() const { return _event_handler; }
  
  virtual void on_event(event &e) { }

private:
  
#ifdef _WIN32
  struct platform_window : window
  {
    HWND handle;
    HDC hdc;
    HGLRC hglrc;

    platform_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags = default_window_flags);
    virtual ~platform_window();
  };

  LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  WNDCLASS _window_class;
#endif

  bool _should_quit = false;
  unsigned _next_id = 1;

  typedef std::map<window_id_t, std::unique_ptr<platform_window>> windows_t;
  windows_t _windows;
  event_handler_t _event_handler;

  void tick()
  {
    double time = 0.0, delta = 0.0;

    for (auto &kvp : _windows)
    {
      event e(event_type::tick, *kvp.second, time, delta);
      on_event(e);

      if (!e.used && _event_handler != nullptr)
        _event_handler(e);
    }

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
inline application::platform_window::platform_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags)
  : window(a, win_id, title, width, height, flags)
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

	auto pf = ChoosePixelFormat(hdc, &pfd); 
	SetPixelFormat(hdc, pf, &pfd);
 
	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);
}

//---------------------------------------------------------------------------------------------------------------------
inline application::platform_window::~platform_window()
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
