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
  static const GLenum ELEMENT_ARRAY_BUFFER = 0x8893;
  static const GLenum STREAM_DRAW = 0x88E0;
  static const GLenum STREAM_READ = 0x88E1;
  static const GLenum STREAM_COPY = 0x88E2;
  static const GLenum STATIC_DRAW = 0x88E4;
  static const GLenum STATIC_READ = 0x88E5;
  static const GLenum STATIC_COPY = 0x88E6;
  static const GLenum DYNAMIC_DRAW = 0x88E8;
  static const GLenum DYNAMIC_READ = 0x88E9;
  static const GLenum DYNAMIC_COPY = 0x88EA;
  static const GLenum PIXEL_PACK_BUFFER = 0x88EB;
  static const GLenum PIXEL_UNPACK_BUFFER = 0x88EC;
  static const GLenum FRAGMENT_SHADER = 0x8B30;
  static const GLenum VERTEX_SHADER = 0x8B31;
  static const GLenum COMPILE_STATUS = 0x8B81;
  static const GLenum LINK_STATUS = 0x8B82;
  static const GLenum GEOMETRY_SHADER = 0x8DD9;
  static const GLenum COMPUTE_SHADER = 0x91B9;

  bool init();
};

#undef GL3D_API_FUNC

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern detail::gl_api gl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

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

