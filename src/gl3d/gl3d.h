#ifndef __GL3D_H__
#define __GL3D_H__

#include <atomic>
#include <bitset>
#include <cassert>
#include <functional>
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

#if !defined(GL3D_GLSL_VERSION)
	#define GL3D_GLSL_VERSION "330"
#endif

#define GL3D_UNIFORM_PROJECTION_MATRIX "u_ProjectionMatrix"
#define GL3D_UNIFORM_MODELVIEW_MATRIX "u_ModelviewMatrix"

#define GL3D_LAYOUT_LOCATION_POS     0
#define GL3D_LAYOUT_LOCATION_NORMAL  1
#define GL3D_LAYOUT_LOCATION_TANGENT 2
#define GL3D_LAYOUT_LOCATION_COLOR   3
#define GL3D_LAYOUT_LOCATION_UV0     4
#define GL3D_LAYOUT_LOCATION_UV1     5
#define GL3D_LAYOUT_LOCATION_UV2     6
#define GL3D_LAYOUT_LOCATION_UV3     7

#define __GL3D_TOSTRING2(arg) #arg
#define __GL3D_TOSTRING(arg) __GL3D_TOSTRING2(arg)
#define GL3D_TOSTRING(arg) __GL3D_TOSTRING(arg)

#define GL3D_OFFSET_OF(_Class, _Member) \
	reinterpret_cast<size_t>( &( ( reinterpret_cast<const _Class *>( nullptr ) )->_Member ) )

// Include base 3D math library
#include "gl3d_math.h"

namespace gl3d {

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
	using init_proc_address_t = bool( * )( void ** );

	struct __init
	{
		void **proc_address;
		init_proc_address_t lambda;

		__init( std::vector<__init *> &output, void **target, init_proc_address_t l ): proc_address( target ), lambda( l )
		{ output.push_back( this ); }
	};

	std::vector<__init *> _initializers;

public:
	GL3D_API_FUNC( void,   GenBuffers, GLsizei, GLuint * )
	GL3D_API_FUNC( void,   DeleteBuffers, GLsizei, const GLuint * )
	GL3D_API_FUNC( void,   BindBuffer, GLenum, GLuint )
	GL3D_API_FUNC( void,   BufferData, GLenum, ptrdiff_t, const GLvoid *, GLenum )
	GL3D_API_FUNC( void,   GenVertexArrays, GLsizei, GLuint * )
	GL3D_API_FUNC( void,   BindVertexArray, GLuint )
	GL3D_API_FUNC( void,   EnableVertexAttribArray, GLuint )
	GL3D_API_FUNC( void,   VertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid * )
	GL3D_API_FUNC( void,   BindAttribLocation, GLuint, GLuint, const char * )
	GL3D_API_FUNC( void,   DeleteVertexArrays, GLsizei, const GLuint * )
	GL3D_API_FUNC( GLuint, CreateShader, GLenum )
	GL3D_API_FUNC( void,   DeleteShader, GLuint )
	GL3D_API_FUNC( void,   ShaderSource, GLuint, GLsizei, const char **, const GLint * )
	GL3D_API_FUNC( void,   CompileShader, GLuint )
	GL3D_API_FUNC( void,   GetShaderiv, GLuint, GLenum, GLint * )
	GL3D_API_FUNC( GLuint, CreateProgram )
	GL3D_API_FUNC( void,   DeleteProgram, GLuint )
	GL3D_API_FUNC( void,   AttachShader, GLuint, GLuint )
	GL3D_API_FUNC( void,   DetachShader, GLuint, GLuint )
	GL3D_API_FUNC( void,   LinkProgram, GLuint )
	GL3D_API_FUNC( void,   UseProgram, GLuint )
	GL3D_API_FUNC( void,   GetProgramiv, GLuint, GLenum, GLint * )
	GL3D_API_FUNC( GLint,  GetUniformLocation, GLuint, const char * )
	GL3D_API_FUNC( void,   Uniform1i, GLint, GLint )
	GL3D_API_FUNC( void,   Uniform2fv, GLint, GLsizei, const GLfloat * )
	GL3D_API_FUNC( void,   UniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat * )
	GL3D_API_FUNC( void,   ActiveTexture, GLenum )
	GL3D_API_FUNC( void,   Enablei, GLenum, GLuint )
	GL3D_API_FUNC( void,   Disablei, GLenum, GLuint )
	GL3D_API_FUNC( void,   BlendFunci, GLuint, GLenum, GLenum )
	GL3D_API_FUNC( void,   BlendEquationi, GLuint, GLenum )

