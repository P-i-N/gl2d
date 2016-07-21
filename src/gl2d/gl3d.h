#ifndef __GL3D_H__
#define __GL3D_H__

#include <atomic>
#include <vector>

#if !defined(GL3D_APIENTRY)
#if defined(WIN32)
#define GL3D_APIENTRY __stdcall
#else
#define GL3D_APIENTRY
#endif
#endif

#if defined(WIN32)
#include <windows.h>
#else
#endif

#include <gl/GL.h>

namespace gl3d {
  
namespace detail {

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xvec2
{
  T x = 0, y = 0;

  xvec2() { }
  xvec2(const xvec2 &copy): x(copy.x), y(copy.y) { }

  template <typename T2>
  xvec2(T2 v): x(static_cast<T>(v)), y(static_cast<T>(v)) { }

  template <typename T2>
  xvec2(T2 _x, T2 _y): x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }

  T *data() { return &x; }
  const T *data() const { return &x; }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xvec3
{
  T x = 0, y = 0, z = 0;

  xvec3() { }
  xvec3(const xvec3 &copy): x(copy.x), y(copy.y), z(copy.z) { }

  template <typename T2>
  xvec3(T2 v): x(static_cast<T>(v)), y(static_cast<T>(v)), z(static_cast<T>(v)) { }

  template <typename T2>
  xvec3(T2 _x, T2 _y, T2 _z): x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) { }

  T *data() { return &x; }
  const T *data() const { return &x; }
};

}