//---------------------------------------------------------------------------------------------------------------------
class ref_counted
{
  mutable std::atomic_int _refCount = { 0 };

public:
  void ref() const { ++_refCount; }
  void unref() const { if (!(--_refCount)) delete this; }

protected:
  virtual ~ref_counted() { }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> class ptr
{
public:
  ptr() { }
  ptr(T *p): _ptr(p) { if (p) p->ref(); }
  ptr(const ptr &p): _ptr(p._ptr) { if (_ptr) _ptr->ref(); }
  ptr(ptr &&rhs): _ptr(rhs._ptr) { rhs._ptr = nullptr; }
  ~ptr() { if (_ptr) _ptr->unref(); }

  T *operator->() const { return _ptr; }
  operator T*() const { return _ptr; }
  ptr &operator=(T *p) { assign(p); return *this; }

private:
  void assign(T *p)
  {
    if (p == _ptr) return;
    T *oldPtr = _ptr;
    if (p) p->ref();
    if (oldPtr) oldPtr->unref();
    _ptr = p;
  }

  T *_ptr = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
class compiled_object : public ref_counted
{
public:
  void set_dirty(bool set = true) { _dirty = set; }
  bool dirty() const { return _dirty; }

protected:
  bool _dirty = true;
};

//---------------------------------------------------------------------------------------------------------------------
class compiled_program : public compiled_object
{
public:
  GLuint id() const { return _program.id; }

  const std::string &last_error() const { return _lastError; }

  void set_glsl_version(const std::string &verString) { _glslVersion = verString; set_dirty(); }
  const std::string &glsl_version() const { return _glslVersion; }

  void define(const std::string &name, const std::string &value)
  {
    _macros[name] = value;
    set_dirty();
  }

  bool undef(const std::string &name)
  {
    auto iter = _macros.find(name);
    if (iter == _macros.end()) return false;
    _macros.erase(iter);
    set_dirty();
    return true;
  }

  void undef_all() { if (!_macros.empty()) { _macros.clear(); set_dirty(); } }

  std::string get_macro_string() const
  {
    std::string macroString = "#version " + _glslVersion + "\n";
    for (auto &&kvp : _macros) macroString += "#define " + kvp.first + " " + kvp.second + "\n";
    return macroString;
  }

  virtual bool bind() = 0;
  virtual void unbind() { gl.UseProgram(0); }
  
protected:
  virtual ~compiled_program()
  {
    _program.destroy();    
  }

  std::map<std::string, std::string> _macros;
  detail::gl_resource_program _program;
  std::string _glslVersion = "330";
  std::string _lastError;
};

//---------------------------------------------------------------------------------------------------------------------
class buffer : public compiled_object
{
public:
  buffer() { }

  GLuint id() const { return _buffer.id; }

  void clear()
  {
    if (_owner && _data) { delete [] _data; _data = nullptr; }

    _size = 0;
    _keepData = false;
    set_dirty();
  }

  void alloc_data(const void *ptr, size_t size, bool keep = true)
  {
    if (_owner && _size != size)
      clear();

    if (size)
    {
      _size = size;
      if (!_data) _data = new uint8_t[_size];
      if (ptr) memcpy(const_cast<uint8_t *>(_data), ptr, _size);
    }

    _owner = true;
    _keepData = keep;
  }

  void set_data(const void *ptr, size_t size)
  {
    clear();
    _data = static_cast<const uint8_t *>(ptr);
    _size = size;
  }

  const uint8_t *data() const { return _data; }

  size_t size() const { return _size; }

  bool bind(GLenum type);
  void unbind(GLenum type);

protected:
  virtual ~buffer()
  {
    clear();
    _buffer.destroy();
  }

  bool _keepData = false;
  bool _owner = false;
  const uint8_t *_data = nullptr;
  size_t _size = 0;
  gl_resource_buffer _buffer;
};

//---------------------------------------------------------------------------------------------------------------------
class base_geometry : public compiled_object
{
public:
  GLuint id() const { return _vao.id; }

  void set_vertex_buffer(buffer *vb) { _vertexBuffer = vb; set_dirty(); }
  buffer *vertex_buffer() const { return _vertexBuffer; }
  void set_index_buffer(buffer *ib) { _indexBuffer = ib; set_dirty(); }
  buffer *index_buffer() const { return _indexBuffer; }

  virtual bool bind();
  virtual void unbind();
  
protected:
  virtual ~base_geometry()
  {
    _vao.destroy();
  }

  detail::gl_resource_vao _vao;
  ptr<buffer> _vertexBuffer = new buffer();
  ptr<buffer> _indexBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> struct init_vao_arg { };

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

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... T> class layout
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

public:
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

  void clear_vertices() { _vertexCursor = 0; set_dirty(); }
  void clear_indices() { _indexCursor = 0; set_dirty(); }
  void clear() { _vertexCursor = _indexCursor = 0; set_dirty(); }

  size_t size_vertices() const { return _vertexCursor; }
  size_t size_indices() const { return _indexCursor; }

  T *alloc_vertices(size_t count)
  {
    if (!_vertexBuffer)
      return nullptr;

    if (_vertexCursor + count > size_vertices())
      _vertices.resize(_vertexCursor + count);

    auto result = _vertices.data() + _vertexCursor;
    _vertexCursor += count;
    set_dirty();
    return result;
  }

  void pop_vertices(size_t count) { _vertexCursor = (count > _vertexCursor) ? 0 : (_vertexCursor - count); }
  void pop_indices(size_t count) { _indexCursor = (count > _indexCursor) ? 0 : (_indexCursor - count); }

  bool bind() override
  {
    if (dirty())
    {
      if (_vertexBuffer) _vertexBuffer->set_data(_vertices.data(), _vertexCursor * sizeof(T));
      if (_indexBuffer) _indexBuffer->set_data(_indices.data(), _indexCursor * sizeof(int));
      set_dirty(false);
    }

    if (!_vao.id && _vertexBuffer)
    {
      _vertexBuffer->bind(gl.ARRAY_BUFFER);
      gl.GenVertexArrays(1, &_vao.id);
      gl.BindVertexArray(_vao);
      T::init_vao();
    }

    return detail::base_geometry::bind();
  }

protected:
  virtual ~custom_geometry()
  {

  }

  std::vector<T> _vertices;
  size_t _vertexCursor = 0;

  std::vector<int> _indices;
  size_t _indexCursor = 0;
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

  bool bind() override;

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

  bool bind() override;
  void dispatch(int numGroupsX, int numGroupsY, int numGroupsZ = 1);

protected:
  virtual ~compute()
  {
    _shader.destroy();
  }

  std::string _source;
  detail::gl_resource_shader _shader;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::compiled_object
{
public:
  texture(GLenum textureType = GL_TEXTURE_2D): _type(textureType)
  {
    
  }

  GLenum type() const { return _type; }
  GLuint id() const { return _texture.id; }

  struct part
  {
    int layer_index = 0;   // texture layer index
    int mip_level = 0;     // layer mip level
    ivec2 size;            // width and height in pixels
    size_t offset = 0;     // byte offset from start of the buffer
    size_t row_stride = 0; // row size in bytes
    size_t length = 0;     // total part size in bytes
  };

  void set_params(int width, int height, GLenum format, size_t sizeLayers);

  void set_format(GLenum format) { set_params(_size.x, _size.y, format, _sizeLayers); }
  GLenum format() const { return _format; }

  void set_size(int width, int height) { set_params(width, height, _format, _sizeLayers); }
  ivec2 size(size_t mipLevel = 0) const;
  
  void set_size_layers(size_t count) { set_params(_size.x, _size.y, _format, count); }
  size_t size_layers() const { return _sizeLayers; }

  void set_pixel_buffer(detail::buffer *pb) { _pbo = pb; set_dirty(); }
  detail::buffer *pixel_buffer() const { return _pbo; }

protected:
  virtual ~texture()
  {
    _texture.destroy();
  }

  void update_parts();

  GLenum _type;
  detail::gl_resource_texture _texture;
  detail::ptr<detail::buffer> _pbo = new detail::buffer();
  std::vector<part> _parts;
  GLenum _format = GL_RGBA;
  ivec2 _size;
  size_t _sizeLayers = 1;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context3d
{
public:
  context3d() { }
  virtual ~context3d() { }

  void clear();

  bool bind(detail::base_geometry *geometry);
  bool bind(detail::compiled_program *prog);
  bool bind(texture *tex, int slot = 0);

  bool set_uniform(const char *name, int value);
  bool set_uniform(const char *name, const vec2 &value);
  bool set_uniform(const char *name, texture *value);
    
private:
  detail::ptr<detail::base_geometry> _geometry;
  detail::ptr<detail::compiled_program> _program;
  detail::ptr<texture> _textures[16];
};

}

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_H_IMPL__
#define __GL3D_H_IMPL__

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#endif

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

//------------------------------------------------------------------------------------------------------------------------
bool buffer::bind(GLenum type)
{
  if (dirty())
  {
    if (!_buffer.id)
      gl.GenBuffers(1, &_buffer.id);

    gl.BindBuffer(type, _buffer.id);
    gl.BufferData(type, _size, _data, gl.STREAM_DRAW);

    if (_owner && !_keepData && _data) { delete [] _data; _data = nullptr; }
    set_dirty(false);
  }
  else
    gl.BindBuffer(type, _buffer.id);

  return _buffer.id > 0;
}

//------------------------------------------------------------------------------------------------------------------------
void buffer::unbind(GLenum type)
{
  gl.BindBuffer(type, 0);
}

//------------------------------------------------------------------------------------------------------------------------
bool base_geometry::bind()
{
  if (!_vertexBuffer || !_vao.id)
    return false;

  _vertexBuffer->bind(gl.ARRAY_BUFFER);
  gl.BindVertexArray(_vao);

  if (_indexBuffer)
    _indexBuffer->bind(gl.ELEMENT_ARRAY_BUFFER);
  
  return true;
}

//------------------------------------------------------------------------------------------------------------------------
void base_geometry::unbind()
{
  if (_vertexBuffer)
    _vertexBuffer->unbind(gl.ARRAY_BUFFER);
  
  gl.BindVertexArray(0);

  if (_indexBuffer)
    _indexBuffer->unbind(gl.ELEMENT_ARRAY_BUFFER);
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

  gl.UseProgram(_program.id);
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

  gl.UseProgram(_program.id);
  return _program.id != 0;
}

//------------------------------------------------------------------------------------------------------------------------
void compute::dispatch(int numGroupsX, int numGroupsY, int numGroupsZ)
{
  
}

//------------------------------------------------------------------------------------------------------------------------
void texture::update_parts()
{
  set_dirty();
}

//------------------------------------------------------------------------------------------------------------------------
void texture::set_params(int width, int height, GLenum format, size_t sizeLayers)
{
  if (_size.x != width || _size.y != height || _format != format || _sizeLayers != sizeLayers)
  {
    _size = ivec2(width, height); _format = format; _sizeLayers = sizeLayers;
    update_parts();
  }
}

//------------------------------------------------------------------------------------------------------------------------
ivec2 texture::size(size_t mipLevel) const
{
  return _size;
}

//------------------------------------------------------------------------------------------------------------------------
void context3d::clear()
{
  if (_geometry) _geometry->unbind();
  _geometry = nullptr;

  if (_program) _program->unbind();
  _program = nullptr;

  for (size_t i = 0; i < 16; ++i)
  {
    _textures[i] = nullptr;
  }
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind(detail::base_geometry *geometry)
{
  if (geometry != _geometry)
  {
    if (_geometry) _geometry->unbind();
    if (_geometry = geometry) return _geometry->bind();
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind(detail::compiled_program *prog)
{
  if (prog != _program)
  {
    if (_program) _program->unbind();
    if (_program = prog) return _program->bind();
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind(texture *tex, int slot)
{
  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(const char *name, int value)
{
  if (!_program) return false;
  if (auto id = gl.GetUniformLocation(_program->id(), name))
  {
    gl.Uniform1i(id, value);
    return true;      
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(const char *name, const vec2 &value)
{
  if (!_program) return false;
  if (auto id = gl.GetUniformLocation(_program->id(), name))
  {
    gl.Uniform2fv(id, 1, value.data());
    return true;      
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(const char *name, texture *value)
{
  return false;
}

}

#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