	static constexpr GLenum FUNC_ADD               = 0x8006;
	static constexpr GLenum CLAMP_TO_EDGE          = 0x812F;
	static constexpr GLenum TEXTURE0               = 0x84C0;
	static constexpr GLenum TEXTURE_CUBE_MAP       = 0x8513;
	static constexpr GLenum DEPTH_CLAMP            = 0x8650;
	static constexpr GLenum ARRAY_BUFFER           = 0x8892;
	static constexpr GLenum ELEMENT_ARRAY_BUFFER   = 0x8893;
	static constexpr GLenum STREAM_DRAW            = 0x88E0;
	static constexpr GLenum STREAM_READ            = 0x88E1;
	static constexpr GLenum STREAM_COPY            = 0x88E2;
	static constexpr GLenum STATIC_DRAW            = 0x88E4;
	static constexpr GLenum STATIC_READ            = 0x88E5;
	static constexpr GLenum STATIC_COPY            = 0x88E6;
	static constexpr GLenum DYNAMIC_DRAW           = 0x88E8;
	static constexpr GLenum DYNAMIC_READ           = 0x88E9;
	static constexpr GLenum DYNAMIC_COPY           = 0x88EA;
	static constexpr GLenum PIXEL_PACK_BUFFER      = 0x88EB;
	static constexpr GLenum PIXEL_UNPACK_BUFFER    = 0x88EC;
	static constexpr GLenum FRAGMENT_SHADER        = 0x8B30;
	static constexpr GLenum VERTEX_SHADER          = 0x8B31;
	static constexpr GLenum COMPILE_STATUS         = 0x8B81;
	static constexpr GLenum LINK_STATUS            = 0x8B82;
	static constexpr GLenum TEXTURE_2D_ARRAY       = 0x8C1A;
	static constexpr GLenum GEOMETRY_SHADER        = 0x8DD9;
	static constexpr GLenum TEXTURE_CUBE_MAP_ARRAY = 0x9009;
	static constexpr GLenum COMPUTE_SHADER         = 0x91B9;

	bool init();
};

#undef GL3D_API_FUNC

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern detail::gl_api gl;

#pragma endregion

namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Utilities