typedef detail::xvec2<float> vec2;
typedef detail::xvec2<int> ivec2;
typedef detail::xvec3<float> vec3;
typedef detail::xvec3<int> ivec3;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct rgba_color
{
  float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;

  rgba_color() { }
  rgba_color(const rgba_color &copy): r(copy.r), g(copy.g), b(copy.b), a(copy.a) { }
  rgba_color(float _r, float _g, float _b, float _a = 1.0f): r(_r), g(_g), b(_b), a(_a) { }
  rgba_color(uint32_t argb)
    : r(((argb >> 16) & 0xFFu) / 255.0f), g(((argb >> 8) & 0xFFu) / 255.0f)
    , b((argb & 0xFFu) / 255.0f), a(((argb >> 24) & 0xFFu) / 255.0f) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct gl_resource
{
  GLuint id = 0;
  operator GLuint() const { return id; }
};

struct gl_resource_buffer : gl_resource { void destroy(); };
struct gl_resource_vao : gl_resource { void destroy(); };
struct gl_resource_shader : gl_resource { void destroy(); };
struct gl_resource_program : gl_resource { void destroy(); };
struct gl_resource_texture : gl_resource { void destroy(); };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GL3D_API_FUNC(retValue, name, ...) \
  typedef retValue(GL3D_APIENTRY *gl_ ## name ## _ptr_t)(__VA_ARGS__); \
  gl_ ## name ## _ptr_t name = reinterpret_cast<gl_ ## name ## _ptr_t>(get_gl_proc_address("gl" ## #name));

#define GL3D_API_FUNC_INIT(name) \
  name = reinterpret_cast<gl_ ## name ## _ptr_t>(get_gl_proc_address("gl" ## #name)); \
  if (name == nullptr) return false

//---------------------------------------------------------------------------------------------------------------------
struct gl_api
{
  static void *get_gl_proc_address(const char *funcName) { return wglGetProcAddress(funcName); }

  GL3D_API_FUNC(void, GenBuffers, GLsizei, GLuint *)
  GL3D_API_FUNC(void, DeleteBuffers, GLsizei, const GLuint *)
  GL3D_API_FUNC(void, BindBuffer, GLenum, GLuint)
  GL3D_API_FUNC(void, BufferData, GLenum, ptrdiff_t, const GLvoid *, GLenum)
  GL3D_API_FUNC(void, GenVertexArrays, GLsizei, GLuint *)
  GL3D_API_FUNC(void, BindVertexArray, GLuint)
  GL3D_API_FUNC(void, EnableVertexAttribArray, GLuint)
  GL3D_API_FUNC(void, VertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *)
  GL3D_API_FUNC(void, BindAttribLocation, GLuint, GLuint, const char *)
  GL3D_API_FUNC(void, DeleteVertexArrays, GLsizei, const GLuint *)
  GL3D_API_FUNC(GLuint, CreateShader, GLenum)
  GL3D_API_FUNC(void, DeleteShader, GLuint)
  GL3D_API_FUNC(void, ShaderSource, GLuint, GLsizei, const char **, const GLint *)
  GL3D_API_FUNC(void, CompileShader, GLuint)
  GL3D_API_FUNC(void, GetShaderiv, GLuint, GLenum, GLint *)
  GL3D_API_FUNC(GLuint, CreateProgram)
  GL3D_API_FUNC(void, DeleteProgram, GLuint)
  GL3D_API_FUNC(void, AttachShader, GLuint, GLuint)
  GL3D_API_FUNC(void, DetachShader, GLuint, GLuint)
  GL3D_API_FUNC(void, LinkProgram, GLuint)
  GL3D_API_FUNC(void, UseProgram, GLuint)
  GL3D_API_FUNC(void, GetProgramiv, GLuint, GLenum, GLint *)
  GL3D_API_FUNC(GLint, GetUniformLocation, GLuint, const char *)
  GL3D_API_FUNC(void, Uniform1i, GLint, GLint)
  GL3D_API_FUNC(void, Uniform2fv, GLint, GLsizei, const GLfloat *)
  GL3D_API_FUNC(void, ActiveTexture, GLenum)
    
  static const GLenum CLAMP_TO_EDGE = 0x812F;
  static const GLenum TEXTURE0 = 0x84C0;
  static const GLenum ARRAY_BUFFER = 0x8892;
  static const GLenum STREAM_DRAW = 0x88E0;
  static const GLenum FRAGMENT_SHADER = 0x8B30;
  static const GLenum VERTEX_SHADER = 0x8B31;
  static const GLenum COMPILE_STATUS = 0x8B81;
  static const GLenum LINK_STATUS = 0x8B82;
  static const GLenum GEOMETRY_SHADER = 0x8DD9;

  bool init();

  gl_resource_shader compile_shader(GLenum shaderType, const char *source);

  gl_resource_program link_program(const gl_resource_shader &vert, const gl_resource_shader &geom, const gl_resource_shader &frag);
  gl_resource_program link_program(const gl_resource_shader &vert, const gl_resource_shader &frag)
  {
    return link_program(vert, gl_resource_shader(), frag);
  }
};

#undef GL3D_API_FUNC
#undef GL3D_API_FUNC_INIT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct gl_api_accessor { gl_api *operator->() const; };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class ptr
{
public:
  ptr() { }
  ptr(T *p): _ptr(p) { if (p) p->ref(); }
  ptr(const ptr &p): _ptr(p._ptr) { if (_ptr) _ptr->ref(); }
  ptr(ptr &&rhs): _ptr(rhs._ptr) { rhs._ptr = nullptr; }
  ~ptr() { if (_ptr) _ptr->unref(); }

  T *operator->() const { return _ptr; }

private:
  T *_ptr = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class ref_counted
{
  friend class ptr<T>;
public:
  typedef ptr<T> ptr;

protected:
  virtual ~ref_counted() { }

private:
  void ref() const { ++_refCount; }
  void unref() const { if (!(--_refCount)) delete this; }

  mutable std::atomic_int _refCount = { 0 };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

extern detail::gl_api_accessor gl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

template <typename T> struct init_vao_arg { };

#define INIT_VAO_ARG(_Type, _NumElements, _ElementType) \
  template <> struct init_vao_arg<_Type> { \
    static void apply(GLuint index, size_t size, const void *offset) { \
      gl->VertexAttribPointer(index, _NumElements, _ElementType, GL_FALSE, size, offset); } };

INIT_VAO_ARG(vec2, 2, GL_FLOAT)
INIT_VAO_ARG(vec3, 3, GL_FLOAT)
INIT_VAO_ARG(rgba_color, 4, GL_FLOAT)

#undef INIT_VAO_ARG

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... T> struct layout
{
  template <typename Head, typename... Tail> struct helper
  {
    Head head;
    helper<Tail...> tail;

    void init_vao(GLuint index, size_t size, size_t offset)
    {
      gl->EnableVertexAttribArray(index);
      detail::init_vao_arg<Head>::apply(index, size, reinterpret_cast<const void *>(offset));
      tail.init_vao(index + 1, size, offset + offsetof(std::remove_pointer_t<decltype(this)>, tail));
    }
  };

  template <typename Head> struct helper<Head>
  {
    Head head;

    void init_vao(GLuint index, size_t size, size_t offset)
    {
      gl->EnableVertexAttribArray(index);
      detail::init_vao_arg<Head>::apply(index, size, reinterpret_cast<const void *>(offset));
    }
  };

  static void init_vao() { helper<T...> h; h.init_vao(0, sizeof(h), 0); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vertex3d : layout<vec3, vec3, rgba_color, vec2>
{
  vec3 pos;
  vec3 normal;
  rgba_color color;
  vec2 uv;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class geometry : public detail::ref_counted<geometry>
{
public:
protected:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class technique : public detail::ref_counted<technique>
{
  
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::ref_counted<texture>
{
  
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context3d
{
public:

};

}

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_H_IMPL__
#define __GL3D_H_IMPL__

#include <memory>

namespace gl3d {

static std::unique_ptr<detail::gl_api> g_gl;
static detail::gl_api_accessor gl;

namespace detail {

//------------------------------------------------------------------------------------------------------------------------
gl_api *gl_api_accessor::operator->() const
{
  if (!g_gl) g_gl.reset(new detail::gl_api());
  return g_gl.get();
}

//------------------------------------------------------------------------------------------------------------------------
void gl_resource_buffer::destroy() { if (id > 0) g_gl->DeleteBuffers(1, &id); id = 0; }
void gl_resource_vao::destroy() { if (id > 0) g_gl->DeleteVertexArrays(1, &id); id = 0; }
void gl_resource_shader::destroy() { if (id > 0) g_gl->DeleteShader(id); id = 0; }
void gl_resource_program::destroy() { if (id > 0) g_gl->DeleteProgram(id); id = 0; }
void gl_resource_texture::destroy() { if (id > 0) glDeleteTextures(1, &id); id = 0; }

//------------------------------------------------------------------------------------------------------------------------
bool gl_api::init()
{
  auto funcPtr = reinterpret_cast<size_t *>(this);
  for (size_t i = 0; i < sizeof(gl_api) / sizeof(size_t *); ++i, ++funcPtr)
    if (!funcPtr)
      return false;

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
gl_resource_shader gl_api::compile_shader(GLenum shaderType, const char *source)
{
  gl_resource_shader result;
  result.id = CreateShader(shaderType);
  ShaderSource(result, 1, &source, nullptr);
  CompileShader(result);

  GLint status;
  GetShaderiv(result, COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    DeleteShader(result);
    return gl_resource_shader();
  }

  return result;
}

//------------------------------------------------------------------------------------------------------------------------
gl_resource_program gl_api::link_program(const gl_resource_shader &vert, const gl_resource_shader &geom, const gl_resource_shader &frag)
{
  gl_resource_program result;
  result.id = CreateProgram();
  AttachShader(result, vert);
  if (geom.id) AttachShader(result, geom);
  AttachShader(result, frag);
  LinkProgram(result);

  GLint status;
  GetProgramiv(result, LINK_STATUS, &status);

  DetachShader(result, vert);
  if (geom.id) DetachShader(result, geom);
  DetachShader(result, frag);

  if (status == GL_FALSE)
  {
    DeleteProgram(result);
    return gl_resource_program();
  }

  return result;
}

}

}

#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
