#ifndef __GL3D_H__
#define __GL3D_H__

#include <atomic>
#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

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

#define GL3D_UNIFORM_PROJECTION_MATRIX "u_ProjectionMatrix"
#define GL3D_UNIFORM_MODELVIEW_MATRIX "u_ModelviewMatrix"

#define GL3D_OFFSET_OF(_Class, _Member) \
	reinterpret_cast<size_t>( &( ( reinterpret_cast<const _Class *>( nullptr ) )->_Member ) )

// Include base 3D math library
#include "gl3d_math.h"

namespace gl3d {

typedef uint32_t hash_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region OpenGL API

namespace detail {

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
  GL3D_API_FUNC(void, UniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat *)
  GL3D_API_FUNC(void, ActiveTexture, GLenum)
    
  static const GLenum CLAMP_TO_EDGE = 0x812F;
  static const GLenum TEXTURE0 = 0x84C0;
  static const GLenum TEXTURE_CUBE_MAP = 0x8513;
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
  static const GLenum TEXTURE_2D_ARRAY = 0x8C1A;
  static const GLenum GEOMETRY_SHADER = 0x8DD9;
  static const GLenum TEXTURE_CUBE_MAP_ARRAY = 0x9009;
  static const GLenum COMPUTE_SHADER = 0x91B9;