//---------------------------------------------------------------------------------------------------------------------
inline ivec2 calculate_mip_size( int width, int height, size_t mipLevel )
{
	return ivec2(
	           maximum( 1.0f, floor( width / pow( 2.0f, static_cast<float>( mipLevel ) ) ) ),
	           maximum( 1.0f, floor( height / pow( 2.0f, static_cast<float>( mipLevel ) ) ) ) );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct init_vao_arg { };

#define GL3D_INIT_VAO_ARG(_Type, _NumElements, _ElementType) \
	template <> struct init_vao_arg<_Type> { \
		static constexpr char *name = #_Type; \
		static void apply(GLuint location, size_t size, const void *offset) { \
			gl.EnableVertexAttribArray(location); \
			gl.VertexAttribPointer(location, _NumElements, _ElementType, GL_FALSE, static_cast<GLsizei>(size), offset); } };

GL3D_INIT_VAO_ARG( int, 1, GL_INT )
GL3D_INIT_VAO_ARG( float, 1, GL_FLOAT )
GL3D_INIT_VAO_ARG( vec2, 2, GL_FLOAT )
GL3D_INIT_VAO_ARG( ivec2, 2, GL_INT )
GL3D_INIT_VAO_ARG( vec3, 3, GL_FLOAT )
GL3D_INIT_VAO_ARG( ivec3, 3, GL_INT )
GL3D_INIT_VAO_ARG( vec4, 4, GL_FLOAT )
GL3D_INIT_VAO_ARG( ivec4, 4, GL_INT )

#undef GL3D_INIT_VAO_ARG

//---------------------------------------------------------------------------------------------------------------------
template <typename H, typename... T> struct layout_expander : H, layout_expander<T...>
{
	static constexpr size_t attribute_mask = ( 1 << H::layout_location ) | layout_expander<T...>::attribute_mask;

protected:
	static const std::string &layout_string()
	{
		static std::string str = detail::init_vao_arg<H::type>::name + std::string( ";" ) + layout_expander<T...>::layout_string();
		return str;
	}
};

template <typename H> struct layout_expander<H> : H
{
	static constexpr size_t attribute_mask = ( 1 << H::layout_location );

protected:
	static const std::string &layout_string()
	{
		static std::string str = detail::init_vao_arg<H::type>::name + std::string( ";" );
		return str;
	}
};

//---------------------------------------------------------------------------------------------------------------------
struct layout_desc
{
	unsigned attribute_mask = 0;
	std::string attribute_string = "";
	std::function<void()> vao_initializer = nullptr;
	size_t hash = 0;

	layout_desc() = default;

	layout_desc( unsigned attrMask, std::string_view attrString, std::function<void()> vaoInit )
		: attribute_mask( attrMask )
		, attribute_string( attrString )
		, vao_initializer( vaoInit )
		, hash( std::hash<std::string> {}( attribute_string + std::to_string( attribute_mask ) ) )
	{

	}
};

//---------------------------------------------------------------------------------------------------------------------
struct gl_format_descriptor
{
	size_t pixel_size;
	GLenum layout;
	GLenum element_format;

	static gl_format_descriptor get( GLenum format )
	{
		switch ( format )
		{
			case GL_RGBA:
				return { 4, GL_RGBA, GL_UNSIGNED_BYTE };
			default:
				return { 0, GL_NONE, GL_NONE };
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
	bool compile( GLenum shaderType, std::string_view source );
};

struct gl_resource_program : gl_resource
{
	void destroy();
	bool link( const std::initializer_list<gl_resource_shader> &shaders );
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Base classes

//---------------------------------------------------------------------------------------------------------------------
class compiled_object
{
public:
	void set_dirty( bool set = true ) { _dirty = set; }
	bool dirty() const { return _dirty; }

protected:
	bool _dirty = true;
};

//---------------------------------------------------------------------------------------------------------------------
class buffer : public compiled_object
{
public:
	using ptr = std::shared_ptr<buffer>;

	buffer( GLenum type )
		: compiled_object()
		, _type( type )
	{

	}

	virtual ~buffer()
	{
		clear();
		_buffer.destroy();
	}

	GLenum type() const { return _type; }

	GLuint id() const { return _buffer.id; }

	virtual void clear()
	{
		if ( _owner ) { delete[] _data; _data = nullptr; }

		_keepData = false;
		_owner = false;
		_data = nullptr;
		_size = 0;
		set_dirty();
	}

	void *alloc_data( const void *data, size_t size, bool keep = false )
	{
		clear();

		if ( size )
		{
			_size = size;
			_data = new uint8_t[_size];
			if ( data ) memcpy( _data, data, _size );
		}

		_owner = true;
		_keepData = keep;
		return _data;
	}

	void set_data( const void *data, size_t size )
	{
		clear();

		_data = const_cast<uint8_t *>( static_cast<const uint8_t *>( data ) );
		_size = size;
	}

	const uint8_t *data() const { return _data; }

	size_t size() const { return _size; }

	size_t stride() const { return _stride; }

	size_t size_elements() const { return _size / _stride; }

	virtual bool bind();
	virtual void unbind();

protected:
	GLenum _type;
	detail::gl_resource_buffer _buffer;
	bool _keepData = false;
	bool _owner = false;
	uint8_t *_data = nullptr;
	size_t _size = 0;
	size_t _stride = 1;
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region GLSL std140 layout types

namespace std140 {

}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Basic shaders (vertex3d)

//------------------------------------------------------------------------------------------------------------------------
static const char *vertex_shader_code3d = R"GLSHADER(
GL3D_VERTEX_POS(vec3);
GL3D_VERTEX_NORMAL(vec3);
GL3D_VERTEX_COLOR(vec4);
GL3D_VERTEX_UV0(vec2);

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ModelviewMatrix;

out vec3 Normal;
out vec4 Color;
out vec2 UV;

void main()
{
  gl_Position = u_ProjectionMatrix * u_ModelviewMatrix * vec4(vertex_pos, 1);
  Normal = vertex_normal;
  Color = vertex_color;
  UV = vertex_uv0;
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

#define GL3D_VERTEX_STRUCT(_Name, _Location) \
	template <typename T> struct vertex_ ## _Name { \
	T _Name; \
	protected: \
		static constexpr size_t layout_location = _Location; \
		using type = T; };

GL3D_VERTEX_STRUCT(pos, GL3D_LAYOUT_LOCATION_POS)
GL3D_VERTEX_STRUCT(normal, GL3D_LAYOUT_LOCATION_NORMAL)
GL3D_VERTEX_STRUCT(tangent, GL3D_LAYOUT_LOCATION_TANGENT)
GL3D_VERTEX_STRUCT(color, GL3D_LAYOUT_LOCATION_COLOR)
GL3D_VERTEX_STRUCT(uv0, GL3D_LAYOUT_LOCATION_UV0)
GL3D_VERTEX_STRUCT(uv1, GL3D_LAYOUT_LOCATION_UV1)
GL3D_VERTEX_STRUCT(uv2, GL3D_LAYOUT_LOCATION_UV2)
GL3D_VERTEX_STRUCT(uv3, GL3D_LAYOUT_LOCATION_UV3)

#undef GL3D_VERTEX_STRUCT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename... T> class layout : public detail::layout_expander<T...>
{
	template <typename Head, typename... Tail> struct helper
	{
		Head head;
		helper<Tail...> tail;

		void init_vao(size_t size, size_t offset)
		{
			detail::init_vao_arg<Head::type>::apply(Head::layout_location, size, reinterpret_cast<const void *>(offset));
			tail.init_vao(size, offset + GL3D_OFFSET_OF(std::remove_pointer_t<decltype(this)>, tail));
		}
	};

	template <typename Head> struct helper<Head>
	{
		Head head;

		void init_vao(size_t size, size_t offset)
		{
			detail::init_vao_arg<Head::type>::apply(Head::layout_location, size, reinterpret_cast<const void *>(offset));
		}
	};

public:
	static const detail::layout_desc &layout_desc()
	{
		static detail::layout_desc desc(attribute_mask, detail::layout_expander<T...>::layout_string(), []()
		{ static helper<T...> h; h.init_vao(sizeof(h), 0); });
		return desc;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vertex3d : layout<vertex_pos<vec3>, vertex_normal<vec3>, vertex_color<vec4>, vertex_uv0<vec2>>
{

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
class structured_buffer : public detail::buffer
{
public:
	using element_t = T;

	structured_buffer(GLenum type)
		: buffer(type)
	{
		_stride = sizeof(T);
	}

	virtual ~structured_buffer() = default;

	size_t size_elements() const { return _elementCursor; }

	T *alloc_elements(size_t count)
	{
		if (_elementCursor + count > _elements.size())
			_elements.resize(_elementCursor + count);

		auto result = _elements.data() + _elementCursor;
		_elementCursor += count;
		set_dirty();
		return result;
	}

	void pop_elements(size_t count)
	{
		_elementCursor = (count > _elementCursor) ? 0 : (_elementCursor - count);
		set_dirty();
	}

	bool bind() override
	{
		if (dirty())
			set_data(_elements.data(), _elementCursor * sizeof(T));

		return buffer::bind();
	}

protected:
	std::vector<T> _elements;
	size_t _elementCursor = 0;
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
class vertex_buffer : public structured_buffer<T>
{
public:
	using ptr = std::shared_ptr<vertex_buffer>;

	const detail::layout_desc &get_layout_desc() const { return _layout_desc; }

	vertex_buffer()
		: structured_buffer(detail::gl_api::ARRAY_BUFFER)
		, _layout_desc(T::layout_desc())
	{

	}

	virtual ~vertex_buffer()
	{
		_vao.destroy();
	}

	bool bind() override
	{
		if (!_vao.id)
		{
			structured_buffer::bind();
			gl.GenVertexArrays(1, &_vao.id);
			gl.BindVertexArray(_vao);
			_layout_desc.vao_initializer();
		}

		if (!structured_buffer::bind())
			return false;

		gl.BindVertexArray(_vao);
		return true;
	}

	void unbind() override
	{
		gl.BindVertexArray(0);
		structured_buffer::unbind();
	}

protected:
	detail::layout_desc _layout_desc;

	detail::gl_resource_vao _vao;
};

//---------------------------------------------------------------------------------------------------------------------
class index_buffer : public structured_buffer<int>
{
public:
	using ptr = std::shared_ptr<index_buffer>;

	index_buffer()
		: structured_buffer(detail::gl_api::ELEMENT_ARRAY_BUFFER)
	{

	}

	virtual ~index_buffer() = default;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename T> class geometry : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<geometry>;
	using vertex_t = T;

	geometry() = default;

	virtual ~geometry() = default;

	GLuint id() const { return _vao.id; }

	void set_vertices(typename vertex_buffer<T>::ptr vb) { _vertexBuffer = vb; set_dirty(); }
	typename vertex_buffer<T>::ptr vertices() const { return _vertexBuffer; }

	void set_indices(index_buffer::ptr ib) { _indexBuffer = ib; set_dirty(); }
	index_buffer::ptr indices() const { return _indexBuffer; }

  void clear_vertices() { if (_vertexBuffer) { _vertexBuffer->clear(); set_dirty(); } }
	void clear_indices() { if (_indexBuffer) { _indexBuffer->clear(); set_dirty(); } }
	void clear() { clear_vertices(); clear_indices(); }

  T *alloc_vertices(size_t count)
  {
    if (!_vertexBuffer)
			set_vertices(std::make_shared<vertex_buffer<T>>());

		return _vertexBuffer->alloc_elements(count);
  }

	void pop_vertices(size_t count)
	{
		if (_vertexBuffer)
			_vertexBuffer->pop_elements(count);
	}

	int *alloc_indices(size_t count)
	{
		if (!_indexBuffer)
			set_indices(std::make_shared<index_buffer>());

		return _indexBuffer->alloc_elements(count);
	}

	void pop_indices(size_t count)
	{
		if (_indexBuffer)
			_indexBuffer->pop_elements(count);
	}

  bool bind()
  {
		bool result = true;

		if (_vertexBuffer)
			result = result && _vertexBuffer->bind();

		if (_indexBuffer)
			result = result && _indexBuffer->bind();

		return result;
  }

	void unbind()
	{
		if (_vertexBuffer)
			_vertexBuffer->unbind();

		if (_indexBuffer)
			_indexBuffer->unbind();
	}

	bool draw(GLenum primitive = GL_TRIANGLES, size_t offset = 0, size_t length = static_cast<size_t>(-1))
	{
		if (!_vertexBuffer) return false;

		auto numElements = _vertexBuffer->size_elements();
		if (offset >= numElements) return false;

		if (offset + length > numElements)
			length = numElements - offset;

		glDrawArrays(primitive, static_cast<GLint>(offset), static_cast<GLsizei>(length));
		return true;
	}

	bool draw_instanced(size_t count, GLenum primitive = GL_TRIANGLES, size_t offset = 0, size_t length = static_cast<size_t>(-1))
	{
		if (!_vertexBuffer) return false;

		auto numElements = _vertexBuffer->size_elements();
		if (offset >= numElements) return false;

		if (offset + length > numElements)
			length = numElements - offset;

		return true;
	}

protected:
	typename vertex_buffer<T>::ptr _vertexBuffer;
	index_buffer::ptr _indexBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
class program : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<program>;

	program() = default;

	virtual ~program()
	{
		_vertShader.destroy();
		_geomShader.destroy();
		_fragShader.destroy();
		_computeShader.destroy();
		_program.destroy();
	}

	GLuint id() const { return _program.id; }

	const std::string &last_error() const { return _lastError; }

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
		std::string macroString = "#version " GL3D_GLSL_VERSION "\n";

#define GL3D_LAYOUT_MACRO(_Suffix, _Location, _NameSuffix) macroString += \
        "#define GL3D_VERTEX_" _Suffix "(_Type) layout(location = " GL3D_TOSTRING(_Location) \
        ") in _Type vertex_" _NameSuffix "\n";

		GL3D_LAYOUT_MACRO("POS",     GL3D_LAYOUT_LOCATION_POS,     "pos")
		GL3D_LAYOUT_MACRO("NORMAL",  GL3D_LAYOUT_LOCATION_NORMAL,  "normal")
		GL3D_LAYOUT_MACRO("TANGENT", GL3D_LAYOUT_LOCATION_TANGENT, "tangent")
		GL3D_LAYOUT_MACRO("COLOR",   GL3D_LAYOUT_LOCATION_COLOR,   "color")
		GL3D_LAYOUT_MACRO("UV0",     GL3D_LAYOUT_LOCATION_UV0,     "uv0")
		GL3D_LAYOUT_MACRO("UV1",     GL3D_LAYOUT_LOCATION_UV1,     "uv1")
		GL3D_LAYOUT_MACRO("UV2",     GL3D_LAYOUT_LOCATION_UV2,     "uv2")
		GL3D_LAYOUT_MACRO("UV3",     GL3D_LAYOUT_LOCATION_UV3,     "uv3")

#undef GL3D_LAYOUT_MACRO

		for (auto && kvp : _macros) macroString += "#define " + kvp.first + " " + kvp.second + "\n";
		return macroString;
	}

	void set_vert_source(std::string_view code) { _vertSource = code; set_dirty(); }
	const std::string &vert_source() const { return _vertSource; }

	void set_geom_source(std::string_view code) { _geomSource = code; set_dirty(); }
	const std::string &geom_source() const { return _geomSource; }

	void set_frag_source(std::string_view code) { _fragSource = code; set_dirty(); }
	const std::string &frag_source() const { return _fragSource; }

	void set_compute_source(std::string_view code) { _computeSource = code; set_dirty(); }
	const std::string &compute_source() const { return _computeSource; }

	size_t compile_permutation(std::string_view macros);

	bool bind(size_t permutation = 0);

	void unbind() { gl.UseProgram(0); }

protected:
	std::string _vertSource;
	std::string _geomSource;
	std::string _fragSource;
	std::string _computeSource;

	detail::gl_resource_shader _vertShader;
	detail::gl_resource_shader _geomShader;
	detail::gl_resource_shader _fragShader;
	detail::gl_resource_shader _computeShader;

	detail::gl_resource_program _program;

	std::map<std::string, std::string> _macros;
	std::string _lastError;

	struct permutation_desc
	{
		std::string macros;
		detail::gl_resource_program program;
	};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<texture>;

  texture(GLenum textureType = GL_TEXTURE_2D)
		: _type(textureType)
	{

	}

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
	detail::buffer::ptr _pbo = std::make_shared<detail::buffer>(detail::gl_api::PIXEL_UNPACK_BUFFER);
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

class fbo : public detail::compiled_object
{
public:
	using ptr = std::shared_ptr<fbo>;

	fbo() = default;

	virtual ~fbo() = default;

protected:
	// TODO
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct rasterizer_state
{
	GLenum face_cull_mode = GL_NONE;
	bool front_ccw = false;
	bool wireframe = false;
	bool depth_clamp = false;
	bool scissor_test = false;

	void bind();
};

struct blend_state
{
	struct slot_desc
	{
		GLenum src = GL_ONE;
		GLenum dst = GL_ZERO;
		GLenum op = detail::gl_api::FUNC_ADD;
		GLenum src_alpha = GL_ONE;
		GLenum dst_alpha = GL_ZERO;
		GLenum op_alpha = GL_NONE;
	} slot[8];

	std::bitset<8> enabled_slots = { 0 };

	void bind();
};

struct depth_stencil_state
{
	GLenum depth_func = GL_LESS;
	uint8_t stencil_read_mask = 0;
	uint8_t stencil_write_mask = 0;
	bool stencil_test = false;
	bool depth_test = true;
	bool depth_write = true;

	void bind();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context3d
{
public:
  context3d();
  virtual ~context3d() { }

  void clear();

	template <typename T>
	bool bind_geometry(std::shared_ptr<geometry<T>> geom)
	{
		if (!geom)
		{
			if (_vertexBuffer) { _vertexBuffer->unbind(); _vertexBuffer = nullptr; }
			if (_indexBuffer) { _indexBuffer->unbind(); _indexBuffer = nullptr; }
			return true;
		}

		bool result = true;

		if (_vertexBuffer = geom->vertices())
			result = result && _vertexBuffer->bind();

		if (_indexBuffer = geom->indices())
			result = result && _indexBuffer->bind();

		return result;
	}

  bool bind_program(program::ptr prog);
  bool bind_texture(texture::ptr tex, int slot = 0);

	GLint get_uniform_location(std::string_view name) const;

  bool set_uniform(std::string_view name, int value);
  bool set_uniform(std::string_view name, const vec2 &value);
  bool set_uniform(std::string_view name, const mat4 &value);
  bool set_uniform(std::string_view name, texture::ptr value);
    
  int get_free_texture_slot() const { for (int i = 0; i < 16; ++i) if (!_textures[i]) return i; return -1; }

  bool draw(GLenum primitive = GL_TRIANGLES, size_t offset = 0, size_t length = static_cast<size_t>(-1));

private:
  program::ptr _basicProgram;

	detail::buffer::ptr _vertexBuffer;
	index_buffer::ptr _indexBuffer;
	program::ptr _program;
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
bool buffer::bind()
{
	if (dirty())
	{
		if (!_buffer.id) gl.GenBuffers(1, &_buffer.id);
		gl.BindBuffer(_type, _buffer.id);
		gl.BufferData(_type, _size, _data, gl.STREAM_DRAW);

		if (_owner && !_keepData) { delete[] _data; _data = nullptr; }
		set_dirty(false);
	}
	else
		gl.BindBuffer(_type, _buffer.id);

	return _buffer.id > 0;
}

//------------------------------------------------------------------------------------------------------------------------
void buffer::unbind()
{
	gl.BindBuffer(_type, 0);
}

}

//---------------------------------------------------------------------------------------------------------------------
size_t program::compile_permutation(std::string_view macros)
{
	// TODO
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
bool program::bind(size_t permutation)
{
	if (dirty())
	{
		std::string macroString = get_macro_string();

		if (_computeSource.empty())
		{
			_computeShader.destroy();

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
		}
		else
		{
			_vertShader.destroy();
			_geomShader.destroy();
			_fragShader.destroy();
			
			_computeShader.compile(gl.COMPUTE_SHADER, macroString + _computeSource);
			_program.link({ _computeShader });
		}

		set_dirty(false);

	}

	gl.UseProgram(_program.id);
	return _program.id != 0;
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
      _pbo->bind();
      auto desc = detail::gl_format_descriptor::get(_format);
      if (_type == GL_TEXTURE_1D || _type == GL_TEXTURE_2D)
      {
        for (auto &&p : _parts)
          glTexImage2D(_type, static_cast<GLint>(p.mip_level), desc.layout, p.size.x, p.size.y, 0, _format, desc.element_format, reinterpret_cast<const GLvoid *>(p.offset));
      }
      _pbo->unbind();
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

//---------------------------------------------------------------------------------------------------------------------
void rasterizer_state::bind()
{
	glFrontFace(front_ccw ? GL_CCW : GL_CW);
	face_cull_mode != GL_NONE ? (glEnable(GL_CULL_FACE), glCullFace(face_cull_mode)) : glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	depth_clamp ? glEnable(gl.DEPTH_CLAMP) : glDisable(gl.DEPTH_CLAMP);
	scissor_test ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
}

//---------------------------------------------------------------------------------------------------------------------
void blend_state::bind()
{
	for (GLuint i = 0; i < 8; ++i)
	{
		enabled_slots[i] ? gl.Enablei(GL_BLEND, i) : gl.Disablei(GL_BLEND, i);
		gl.BlendFunci(i, slot[i].src, slot[i].dst);
		gl.BlendEquationi(i, slot[i].op);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void depth_stencil_state::bind()
{
	depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	stencil_test ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
	glDepthMask(depth_write ? GL_TRUE : GL_FALSE);
	glDepthFunc(depth_func);
}

//------------------------------------------------------------------------------------------------------------------------
context3d::context3d()
{
	_basicProgram = std::make_shared<program>();
  _basicProgram->set_vert_source(detail::vertex_shader_code3d);
  _basicProgram->set_frag_source(detail::fragment_shader_code3d);
}

//------------------------------------------------------------------------------------------------------------------------
void context3d::clear()
{
	if (_vertexBuffer) _vertexBuffer->unbind();
	_vertexBuffer = nullptr;

	if (_indexBuffer) _indexBuffer->unbind();
	_indexBuffer = nullptr;

  if (_program) _program->unbind();
  _program = nullptr;

  for (auto &texture : _textures)
    texture = nullptr;

  bind_program(_basicProgram);
  glEnable(GL_DEPTH_TEST);
}

//------------------------------------------------------------------------------------------------------------------------
bool context3d::bind_program(program::ptr prog)
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
  if (!_vertexBuffer) return false;

	auto numElements = _vertexBuffer->size_elements();
	if (offset >= numElements) return false;

	if (offset + length > numElements)
		length = numElements - offset;

  glDrawArrays(primitive, static_cast<GLint>(offset), static_cast<GLsizei>(length));
  return true;
}

}

#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
