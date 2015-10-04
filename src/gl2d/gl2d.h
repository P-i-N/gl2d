#pragma once

#include <windows.h>
#include <gl/GL.h>

namespace gl2d {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vec2
{
  float x = 0.0f, y = 0.0f;

  vec2() { }

  vec2(const vec2 &copy)
    : x(copy.x)
    , y(copy.y)
  {
    
  }

  vec2(float _x, float _y)
    : x(_x)
    , y(_y)
  {
    
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct rect
{
  vec2 min, max;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct color
{
  float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;

  color() { }

  color(const color &copy)
    : r(copy.r)
    , g(copy.g)
    , b(copy.b)
    , a(copy.a)
  {
    
  }

  color(float _r, float _g, float _b, float _a = 1.0f)
    : r(_r)
    , g(_g)
    , b(_b)
    , a(_a)
  {
    
  }
};

namespace detail {
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GL2D_API_FUNC(retValue, name, ...) \
  typedef retValue(*gl_ ## name ## _ptr_t)(__VA_ARGS__); \
  gl_ ## name ## _ptr_t gl_ ## name = nullptr;

#define GL2D_API_FUNC_INIT(name) \
  gl_ ## name = reinterpret_cast<gl_ ## name ## _ptr_t>(wglGetProcAddress("gl" ## #name));

struct api
{
  GL2D_API_FUNC(GLuint, CreateProgram);

  api()
  {
    GL2D_API_FUNC_INIT(CreateProgram);
  }
};

#undef GL2D_API_FUNC
#undef GL2D_API_FUNC_INIT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vertex
{
  vec2 pos;
  color color;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vbo
{
  
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct shader
{
  const api &gl_api;

  explicit shader(const api &glapi)
    : gl_api(glapi)
  {

  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct state
{
  rect viewport;
  rect scissors;
  color color;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context
{
public:
  context()
    : _shader(_api)
  {

  }

  virtual ~context()
  {
    
  }

  void color(const gl2d::color &c) { _state.color = c; }

  const gl2d::color &color() const { return _state.color; }

  void move_to(const vec2 &pos)
  {
    
  }

  void line_to(const vec2 &pos)
  {
    
  }

  void frame_begin()
  {
    
  }
  
  void frame_end()
  {
    
  }

private:
  detail::api _api;

  detail::shader _shader;

  detail::vbo _vbo;

  detail::state _state;
};

}
