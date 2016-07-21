#pragma once

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl2d.h"

#if defined(WIN32)
#include <windowsx.h>
#else
#endif

namespace gl2d {

class application;

typedef int window_id_t;

const window_id_t invalid_window_id = static_cast<window_id_t>(-1);

struct window_flag
{
  enum 
  {
    none = 0,
    resizable = 1,
    fullscreen = 2,
    title = 4,
  };
};

static const unsigned default_window_flags = window_flag::resizable | window_flag::title;

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
  f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,

  last
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class mouse_button
{
  none = 0,
  left, right, middle,
  wheel_up, wheel_down,
  wheel_left, wheel_right,
  special_0, special_1,
  back, forward,

  last
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class event_type
{
  unknown = 0,
  render,
  open, close, resize,
  key_down, key_up, key_press,
  mouse_down, mouse_up, mouse_move
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct mod_flag
{
  enum
  {
    none = 0,
    alt = 1,
    control = 2,
    shift = 4,
    mouse_left = 8,
    mouse_right = 16,
  };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct event
{
  event_type type;
  window_id_t window_id;
  double time;
  double delta;
  unsigned mods = mod_flag::none;

  union
  {
    struct { key key; char key_char; } keyboard;
    struct { int width, height; } resize;
    struct { int x, y, dx, dy; mouse_button button; } mouse;
  };

  event(event_type et, window_id_t id, double t, double d)
    : type(et)
    , window_id(id)
    , time(t)
    , delta(d)
  {
    
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

struct window
{
  application *app;
  window_id_t id;
  std::string title;
  int width, height;
  context ctx;

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

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class application
{
public:
  application();
  virtual ~application();

  void run();

  window_id_t window_open(const std::string &title, int width, int height, unsigned flags = default_window_flags)
  {
    auto id = _next_id++;
    auto window = std::make_unique<platform_window>(this, id, title, width, height, flags);

    if (!window->ctx.init())
    {
      window_close(id);
      return invalid_window_id;
    }

    if (!_main_window_id)
      _main_window_id = id;

    _windows[id] = std::move(window);
    return id;
  }

  bool window_close(window_id_t id)
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
    {
      send({ event_type::close, iter->second->id, _time, _delta });
      _windows.erase(iter);
      _should_quit |= _windows.empty();

      if (id == _main_window_id)
        _main_window_id = 0;

      return true;
    }

    return false;
  }

  void set_window_title(window_id_t id, const std::string &text)
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
      iter->second->set_title(text);
  }

  const std::string &window_title(window_id_t id) const
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
      return iter->second->title;

    static const std::string empty_title = "";
    return empty_title;
  }

  void set_window_size(window_id_t id, int width, int height)
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
      iter->second->set_size(width, height);
  }

  typedef std::function<void(const event &)> event_handler_t;

  void set_event_handler(const event_handler_t &handler) { _event_handler = handler; }
  const event_handler_t &event_handler() const { return _event_handler; }

  typedef std::function<void(float)> tick_handler_t;

  void set_tick_handler(const tick_handler_t &handler) { _tick_handler = handler; }
  const tick_handler_t &tick_handler() const { return _tick_handler; }
  
  gl2d::context *get_window_context(window_id_t id) const
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
      return &(iter->second->ctx);

    return nullptr;
  }

protected:
  virtual void on_event(const event &e) { }
  
private:
#ifdef _WIN32
  struct platform_window : detail::window
  {
    HWND handle;
    HDC hdc;
    HGLRC hglrc;
    DWORD style;

    platform_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags = default_window_flags);
    virtual ~platform_window();

    void make_current();
    void flip();
    void set_title(const std::string &text);
    void set_size(int w, int h);
  };

  LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  
  WNDCLASS _window_class;

  uint64_t _timer_offset = 0;
  uint64_t _timer_frequency = 0;
  uint64_t _last_timer_counter = 0;
#endif

  bool _should_quit = false;
  unsigned _next_id = 1;

  float _time = 0.0;
  float _delta = 0.0;

  typedef std::map<window_id_t, std::unique_ptr<platform_window>> windows_t;
  windows_t _windows;
  window_id_t _main_window_id = 0;
  event_handler_t _event_handler;
  tick_handler_t _tick_handler;

  void update_timer();

  void send(const event &e)
  {
    on_event(e);
    if (_event_handler != nullptr)
      _event_handler(e);
  }

  void tick()
  {
    update_timer();

    if (_tick_handler != nullptr)
    {
      if (_main_window_id)
        gl2d::context::current = &(_windows[_main_window_id]->ctx);
      else
        gl2d::context::current = nullptr;

      _tick_handler(_delta);
    }

    for (auto &kvp : _windows)
    {
      auto &w = *kvp.second;

      w.make_current();
      gl2d::context::current = &(w.ctx);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      send({ event_type::render, w.id, _time, _delta });
      w.ctx.render(w.width, w.height);
      w.flip();
    }

    gl2d::context::current = nullptr;
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
  _window_class.hbrBackground = GetStockBrush(BLACK_BRUSH);
  _window_class.lpszClassName = TEXT("gl2d_window");
  _window_class.style = CS_OWNDC;
  _window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&_window_class);
}

//---------------------------------------------------------------------------------------------------------------------
inline application::~application()
{
  UnregisterClass(TEXT("gl2d_window"), _window_class.hInstance);
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::run()
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  _timer_offset = li.QuadPart;

  QueryPerformanceFrequency(&li);
  _timer_frequency = li.QuadPart;

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

    std::this_thread::yield();
  }
}