  bool init();
};

#undef GL3D_API_FUNC

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern detail::gl_api gl;

#pragma endregion

namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Utilities

//---------------------------------------------------------------------------------------------------------------------
inline ivec2 calculate_mip_size(int width, int height, size_t mipLevel)
{
  return ivec2(
    maximum(1.0f, floor(width / pow(2.0f, static_cast<float>(mipLevel)))),
    maximum(1.0f, floor(height / pow(2.0f, static_cast<float>(mipLevel)))));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct init_vao_arg { };

#define GL3D_INIT_VAO_ARG(_Type, _NumElements, _ElementType) \
  template <> struct init_vao_arg<_Type> { \
    static void apply(GLuint index, size_t size, const void *offset) { \
      gl.VertexAttribPointer(index, _NumElements, _ElementType, GL_FALSE, static_cast<GLsizei>(size), offset); } };

GL3D_INIT_VAO_ARG(int, 1, GL_INT)
GL3D_INIT_VAO_ARG(float, 1, GL_FLOAT)
GL3D_INIT_VAO_ARG(vec2, 2, GL_FLOAT)
GL3D_INIT_VAO_ARG(ivec2, 2, GL_INT)
GL3D_INIT_VAO_ARG(vec3, 3, GL_FLOAT)
GL3D_INIT_VAO_ARG(ivec3, 3, GL_INT)
GL3D_INIT_VAO_ARG(vec4, 4, GL_FLOAT)
GL3D_INIT_VAO_ARG(ivec4, 4, GL_INT)

#undef GL3D_INIT_VAO_ARG

//---------------------------------------------------------------------------------------------------------------------
struct gl_format_descriptor
{
  size_t pixel_size;
  GLenum layout;
  GLenum element_format;

  static gl_format_descriptor get(GLenum format)
  {
    switch (format)
    {
      case GL_RGBA: return { 4, GL_RGBA, GL_UNSIGNED_BYTE };
      default: return { 0, GL_NONE, GL_NONE };
    }
  }
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Base GL resources

struct gl_resource
{
  GLuint id = 0;
  operator GLuint() const { return id; }

protected:
	gl_resource() = default;
};

struct gl_resource_buffer : gl_resource { void destroy(); };
struct gl_resource_vao : gl_resource { void destroy(); };
struct gl_resource_texture : gl_resource { void destroy(); };

struct gl_resource_shader : gl_resource
{
  void destroy();
  bool compile(GLenum shaderType, std::string_view source);
};

struct gl_resource_program : gl_resource
{
  void destroy();
  bool link(const std::initializer_list<gl_resource_shader> &shaders);
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Base classes

//---------------------------------------------------------------------------------------------------------------------
class compiled_object
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
	using ptr = std::shared_ptr<compiled_program>;

  GLuint id() const { return _program.id; }

  const std::string &last_error() const { return _lastError; }

  void set_glsl_version(std::string_view verString) { _glslVersion = verString; set_dirty(); }
  const std::string &glsl_version() const { return _glslVersion; }

  void define(std::string_view name, std::string_view value)
  {
    _macros[std::string(name)] = value;
    set_dirty();
  }

  bool undef(std::string_view name)
  {
    auto iter = _macros.find(std::string(name));
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

  void flush_cache()
  {
    for (auto &&kvp : _cache)
      if (kvp.second.id != _program.id)
        kvp.second.destroy();

    _cache.clear();
  }

  virtual bool bind() = 0;
  virtual void unbind() { gl.UseProgram(0); }
  
protected:
  virtual ~compiled_program()
  {
    flush_cache();
    _program.destroy();
  }

  std::map<std::string, std::string> _macros;
  std::unordered_map<hash_t, detail::gl_resource_program> _cache;
  detail::gl_resource_program _program;
  std::string _glslVersion = "330";
  std::string _lastError;
};

//---------------------------------------------------------------------------------------------------------------------
class buffer : public compiled_object
{
public:
	using ptr = std::shared_ptr<buffer>;

	buffer() = default;

	virtual ~buffer()
	{
		clear();
		_buffer.destroy();
	}

  GLuint id() const { return _buffer.id; }

  void clear()
  {
    if (_owner && _data) { delete [] _data; _data = nullptr; }

    _keepData = false;
    _owner = false;
    _data = nullptr;
    _size = 0;
    set_dirty();
  }

  void *alloc_data(const void *data, size_t size, bool keep = false)
  {
    if (_owner && _size != size)
      clear();

    if (size)
    {
      _size = size;
      if (!_data) _data = new uint8_t[_size];
      if (data) memcpy(_data, data, _size);
    }

    _owner = true;
    _keepData = keep;
    return _data;
  }

  void set_data(const void *data, size_t size)
  {
    clear();
    _data = const_cast<uint8_t *>(static_cast<const uint8_t *>(data));
    _size = size;
  }

  const uint8_t *data() const { return _data; }

  size_t size() const { return _size; }

  bool bind(GLenum type);
  void unbind(GLenum type);

protected:
  bool _keepData = false;
  bool _owner = false;
  uint8_t *_data = nullptr;
  size_t _size = 0;
  gl_resource_buffer _buffer;
};

//---------------------------------------------------------------------------------------------------------------------
class base_geometry : public compiled_object
{
public:
	using ptr = std::shared_ptr<base_geometry>;

  GLuint id() const { return _vao.id; }

  void set_vertex_buffer(buffer::ptr vb) { _vertexBuffer = vb; set_dirty(); }
  buffer::ptr vertex_buffer() const { return _vertexBuffer; }
  void set_index_buffer(buffer::ptr ib) { _indexBuffer = ib; set_dirty(); }
  buffer::ptr index_buffer() const { return _indexBuffer; }

  virtual size_t size_vertices() const = 0;
  virtual size_t size_indices() const = 0;

  virtual bool bind();
  virtual void unbind();
  
protected:
  virtual ~base_geometry()
  {
    _vao.destroy();
  }

  detail::gl_resource_vao _vao;
	std::shared_ptr<buffer> _vertexBuffer = std::make_shared<buffer>();
	std::shared_ptr<buffer> _indexBuffer;
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Basic shaders (vertex3d)

//------------------------------------------------------------------------------------------------------------------------
static const char *vertex_shader_code3d = R"GLSHADER(
layout(location = 0) in vec3 vert_Position;
layout(location = 1) in vec3 vert_Normal;
layout(location = 2) in vec4 vert_Color;
layout(location = 3) in vec2 vert_UV;

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ModelviewMatrix;

out vec3 Normal;
out vec4 Color;
out vec2 UV;

void main()
{
  gl_Position = u_ProjectionMatrix * u_ModelviewMatrix * vec4(vert_Position, 1);
  Normal = vert_Normal;
  Color = vert_Color;
  UV = vert_UV;
}
)GLSHADER";

//------------------------------------------------------------------------------------------------------------------------
static const char *fragment_shader_code3d = R"GLSHADER(
in vec3 Normal;
in vec4 Color;
in vec2 UV;

out vec4 out_Color;

void main()
{
  out_Color = Color;
}
)GLSHADER";

#pragma endregion

} // namespace detail

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
      tail.init_vao(index + 1, size, offset + GL3D_OFFSET_OF(std::remove_pointer_t<decltype(this)>, tail));
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

struct vertex3d : layout<vec3, vec3, vec4, vec2>
{
  vec3 pos;
  vec3 normal;
  vec4 color = vec4::one();
  vec2 uv;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename T> class custom_geometry : public detail::base_geometry
{
public:
	using ptr = std::shared_ptr<custom_geometry>;

	custom_geometry() = default;
	virtual ~custom_geometry() = default;

  void clear_vertices() { _vertexCursor = 0; set_dirty(); }
  void clear_indices() { _indexCursor = 0; set_dirty(); }
  void clear() { _vertexCursor = _indexCursor = 0; set_dirty(); }

  size_t size_vertices() const override { return _vertexCursor; }
  size_t size_indices() const override { return _indexCursor; }

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
	using ptr = std::shared_ptr<technique>;

	technique() = default;

	virtual ~technique()
	{
		_vertShader.destroy();
		_geomShader.destroy();
		_fragShader.destroy();
	}

  void set_vert_source(std::string_view code) { _vertSource = code; set_dirty(); }
  const std::string &vert_source() const { return _vertSource; }

  void set_geom_source(std::string_view code) { _geomSource = code; set_dirty(); }
  const std::string &geom_source() const { return _geomSource; }

  void set_frag_source(std::string_view code) { _fragSource = code; set_dirty(); }
  const std::string &frag_source() const { return _fragSource; }

  bool bind() override;

protected:
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
	using ptr = std::shared_ptr<compute>;

	compute() = default;

	virtual ~compute()
	{
		_shader.destroy();
	}

  void set_source(std::string_view code) { _source = code; set_dirty(); }
  const std::string &source() const { return _source; }

  bool bind() override;
  void dispatch(int numGroupsX, int numGroupsY, int numGroupsZ = 1);

protected:
  std::string _source;
  detail::gl_resource_shader _shader;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<texture>;

  texture(GLenum textureType = GL_TEXTURE_2D): _type(textureType) { }

	virtual ~texture()
	{
		_texture.destroy();
	}

  GLenum type() const { return _type; }
  GLuint id() const { return _texture.id; }

  struct part
  {
    size_t layer_index = 0; // texture layer index
    size_t mip_level = 0;   // layer mip level
    ivec2 size;             // width and height in pixels
    size_t offset = 0;      // byte offset from start of the buffer
    size_t row_stride = 0;  // row size in bytes
    size_t length = 0;      // total part size in bytes
  };

  const part *find_part(size_t layerIndex, size_t mipLevel) const
  {
    for (auto &&p : _parts) if (p.layer_index == layerIndex && p.mip_level == mipLevel) return &p;
    return nullptr;
  }

  void set_params(int width, int height, GLenum format, size_t sizeLayers, size_t sizeMipLevels);

  void set_format(GLenum format) { set_params(_size.x, _size.y, format, _sizeLayers, _sizeMipLevels); }
  GLenum format() const { return _format; }

  void set_size(int width, int height) { set_params(width, height, _format, _sizeLayers, _sizeMipLevels); }
  ivec2 size(size_t mipLevel) const { return detail::calculate_mip_size(_size.x, _size.y, mipLevel); }
  
  void set_size_layers(size_t count) { set_params(_size.x, _size.y, _format, count, _sizeMipLevels); }
  size_t size_layers() const { return _sizeLayers; }

  void set_size_mip_levels(size_t count) { set_params(_size.x, _size.y, _format, _sizeLayers, count); }
  size_t size_mip_levels(bool calculate = false) const;
  
  size_t size_bytes() const { return _sizeBytes; }

  void set_pixel_buffer(detail::buffer::ptr pb) { _pbo = pb; set_dirty(); }
  detail::buffer::ptr pixel_buffer() const { return _pbo; }

  void set_filter(GLenum minFilter, GLenum magFilter);
  GLenum min_filter() const { return _minFilter; }
  GLenum mag_filter() const { return _magFilter; }

  void set_wrap(GLenum wrap) { if (wrap != _wrap) { _wrap = wrap; set_dirty(); } }
  GLenum wrap() const { return _wrap; }

  void *alloc_pixels(const void *data, bool keep = false)
  {
    if (!_pbo || !_sizeBytes) return nullptr;
    set_dirty();
    return _pbo->alloc_data(data, _sizeBytes, keep);
  }

  void set_pixels(const void *data)
	{
    if (!_pbo || !_sizeBytes) return;
    set_dirty();
    _pbo->set_data(data, _sizeBytes);
  }
  
  bool bind(int slot = 0);

protected:
  void update_parts();

  GLenum _type;
  detail::gl_resource_texture _texture;
	detail::buffer::ptr _pbo = std::make_shared<detail::buffer>();
  std::vector<part> _parts;
  GLenum _format = GL_RGBA;
  ivec2 _size;
  size_t _sizeLayers = 1;
  size_t _sizeMipLevels = 1;
  size_t _sizeBytes = 0;
  GLenum _minFilter = GL_NEAREST, _magFilter = GL_NEAREST;
  GLenum _wrap = GL_REPEAT;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class render_target : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<render_target>;

	render_target() = default;

	virtual ~render_target() = default;

protected:
	// TODO
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context3d
{
public:
  context3d();
  virtual ~context3d() { }

  void clear();

  bool bind_geometry(detail::base_geometry::ptr geom);
  bool bind_program(detail::compiled_program::ptr prog);
  bool bind_texture(texture::ptr tex, int slot = 0);

	GLint get_uniform_location(std::string_view name) const;

  bool set_uniform(std::string_view name, int value);
  bool set_uniform(std::string_view name, const vec2 &value);
  bool set_uniform(std::string_view name, const mat4 &value);
  bool set_uniform(std::string_view name, texture::ptr value);
    
  int get_free_texture_slot() const { for (int i = 0; i < 16; ++i) if (!_textures[i]) return i; return -1; }

  bool draw(GLenum primitive = GL_TRIANGLES, size_t offset = 0, size_t length = static_cast<size_t>(-1));

private:
  technique::ptr _basicTechnique;
	detail::base_geometry::ptr _geometry;
	detail::compiled_program::ptr _program;
  texture::ptr _textures[16];
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
bool gl_resource_shader::compile(GLenum shaderType, std::string_view source)
{
  if (!id) id = gl.CreateShader(shaderType);
	auto srcData = source.data();
	auto srcLen = static_cast<int>(source.length());
  gl.ShaderSource(id, 1, &srcData, &srcLen);
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
    if (!_buffer.id) gl.GenBuffers(1, &_buffer.id);
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
  _parts.clear();
  _sizeBytes = 0;
  size_t numMips = size_mip_levels(true);
  size_t pixelSize = detail::gl_format_descriptor::get(_format).pixel_size;
  for (size_t i = 0; i < _sizeLayers; ++i)
  {
    for (size_t mip = 0; mip < numMips; ++mip)
    {
      part p;
      p.layer_index = i;
      p.mip_level = mip;
      p.size = size(mip);
      p.offset = _sizeBytes;
      p.row_stride = p.size.x * pixelSize;
      _sizeBytes += p.length = p.row_stride * p.size.y;
      _parts.push_back(p);
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------
void texture::set_params(int width, int height, GLenum format, size_t sizeLayers, size_t sizeMipLevels)
{
  if (_size.x != width || _size.y != height || _format != format || _sizeLayers != sizeLayers || _sizeMipLevels != sizeMipLevels)
  {
    if (_type == GL_TEXTURE_1D || _type == GL_TEXTURE_2D) sizeLayers = 1;
    else if (_type == gl.TEXTURE_CUBE_MAP) sizeLayers = 6;
    _size = ivec2(width, height); _format = format; _sizeLayers = sizeLayers;
    update_parts();
  }
}

//------------------------------------------------------------------------------------------------------------------------
size_t texture::size_mip_levels(bool calculate) const
{
  if (calculate && _sizeMipLevels == 0)
  {
    size_t num = 1;
    while (true) { ivec2 s = size(num - 1); if (s.x == 1 && s.y == 1) break; ++num; }
    return num;
  }

  return _sizeMipLevels;
}

//------------------------------------------------------------------------------------------------------------------------
bool texture::bind(int slot)
{
  if (dirty())
  {
    if (!_texture.id) glGenTextures(1, &_texture.id);
    gl.ActiveTexture(gl.TEXTURE0 + slot);
    glBindTexture(_type, _texture.id);

    if (_pbo->dirty())
    {
      _pbo->bind(gl.PIXEL_UNPACK_BUFFER);
      auto desc = detail::gl_format_descriptor::get(_format);
      if (_type == GL_TEXTURE_1D || _type == GL_TEXTURE_2D)
      {
        for (auto &&p : _parts)
          glTexImage2D(_type, static_cast<GLint>(p.mip_level), desc.layout, p.size.x, p.size.y, 0, _format, desc.element_format, reinterpret_cast<const GLvoid *>(p.offset));
      }
      _pbo->unbind(gl.PIXEL_UNPACK_BUFFER);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap);
    set_dirty(false);
  }
  else
  {
    gl.ActiveTexture(gl.TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, _texture.id);
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
void texture::set_filter(GLenum minFilter, GLenum magFilter)
{
  if (minFilter != _minFilter || magFilter != _magFilter) 
  {
    _minFilter = minFilter; _magFilter = magFilter;
    set_dirty();
  }
}

//------------------------------------------------------------------------------------------------------------------------
context3d::context3d()
{
	_basicTechnique = std::make_shared<technique>();
  _basicTechnique->set_vert_source(detail::vertex_shader_code3d);
  _basicTechnique->set_frag_source(detail::fragment_shader_code3d);
}

//------------------------------------------------------------------------------------------------------------------------
void context3d::clear()
{
  if (_geometry) _geometry->unbind();
  _geometry = nullptr;

  if (_program) _program->unbind();
  _program = nullptr;

  for (size_t i = 0; i < 16; ++i)
    _textures[i] = nullptr;

  bind_program(_basicTechnique);
  glEnable(GL_DEPTH_TEST);
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind_geometry(detail::base_geometry::ptr geom)
{
  if (geom != _geometry)
  {
    if (_geometry) _geometry->unbind();
    if (_geometry = geom) return _geometry->bind();
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind_program(detail::compiled_program::ptr prog)
{
  if (prog != _program)
  {
    if (_program) _program->unbind();
    if (_program = prog) return _program->bind();
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind_texture(texture::ptr tex, int slot)
{
  return true;
}

//---------------------------------------------------------------------------------------------------------------------
GLint context3d::get_uniform_location(std::string_view name) const
{
	if (!_program) return -1;

	char buff[32];
	if (auto len = name.length(); len < sizeof(buff))
	{
		memcpy(buff, name.data(), len); buff[len] = 0;
		return gl.GetUniformLocation(_program->id(), buff);
	}

	return gl.GetUniformLocation(_program->id(), std::string(name).c_str());
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(std::string_view name, int value)
{
	if (auto id = get_uniform_location(name); id >= 0)
  {
    gl.Uniform1i(id, value);
    return true;      
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(std::string_view name, const vec2 &value)
{
	if (auto id = get_uniform_location(name); id >= 0)
	{
    gl.Uniform2fv(id, 1, value.data);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(std::string_view name, const mat4 &value)
{
	if (auto id = get_uniform_location(name); id >= 0)
	{
		gl.UniformMatrix4fv(id, 1, GL_FALSE, value.data);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::set_uniform(std::string_view name, texture::ptr value)
{
	if (auto id = get_uniform_location(name); id >= 0)
	{
		auto slot = get_free_texture_slot(); if (slot < 0) return false;
    value->bind(slot);
    gl.Uniform1i(id, slot);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::draw(GLenum primitive, size_t offset, size_t length)
{
  if (!_geometry) return false;
  
  auto numElements = _geometry->size_vertices();
  if (offset >= numElements) return false;
  
  if (offset + length > numElements)
    length = numElements - offset;
  
  glDrawArrays(primitive, static_cast<GLint>(offset), static_cast<GLsizei>(length));
  return true;
}

}

#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
