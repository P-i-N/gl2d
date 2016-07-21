#ifndef __GLU2D_H__
#define __GLU2D_H__

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
  numpad_0, numpad_1, numpad_2, numpad_3, numpad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
  up, right, down, left,
  insert, del,
  home, end,
  page_up, page_down,
  f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
  ctrl, alt, shift,

  last
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class mouse_button
{
  unknown = 0,
  left, right, middle,
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
  mouse_down, mouse_up, mouse_move, mouse_wheel
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
    struct { bool down; key key; int key_char; } keyboard;
    struct { int width, height; } resize;
    struct { bool down; int x, y, dx, dy; mouse_button button; } mouse;
    struct { int dx, dy; } wheel;
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

class application;

struct window
{
  application *app;
  window_id_t id;
  std::string title;
  int width, height;
  context ctx;
  int mouse_x = 0, mouse_y = 0;
  int mouse_dx = 0, mouse_dy = 0;

  window(application *a, window_id_t win_id, const std::string &win_title, int win_width, int win_height, unsigned flags = default_window_flags)
    : app(a)
    , id(win_id)
    , title(win_title)
    , width(win_width)
    , height(win_height)
  {
      
  }

  virtual ~window() { }

  void fill_mouse_event(event &e)
  {
    e.mouse.x = mouse_x;
    e.mouse.y = mouse_y;
    e.mouse.dx = mouse_dx;
    e.mouse.dy = mouse_dy;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class application
{
public:
  application();
  virtual ~application();

  void run();

  float time() const { return _time; }

  float delta() const { return _delta; }

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

  ivec2 get_window_size(window_id_t id) const
  {
    auto iter = _windows.find(id);
    if (iter != _windows.end())
      return ivec2(iter->second->width, iter->second->height);

    return ivec2();
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
        current_context = &(_windows[_main_window_id]->ctx);
      else
        current_context = nullptr;

      _tick_handler(_delta);
    }

    for (auto &kvp : _windows)
    {
      auto &w = *kvp.second;

      w.make_current();
      current_context = &(w.ctx);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      send({ event_type::render, w.id, _time, _delta });
      w.ctx.render(w.width, w.height);
      w.flip();
    }

    current_context = nullptr;
  }
};

}

// Global application instance
extern detail::application app;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif // __GLU2D_H__

#ifdef GL2D_IMPLEMENTATION
#ifndef __GLU2D_H_IMPL__
#define __GLU2D_H_IMPL__

namespace gl2d {

static detail::application app;

namespace detail {

#ifdef _WIN32

//---------------------------------------------------------------------------------------------------------------------
application::application()
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
application::~application()
{
  UnregisterClass(TEXT("gl2d_window"), _window_class.hInstance);
}

//---------------------------------------------------------------------------------------------------------------------
void application::run()
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
application::platform_window::platform_window(application *a, window_id_t win_id, const std::string &title, int width, int height, unsigned flags)
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
application::platform_window::~platform_window()
{
  wglDeleteContext(hglrc);
  DestroyWindow(handle);
}

//---------------------------------------------------------------------------------------------------------------------
void application::platform_window::make_current()
{
  wglMakeCurrent(hdc, hglrc);
}

//---------------------------------------------------------------------------------------------------------------------
void application::platform_window::flip()
{
  SwapBuffers(hdc);
}

//---------------------------------------------------------------------------------------------------------------------
void application::platform_window::set_title(const std::string &text)
{
  SetWindowTextA(handle, text.c_str());
}

//---------------------------------------------------------------------------------------------------------------------
void application::platform_window::set_size(int w, int h)
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
void application::update_timer()
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  li.QuadPart -= _timer_offset;

  _time = static_cast<float>(static_cast<double>(li.QuadPart) / static_cast<double>(_timer_frequency));
  _delta = static_cast<float>(static_cast<double>(li.QuadPart - _last_timer_counter) / static_cast<double>(_timer_frequency));
  _last_timer_counter = li.QuadPart;
}

//---------------------------------------------------------------------------------------------------------------------
key vk_to_key(WPARAM vk)
{
  static const std::map<unsigned, key> vkToKeyMap =
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
mouse_button xbutton_to_mouse_button(WPARAM xb)
{
  auto lo = LOWORD(xb);

  if (lo == 32)
    return mouse_button::back;
  
  if (lo == 64)
    return mouse_button::forward;

  return mouse_button::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK application::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
          e.mouse.down = false;
          e.mouse.button = mouse_button::unknown;
          e.mouse.x = GET_X_LPARAM(lParam);
          e.mouse.y = GET_Y_LPARAM(lParam);
          kvp.second->mouse_dx = e.mouse.dx = e.mouse.x - kvp.second->mouse_x;
          kvp.second->mouse_dy = e.mouse.dy = e.mouse.y - kvp.second->mouse_y;
          kvp.second->mouse_x = e.mouse.x;
          kvp.second->mouse_y = e.mouse.y;
          send(e);
        }
        return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
          event e(event_type::mouse_down, kvp.second->id, _time, _delta);
          e.mouse.down = true;
          if (message == WM_LBUTTONDOWN)
            e.mouse.button = mouse_button::left;
          else if (message == WM_RBUTTONDOWN)
            e.mouse.button = mouse_button::right;
          else if (message == WM_MBUTTONDOWN)
            e.mouse.button = mouse_button::middle;

          kvp.second->fill_mouse_event(e);
          send(e);
        }
        return 0;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
          event e(event_type::mouse_up, kvp.second->id, _time, _delta);
          e.mouse.down = false;
          if (message == WM_LBUTTONUP)
            e.mouse.button = mouse_button::left;
          else if (message == WM_RBUTTONUP)
            e.mouse.button = mouse_button::right;
          else if (message == WM_MBUTTONUP)
            e.mouse.button = mouse_button::middle;

          kvp.second->fill_mouse_event(e);
          send(e);
        }
        return 0;

        case WM_XBUTTONDOWN:
        {
          auto button = xbutton_to_mouse_button(wParam);
          if (button != mouse_button::unknown)
          {
            event e(event_type::mouse_down, kvp.second->id, _time, _delta);
            e.mouse.down = true;
            e.mouse.button = button;
            kvp.second->fill_mouse_event(e);
            send(e);
          }
        }
        return 0;

        case WM_XBUTTONUP:
        {
          auto button = xbutton_to_mouse_button(wParam);
          if (button != mouse_button::unknown)
          {
            event e(event_type::mouse_up, kvp.second->id, _time, _delta);
            e.mouse.down = false;
            e.mouse.button = button;
            kvp.second->fill_mouse_event(e);
            send(e);
          }
        }
        return 0;

        case WM_MOUSEWHEEL:
        {
          event e(event_type::mouse_wheel, kvp.second->id, _time, _delta);
          e.wheel.dx = 0;
          e.wheel.dy = GET_WHEEL_DELTA_WPARAM(wParam);
          send(e);
        }
        return 0;

        case WM_MOUSEHWHEEL:
        {
          event e(event_type::mouse_wheel, kvp.second->id, _time, _delta);
          e.wheel.dx = GET_WHEEL_DELTA_WPARAM(wParam);
          e.wheel.dy = 0;
          send(e);
        }
        return 0;

        case WM_KEYDOWN:
        {
          bool isKeyPress = (lParam & (1 << 30)) != 0;
          if (!isKeyPress)
          {
            event e(event_type::key_down, kvp.second->id, _time, _delta);
            e.keyboard.down = true;
            e.keyboard.key = vk_to_key(wParam);
            e.keyboard.key_char = 0;
            send(e);
          }
        }
        return 0;

        case WM_KEYUP:
        {
          event e(event_type::key_up, kvp.second->id, _time, _delta);
          e.keyboard.down = false;
          e.keyboard.key = vk_to_key(wParam);
          e.keyboard.key_char = 0;
          send(e);
        }
        return 0;

        case WM_CHAR:
        {
          event e(event_type::key_press, kvp.second->id, _time, _delta);
          e.keyboard.down = true;
          e.keyboard.key = key::unknown;
          e.keyboard.key_char = wParam;
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
LRESULT CALLBACK application::wnd_proc_shared(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  auto app = reinterpret_cast<application *>(GetWindowLongPtr(hWnd, GWL_USERDATA));
  return app ? app->wnd_proc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else

#endif

}

}

#endif // __GLU2D_H_IMPL__
#endif // GL2D_IMPLEMENTATION
