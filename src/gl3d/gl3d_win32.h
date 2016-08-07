#ifndef __GLU2D_H__
#define __GLU2D_H__

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl3d_2d.h"

#if defined(WIN32)
#include <windowsx.h>
#else
#endif

namespace gl3d {

typedef int window_id_t;
const window_id_t invalid_window_id = static_cast<window_id_t>(-1);
const window_id_t main_window_id = 0;

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
  paint,
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
  unsigned mods = mod_flag::none;

  union
  {
    struct { bool down; key key; int key_char; } keyboard;
    struct { int width, height; } resize;
    struct { bool down; int x, y, dx, dy; mouse_button button; } mouse;
    struct { int dx, dy; } wheel;
  };

  event(event_type et, window_id_t id)
    : type(et)
    , window_id(id)
  {
    
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern context2d *current_context2d;
extern context3d *current_context3d;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void run();

float time();
double timed();

float delta();
double deltad();

window_id_t window_open(const std::string &title, int width, int height, unsigned flags = default_window_flags);

bool window_close(window_id_t id = main_window_id);

void set_window_title(const std::string &text, window_id_t id = main_window_id);

const std::string &window_title(window_id_t id = main_window_id);

void set_window_size(int width, int height, window_id_t = main_window_id);

ivec2 get_window_size(window_id_t id = main_window_id);

typedef std::function<void(const event &)> event_handler_t;

extern event_handler_t event_handler;

typedef std::function<void()> tick_handler_t;

extern tick_handler_t tick_handler;
  
void send(const event &e);

void tick();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif // __GLU2D_H__

#ifdef GL3D_IMPLEMENTATION
#ifndef __GLU2D_H_IMPL__
#define __GLU2D_H_IMPL__

#define GL3D_WINDOW_CLASS "gl3d_window"

namespace gl3d {

static context2d *current_context2d = nullptr;
static context3d *current_context3d = nullptr;
static event_handler_t event_handler;
static tick_handler_t tick_handler;

bool g_should_quit = false;
window_id_t g_next_id = 0;

uint64_t g_timer_offset = 0;
uint64_t g_timer_frequency = 0;
uint64_t g_last_timer_counter = 0;

double g_time = 0.0;
double g_delta = 0.0;

//---------------------------------------------------------------------------------------------------------------------
float time() { return static_cast<float>(g_time); }
double timed() { return g_time; }
float delta() { return static_cast<float>(g_delta); }
double deltad() { return g_delta; }

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
      tick();
      std::this_thread::yield();
    }
    else
      break;
  }
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
  return id;
}

//---------------------------------------------------------------------------------------------------------------------
bool window_close(window_id_t id)
{
  auto iter = g_windows.find(id);
  if (iter != g_windows.end())
  {
    send({ event_type::close, iter->second->id });
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
void update_timer()
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  li.QuadPart -= g_timer_offset;

  g_time = static_cast<double>(li.QuadPart) / static_cast<double>(g_timer_frequency);
  g_delta = static_cast<double>(li.QuadPart - g_last_timer_counter) / static_cast<double>(g_timer_frequency);
  g_last_timer_counter = li.QuadPart;
}

//---------------------------------------------------------------------------------------------------------------------
void send(const event &e)
{
  if (event_handler != nullptr)
    event_handler(e); 
}

//---------------------------------------------------------------------------------------------------------------------
void tick()
{
  update_timer();

  if (tick_handler != nullptr)
  {
    auto iter = g_windows.find(main_window_id);
    if (iter != g_windows.end())
      current_context2d = &(iter->second->ctx2d);
    else
      current_context2d = nullptr;

    current_context3d = nullptr;
    tick_handler();
  }

  for (auto &kvp : g_windows)
  {
    auto &w = *kvp.second;
    
    w.make_current();
    current_context2d = &(w.ctx2d);
    current_context3d = &(w.ctx3d);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    current_context3d->clear();
    send({ event_type::paint, w.id });
    w.ctx2d.render(w.width, w.height);
    w.flip();
  }

  current_context2d = nullptr;
  current_context3d = nullptr;
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
mouse_button xbutton_to_mouse_button(WPARAM xb)
{
  auto lo = LOWORD(xb);
  if (lo == 32) return mouse_button::back;
  if (lo == 64) return mouse_button::forward;

  return mouse_button::unknown;
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
        case WM_MOUSEMOVE:
        {
          event e(event_type::mouse_move, kvp.second->id);
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
          event e(event_type::mouse_down, kvp.second->id);
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
          event e(event_type::mouse_up, kvp.second->id);
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
            event e(event_type::mouse_down, kvp.second->id);
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
            event e(event_type::mouse_up, kvp.second->id);
            e.mouse.down = false;
            e.mouse.button = button;
            kvp.second->fill_mouse_event(e);
            send(e);
          }
        }
        return 0;

        case WM_MOUSEWHEEL:
        {
          event e(event_type::mouse_wheel, kvp.second->id);
          e.wheel.dx = 0;
          e.wheel.dy = GET_WHEEL_DELTA_WPARAM(wParam);
          send(e);
        }
        return 0;

        case WM_MOUSEHWHEEL:
        {
          event e(event_type::mouse_wheel, kvp.second->id);
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
            event e(event_type::key_down, kvp.second->id);
            e.keyboard.down = true;
            e.keyboard.key = vk_to_key(wParam);
            e.keyboard.key_char = 0;
            send(e);
          }
        }
        return 0;

        case WM_KEYUP:
        {
          event e(event_type::key_up, kvp.second->id);
          e.keyboard.down = false;
          e.keyboard.key = vk_to_key(wParam);
          e.keyboard.key_char = 0;
          send(e);
        }
        return 0;

        case WM_CHAR:
        {
          event e(event_type::key_press, kvp.second->id);
          e.keyboard.down = true;
          e.keyboard.key = key::unknown;
          e.keyboard.key_char = static_cast<int>(wParam);
          send(e);
        }
        return 0;

        case WM_SIZE:
        {
          event e(event_type::resize, kvp.second->id);
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

}

#endif // __GLU2D_H_IMPL__
#endif // GL2D_IMPLEMENTATION