//---------------------------------------------------------------------------------------------------------------------
inline application::platform_window::platform_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags)
  : window(a, win_id, title, width, height, flags)
{
  style =  WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  RECT adjustedRect;
  adjustedRect.top = adjustedRect.left = 0;
  adjustedRect.right = width;
  adjustedRect.bottom = height;
  AdjustWindowRectEx(&adjustedRect, style & ~WS_OVERLAPPED, FALSE, 0);

  handle = CreateWindow(app->_window_class.lpszClassName, title.c_str(), style, 0, 0, adjustedRect.right - adjustedRect.left, adjustedRect.bottom - adjustedRect.top, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
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
  make_current();
}

//---------------------------------------------------------------------------------------------------------------------
inline application::platform_window::~platform_window()
{
  wglDeleteContext(hglrc);
  DestroyWindow(handle);
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::platform_window::make_current()
{
  wglMakeCurrent(hdc, hglrc);
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::platform_window::flip()
{
  SwapBuffers(hdc);
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::platform_window::set_title(const std::string &text)
{
  SetWindowTextA(handle, text.c_str());
}

//---------------------------------------------------------------------------------------------------------------------
inline void application::platform_window::set_size(int w, int h)
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
inline void application::update_timer()
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  li.QuadPart -= _timer_offset;

  _time = static_cast<float>(static_cast<double>(li.QuadPart) / static_cast<double>(_timer_frequency));
  _delta = static_cast<float>(static_cast<double>(li.QuadPart - _last_timer_counter) / static_cast<double>(_timer_frequency));
  _last_timer_counter = li.QuadPart;
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
        case WM_MOUSEMOVE:
        {
          event e(event_type::mouse_move, kvp.second->id, _time, _delta);
          e.mouse.x = GET_X_LPARAM(lParam);
          e.mouse.y = GET_Y_LPARAM(lParam);
          send(e);
        }
        return 0;

        case WM_SIZE:
        {
          event e(event_type::resize, kvp.second->id, _time, _delta);
          e.resize.width = LOWORD(lParam);
          e.resize.height = HIWORD(lParam);
          send(e);
          kvp.second->width = LOWORD(lParam);
          kvp.second->height = HIWORD(lParam);
        }
        break;

        case WM_SIZING:
          tick();
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
inline LRESULT CALLBACK application::wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  auto app = reinterpret_cast<application *>(GetWindowLongPtr(hWnd, GWL_USERDATA));
  return app ? app->wnd_proc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else

#endif

}
