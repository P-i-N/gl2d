#ifndef __GL3D_EVENTS_H__
#define __GL3D_EVENTS_H__

#define GL3D_MAX_GAMEPADS 8

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
class context2d;
class context3d;

//---------------------------------------------------------------------------------------------------------------------
typedef int window_id_t;
const window_id_t invalid_window_id = static_cast<window_id_t>(-1);
const window_id_t main_window_id = 0;

//---------------------------------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------------------------
enum class mouse_button
{
  unknown = 0,
  left, right, middle,
  special_0, special_1,
  back, forward,

  last
};

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_button
{
  unknown = 0,
  a, b, x, y,
  up, right, down, left,
  shoulder_left, shoulder_right,
  thumb_left, thumb_right,

  last
};

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_axis
{
  unknown = 0,
  thumb_left, thumb_right,
  trigger_left, trigger_right,

  last
};

//---------------------------------------------------------------------------------------------------------------------
enum class event_type
{
  unknown = 0,
  run, quit,
  paint,
  open, close, resize,
  key_down, key_up, key_press,
  mouse_down, mouse_up, mouse_move, mouse_wheel,
  gamepad_down, gamepad_up, gamepad_move, gamepad_connect, gamepad_disconnect
};

//---------------------------------------------------------------------------------------------------------------------
struct event
{
  bool canceled = false;
  event_type type;
  window_id_t window_id;

  union
  {
    struct { key key; int key_char; } keyboard;
    struct { int width, height; } resize;
    struct { int x, y, dx, dy; mouse_button button; } mouse;
    struct { int dx, dy; } wheel;
    struct { int port; float x, y, dx, dy; gamepad_button button; gamepad_axis axis; } gamepad;
  };

  event(event_type et, window_id_t id)
    : type(et)
    , window_id(id)
  {
    
  }
};

//---------------------------------------------------------------------------------------------------------------------
namespace detail { struct keyboard_state
{
  bool key_down[static_cast<size_t>(key::last)];
  bool operator[](key k) const { return key_down[static_cast<size_t>(k)]; }

  void change_key_state(key k, bool down, window_id_t id = invalid_window_id);
}; }

extern detail::keyboard_state keyboard;

//---------------------------------------------------------------------------------------------------------------------
namespace detail { struct mouse_state
{
  bool button_down[static_cast<size_t>(mouse_button::last)];
  bool operator[](mouse_button b) const { return button_down[static_cast<size_t>(b)]; }
  int x, y;

  void change_button_state(mouse_button b, bool down, int originX = 0, int originY = 0, window_id_t id = invalid_window_id);
}; }

extern detail::mouse_state mouse;

//---------------------------------------------------------------------------------------------------------------------
namespace detail { struct gamepad_state
{
  int port = -1;
  bool button_down[static_cast<size_t>(gamepad_button::last)];
  float axis_x[static_cast<size_t>(gamepad_axis::last)];
  float axis_y[static_cast<size_t>(gamepad_axis::last)];
  bool operator[](gamepad_button b) const { return button_down[static_cast<size_t>(b)]; }

  void change_button_state(gamepad_button b, bool down);
  void change_axis_state(gamepad_axis ax, float x, float y);

  static int allocate_port();
  static void release_port(int port);
}; }

extern detail::gamepad_state gamepad[GL3D_MAX_GAMEPADS];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
template <typename F> struct callback_list
{
  typedef std::function<F> function_t;

  struct callback_info
  {
    int priority = 0;
    function_t callback;
    bool operator<(const callback_info &rhs) const { return priority < rhs.priority; }
    bool operator==(const function_t &f) const { return callback == f; }
  };

  std::set<callback_info> callbacks;

  callback_list &operator+=(function_t &&f)
  {
    callback_info ci;
    ci.callback = f;
    callbacks.insert(callbacks.end(), ci);
    return *this;
  }

  callback_list &operator-=(function_t &&f)
  {
    callbacks.erase(std::find(callbacks.begin(), callbacks.end(), f));
    return *this;
  }

  template <typename... Args> void operator()(Args&&... args) const
  {
    for (auto &&ci : callbacks)
      ci.callback(args...);
  }
};

}

extern detail::callback_list<void()> on_tick;
extern detail::callback_list<void(event &)> on_event;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
namespace detail { struct current_state
{
  context2d *ctx2d = nullptr;
  context3d *ctx3d = nullptr;
  float time = 0.0f;
  float delta = 0.0f;
}; }

extern detail::current_state current;

}

#endif // __GL3D_EVENTS_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
#ifndef __GL3D_EVENTS_H_IMPL__
#define __GL3D_EVENTS_H_IMPL__

namespace gl3d {
  
static detail::keyboard_state keyboard;
static detail::mouse_state mouse;
static detail::gamepad_state gamepad[GL3D_MAX_GAMEPADS];
static detail::current_state current;

static decltype(on_tick) on_tick;
static decltype(on_event) on_event;

namespace detail {
  
//---------------------------------------------------------------------------------------------------------------------
void keyboard_state::change_key_state(key k, bool down, window_id_t id)
{
  bool old = (*this)[k];
  if (old != down)
  {
    key_down[static_cast<size_t>(k)] = down;
    event e(down ? event_type::key_down : event_type::key_up, id);
    e.keyboard.key = k;
    e.keyboard.key_char = 0;
    on_event(e);
  }
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_button_state(mouse_button b, bool down, int originX, int originY, window_id_t id)
{
  bool old = (*this)[b];
  if (old != down)
  {
    button_down[static_cast<size_t>(b)] = down;
    event e(down ? event_type::mouse_down : event_type::mouse_up, id);
    e.mouse.button = b;
    on_event(e);
  }
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_button_state(gamepad_button b, bool down)
{
  bool old = (*this)[b];
  if (old != down)
  {
    button_down[static_cast<size_t>(b)] = down;
    event e(down ? event_type::gamepad_down : event_type::gamepad_up, invalid_window_id);
    e.gamepad.port = port;
    e.gamepad.button = b;
    on_event(e);
  }
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_axis_state(gamepad_axis ax, float x, float y)
{
  float oldX = axis_x[static_cast<size_t>(ax)];
  float oldY = axis_y[static_cast<size_t>(ax)];
  if (oldX != x || oldY != y)
  {
    axis_x[static_cast<size_t>(ax)] = x;
    axis_y[static_cast<size_t>(ax)] = y;
    event e(event_type::gamepad_move, invalid_window_id);
    e.gamepad.port = port;
    e.gamepad.axis = ax;
    e.gamepad.x = x;
    e.gamepad.y = y;
    e.gamepad.dx = x - oldX;
    e.gamepad.dy = y - oldY;
    on_event(e);
  }
}

//---------------------------------------------------------------------------------------------------------------------
int gamepad_state::allocate_port()
{
  for (int i = 0; i < GL3D_MAX_GAMEPADS; ++i)
    if (gamepad[i].port < 0)
      return i;

  return -1;  
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::release_port(int port)
{
  if (port >= 0 && port < GL3D_MAX_GAMEPADS && gamepad[port].port == port)
    gamepad[port].port = -1;
}

}

}

#endif // __GL3D_EVENTS_H_IMPL__
#endif // GL3D_IMPLEMENTATION
