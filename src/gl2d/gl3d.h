#ifndef __GL3D_H__
#define __GL3D_H__

#include <atomic>
#include <vector>
#include <map>

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
struct gl_resource_texture : gl_resource { void destroy(); };

struct gl_resource_shader : gl_resource
{
  void destroy();
  bool compile(GLenum shaderType, const std::string &source);
};

struct gl_resource_program : gl_resource
{
  void destroy();
  bool link(const std::initializer_list<gl_resource_shader> &shaders);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GL3D_API_FUNC(retValue, name, ...) \
  public: typedef retValue(GL3D_APIENTRY *gl_ ## name ## _ptr_t)(__VA_ARGS__); \
  gl_ ## name ## _ptr_t name = nullptr; \
  private: \
  __init __init ## name = __init(_initializers, reinterpret_cast<void **>(&name), [](void **ptr)->bool{ \
    *ptr = wglGetProcAddress("gl" ## #name); return (*ptr) != nullptr; }); \
  public:

//---------------------------------------------------------------------------------------------------------------------
class gl_api
{
  typedef bool(*init_proc_address_t)(void **);

  struct __init
  {
    void **proc_address;
    init_proc_address_t lambda;

    __init(std::vector<__init *> &output, void **target, init_proc_address_t l): proc_address(target), lambda(l)
    { output.push_back(this); }
  };
  
  std::vector<__init *> _initializers;

public:
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
  static const GLenum COMPUTE_SHADER = 0x91B9;

  bool init();
};

#undef GL3D_API_FUNC

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ref_counted
{
  mutable std::atomic_int _refCount = { 0 };

public:
  void ref() const { ++_refCount; }
  void unref() const { if (!(--_refCount)) delete this; }

protected:
  virtual ~ref_counted() { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class compiled_object : public ref_counted
{
public:
  void set_dirty(bool set = true) { _dirty = set; }
  bool dirty() const { return _dirty; }

protected:
  bool _dirty = true;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern detail::gl_api gl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct init_vao_arg { };

//---------------------------------------------------------------------------------------------------------------------
#define GL3D_INIT_VAO_ARG(_Type, _NumElements, _ElementType) \
  template <> struct init_vao_arg<_Type> { \
    static void apply(GLuint index, size_t size, const void *offset) { \
      gl.VertexAttribPointer(index, _NumElements, _ElementType, GL_FALSE, size, offset); } };

GL3D_INIT_VAO_ARG(int, 1, GL_INT)
GL3D_INIT_VAO_ARG(float, 1, GL_FLOAT)
GL3D_INIT_VAO_ARG(vec2, 2, GL_FLOAT)
GL3D_INIT_VAO_ARG(ivec2, 2, GL_INT)
GL3D_INIT_VAO_ARG(vec3, 3, GL_FLOAT)
GL3D_INIT_VAO_ARG(ivec3, 3, GL_INT)
GL3D_INIT_VAO_ARG(rgba_color, 4, GL_FLOAT)

#undef GL3D_INIT_VAO_ARG

//---------------------------------------------------------------------------------------------------------------------
class base_geometry : public compiled_object
{
public:
  void set_use_indices(bool use) { _useIndices = use; }

protected:
  virtual ~base_geometry()
  {
    _vao.destroy();
    _verticesVBO.destroy();
    _indicesVBO.destroy();
  }

  bool _useIndices = false;

  std::vector<int> _indices;

  detail::gl_resource_vao _vao;

  detail::gl_resource_buffer _verticesVBO;

  detail::gl_resource_buffer _indicesVBO;
};

//---------------------------------------------------------------------------------------------------------------------
class compiled_program : public compiled_object
{
public:
  const std::string &last_error() const { return _lastError; }

  void define(const std::string &name, const std::string &value)
  {
    _macros[name] = value;
    set_dirty();
  }

  bool undef(const std::string &name)
  {
    auto iter = _macros.find(name);
    if (iter != _macros.end())
    {
      _macros.erase(iter);
      set_dirty();
      return true;
    }

    return false;
  }

  void undef_all()
  {
    if (!_macros.empty())
    {
      _macros.clear();
      set_dirty();
    }
  }

  std::string get_macro_string() const
  {
    std::string macroString = "";
    for (auto &&kvp : _macros)
      macroString += "#define " + kvp.first + " " + kvp.second + "\n";

    return macroString;
  }
  
protected:
  virtual ~compiled_program()
  {
    _program.destroy();    
  }

  std::map<std::string, std::string> _macros;
  detail::gl_resource_program _program;
  std::string _lastError;
};

}

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

template <typename... T> struct layout
{
  template <typename Head, typename... Tail> struct helper
  {
    Head head;
    helper<Tail...> tail;

    void init_vao(GLuint index, size_t size, size_t offset)
    {
      gl.EnableVertexAttribArray(index);
      detail::init_vao_arg<Head>::apply(index, size, reinterpret_cast<const void *>(offset));
      tail.init_vao(index + 1, size, offset + offsetof(std::remove_pointer_t<decltype(this)>, tail));
    }
  };

  template <typename Head> struct helper<Head>
  {
    Head head;

    void init_vao(GLuint index, size_t size, size_t offset)
    {
      gl.EnableVertexAttribArray(index);
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

//---------------------------------------------------------------------------------------------------------------------
template <typename T> class custom_geometry : public detail::base_geometry
{
public:
  custom_geometry()
  {
    
  }

protected:
  virtual ~custom_geometry()
  {

  }

  std::vector<T> _vertices;
};

//---------------------------------------------------------------------------------------------------------------------
typedef custom_geometry<vertex3d> geometry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class technique : public detail::compiled_program
{
public:
  technique() { }
  
  void set_vert_source(const std::string &code) { _vertSource = code; set_dirty(); }
  const std::string &vert_source() const { return _vertSource; }

  void set_geom_source(const std::string &code) { _geomSource = code; set_dirty(); }
  const std::string &geom_source() const { return _geomSource; }

  void set_frag_source(const std::string &code) { _fragSource = code; set_dirty(); }
  const std::string &frag_source() const { return _fragSource; }

  bool bind();

protected:
  virtual ~technique()
  {
    _vertShader.destroy();
    _geomShader.destroy();
    _fragShader.destroy();
  }

  std::string _vertSource;
  std::string _geomSource;
  std::string _fragSource;
  detail::gl_resource_shader _vertShader;
  detail::gl_resource_shader _geomShader;
  detail::gl_resource_shader _fragShader;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class compute : public detail::compiled_program
{
public:
  compute() { }

  void set_source(const std::string &code) { _source = code; set_dirty(); }
  const std::string &source() const { return _source; }

  bool bind();

protected:
  virtual ~compute()
  {
    _shader.destroy();
  }

  std::string _source;
  detail::gl_resource_shader _shader;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::ref_counted
{
public:
  texture()
  {
    
  }

  const ivec2 &size() const { return _size; }

protected:
  virtual ~texture()
  {
    
  }

  detail::gl_resource_texture _texture;

  ivec2 _size;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class render_target : public detail::ref_counted
{
public:
  render_target()
  {
    
  }

protected:
  virtual ~render_target()
  {
    
  }
};

}

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_H_IMPL__
#define __GL3D_H_IMPL__

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#endif

#include <memory>

namespace gl3d {

static detail::gl_api gl;

namespace detail {

//------------------------------------------------------------------------------------------------------------------------
void gl_resource_buffer::destroy() { if (id > 0) gl.DeleteBuffers(1, &id); id = 0; }
void gl_resource_vao::destroy() { if (id > 0) gl.DeleteVertexArrays(1, &id); id = 0; }
void gl_resource_shader::destroy() { if (id > 0) gl.DeleteShader(id); id = 0; }
void gl_resource_program::destroy() { if (id > 0) gl.DeleteProgram(id); id = 0; }
void gl_resource_texture::destroy() { if (id > 0) glDeleteTextures(1, &id); id = 0; }

//------------------------------------------------------------------------------------------------------------------------
bool gl_api::init()
{
  for (auto &&i : _initializers)
    if (!i->lambda(i->proc_address))
      return false;

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool gl_resource_shader::compile(GLenum shaderType, const std::string &source)
{
  if (!id) id = gl.CreateShader(shaderType);
  auto srcPtr = source.c_str();
  gl.ShaderSource(id, 1, &srcPtr, nullptr);
  gl.CompileShader(id);

  GLint status; gl.GetShaderiv(id, gl.COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    gl.DeleteShader(id); id = 0;
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool gl_resource_program::link(const std::initializer_list<gl_resource_shader> &shaders)
{
  if (!id) id = gl.CreateProgram();

  for (auto &&s : shaders) gl.AttachShader(id, s);
  gl.LinkProgram(id);

  GLint status; gl.GetProgramiv(id, gl.LINK_STATUS, &status);
  for (auto &&s : shaders) gl.DetachShader(id, s);
  if (status == GL_FALSE)
  {
    gl.DeleteProgram(id); id = 0;
    return false;
  }

  return true;
}

}

//------------------------------------------------------------------------------------------------------------------------
bool technique::bind()
{
  if (dirty())
  {
    std::string macroString = get_macro_string();

    if (!_vertSource.empty())
      _vertShader.compile(gl.VERTEX_SHADER, macroString + _vertSource);
    else
      _vertShader.destroy();

    if (!_geomSource.empty())
      _geomShader.compile(gl.GEOMETRY_SHADER, macroString + _geomSource);
    else
      _geomShader.destroy();

    if (!_fragSource.empty())
      _fragShader.compile(gl.FRAGMENT_SHADER, macroString + _fragSource);
    else
      _fragShader.destroy();
    
    _program.link({ _vertShader, _geomShader, _fragShader });
    set_dirty(false);
  }

  return _program.id != 0;
}

//------------------------------------------------------------------------------------------------------------------------
bool compute::bind()
{
  if (dirty())
  {
    std::string macroString = get_macro_string();

    if (!_source.empty())
      _shader.compile(gl.COMPUTE_SHADER, macroString + _source);
    else
      _shader.destroy();
    
    _program.link({ _shader });
    set_dirty(false);
  }

  return _program.id != 0;
}

}

#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
