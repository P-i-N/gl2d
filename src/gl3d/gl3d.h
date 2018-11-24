#ifndef __GL3D_H__
#define __GL3D_H__

#include <vector>
#include <memory>

#include <filesystem>

#include "gl3d_base.h"
#include "gl3d_math.h"

#define GL3D_LAYOUT(...) \
	static const gl3d::detail::layout &get_layout() { \
		static gl3d::detail::layout layout { __VA_ARGS__ }; \
		return layout; }

namespace gl3d {

namespace detail {

struct gl_api
{
#if defined(WIN32)
#define GL_PROC(_Returns, _Name, ...) proc_wrapper<_Returns __stdcall ( __VA_ARGS__ )> _Name { "gl" #_Name };
	proc_wrapper<void *__stdcall ( void *, void *, const int * )> CreateContextAttribsARB { "wglCreateContextAttribsARB" };
#else
#define GL_PROC(_Returns, _Name, ...) proc_wrapper<_Returns ( __VA_ARGS__ )> _Name { "gl" #_Name };
#endif

	// *INDENT-OFF*
	GL_PROC(     void, Enablei, gl_enum, unsigned )
	GL_PROC(     void, Disablei, gl_enum, unsigned )
	GL_PROC(     void, BlendFunci, unsigned, gl_enum, gl_enum )
	GL_PROC(     void, BlendEquationi, unsigned, gl_enum )
	GL_PROC(     void, GetIntegerv, gl_enum, int *)

	/// Shaders and programs
	GL_PROC(unsigned, CreateShader, gl_enum)
	GL_PROC(    void, DeleteShader, unsigned)
	GL_PROC(    void, ShaderSource, unsigned, int, const char *const *, const int *)
	GL_PROC(    void, CompileShader, unsigned)
	GL_PROC(    void, GetShaderiv, unsigned, gl_enum, int *)
	GL_PROC(    void, GetShaderInfoLog, unsigned, int, int *, char *)
	GL_PROC(    void, AttachShader, unsigned, unsigned)
	GL_PROC(    void, DetachShader, unsigned, unsigned)
	GL_PROC(unsigned, CreateProgram)
	GL_PROC(    void, DeleteProgram, unsigned)
	GL_PROC(    void, LinkProgram, unsigned)
	GL_PROC(    void, UseProgram, unsigned)
	GL_PROC(    void, GetProgramiv, unsigned, gl_enum, int *)

	/// Uniforms
	GL_PROC( int, GetUniformLocation, unsigned, const char *)
	GL_PROC(void, Uniform1i, int, int)
	GL_PROC(void, Uniform1f, int, float)
	GL_PROC(void, Uniform2fv, int, int, const float *)
	GL_PROC(void, UniformMatrix4fv, int, int, unsigned char, const float *)

	/// Buffers
	GL_PROC(   void, CreateBuffers, int, unsigned *)
	GL_PROC(   void, DeleteBuffers, int, const unsigned *)
	GL_PROC(   void, NamedBufferData, unsigned, int, const void *, gl_enum)
	GL_PROC(   void, NamedBufferStorage, unsigned, int, const void *, unsigned)
	GL_PROC( void *, MapNamedBuffer, unsigned, unsigned)
	GL_PROC( void *, MapNamedBufferRange, unsigned, ptrdiff_t, unsigned, unsigned)
	GL_PROC(uint8_t, UnmapNamedBuffer, unsigned)
	GL_PROC(   void, FlushMappedNamedBufferRange, unsigned, ptrdiff_t, unsigned)
	GL_PROC(   void, BindBufferRange, gl_enum, unsigned, unsigned, ptrdiff_t, size_t)

	/// Vertex array objects
	GL_PROC(void, CreateVertexArrays, int, unsigned *)
	GL_PROC(void, DeleteVertexArrays, int, const unsigned *)
	GL_PROC(void, BindVertexArray, unsigned)
	GL_PROC(void, EnableVertexArrayAttrib, unsigned, unsigned)
	GL_PROC(void, DisableVertexArrayAttrib, unsigned, unsigned)
	GL_PROC(void, VertexArrayAttribFormat, unsigned, unsigned, int, gl_enum, unsigned char, unsigned)
	GL_PROC(void, VertexArrayAttribBinding, unsigned, unsigned, unsigned)
	GL_PROC(void, VertexArrayVertexBuffer, unsigned, unsigned, unsigned, const void *, int)

	/// Textures
	GL_PROC(void, CreateTextures, gl_enum, unsigned, unsigned *)
	GL_PROC(void, TextureParameteri, unsigned, gl_enum, int)
	GL_PROC(void, TextureStorage2D, unsigned, unsigned, gl_format, unsigned, unsigned)
	GL_PROC(void, TextureSubImage2D, unsigned, int, int, int, unsigned, unsigned, gl_enum, gl_enum, const void *)
	GL_PROC(void, BindTextureUnit, unsigned, unsigned)

	// *INDENT-ON*
};

#undef GL_PROC

} // namespace gl3d::detail

extern detail::gl_api gl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class gl_enum : unsigned
{
	NONE = 0, ZERO = 0,
	ONE = 1,

	POINTS = 0x0000, LINES,
	LINE_STRIP = 0x0003, TRIANGLES, TRIANGLE_STRIP,

	TEXTURE_1D = 0x0DE0, TEXTURE_2D,
	TEXTURE_3D = 0x806F,
	TEXTURE_2D_ARRAY = 0x8C1A,
	TEXTURE_CUBE_MAP = 0x8513,
	TEXTURE_CUBE_MAP_ARRAY = 0x9009,

	TEXTURE_CUBE_MAP_POSITIVE_X_EXT = 0x8515, TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
	TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,

	BYTE = 0x1400, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT, FLOAT,
	DOUBLE = 0x140A,

	RED = 0x1903,
	RG = 0x8227, RGB = 0x1907, BGR = 0x80E0, BGRA, RGBA = 0x1908, STENCIL_INDEX = 0x1901, DEPTH_COMPONENT,

	NEVER = 0x0200, LESS, EQUAL, LEQUAL, GREATER, NOTEQUAL, GEQUAL, ALWAYS,

	SRC_COLOR = 0x0300, ONE_MINUS_SRC_COLOR, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, DST_ALPHA, ONE_MINUS_DST_ALPHA,
	DST_COLOR = 0x0306, ONE_MINUS_DST_COLOR, SRC_ALPHA_SATURATE,

	CW = 0x0900, CCW,

	TEXTURE0 = 0x84C0,

	ARRAY_BUFFER = 0x8892, ELEMENT_ARRAY_BUFFER,

	STREAM_DRAW = 0x88E0, STREAM_READ, STREAM_COPY,
	STATIC_DRAW = 0x88E4, STATIC_READ, STATIC_COPY,
	DYNAMIC_DRAW = 0x88E8, DYNAMIC_READ, DYNAMIC_COPY,

	FRAGMENT_SHADER = 0x8B30, VERTEX_SHADER,
	GEOMETRY_SHADER = 0x8DD9,
	COMPUTE_SHADER = 0x91B9,

	READ_ONLY = 0x88B8, WRITE_ONLY, READ_WRITE,

	UNIFORM_BUFFER = 0x8A11,
	UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34,

	FLOAT_VEC2 = 0x8B50, FLOAT_VEC3, FLOAT_VEC4, INT_VEC2, INT_VEC3, INT_VEC4, BOOL,
	FLOAT_MAT2 = 0x8B5A, FLOAT_MAT3, FLOAT_MAT4,

	COMPILE_STATUS = 0x8B81, LINK_STATUS, VALIDATE_STATUS, INFO_LOG_LENGTH,
	CURRENT_PROGRAM = 0x8B8D,

	MAP_READ_BIT = 0x0001,
	MAP_WRITE_BIT = 0x0002,
	MAP_PERSISTENT_BIT = 0x0040,
	MAP_COHERENT_BIT = 0x0080,
	DYNAMIC_STORAGE_BIT = 0x0100,
	CLIENT_STORAGE_BIT = 0x0200,

#if defined(WIN32)
	CONTEXT_MAJOR_VERSION = 0x2091, CONTEXT_MINOR_VERSION,

	CONTEXT_PROFILE_MASK = 0x9126,
	CONTEXT_CORE_PROFILE_BIT = 0x0001,
#endif
};

GL3D_ENUM_PLUS( gl_enum )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class gl_format : unsigned
{
	NONE = 0,
	RGB8 = 0x8051,
	RGBA8 = 0x8058,

	R8 = 0x8229, R16, RG8, RG16,
	R16F, R32F, RG16F, RG32F,
	R8I, R8UI, R16I, R16UI, R32I, R32UI,
	RG8I, RG8UI, RG16I, RG16UI, RG32I, RG32UI,
};

GL3D_ENUM_PLUS( gl_format )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
struct layout
{
	struct attr
	{
		unsigned location, offset, element_count;
		gl_enum element_type;
	};

	std::vector<attr> attribs;
	unsigned mask = 0;
	unsigned stride = 0;

	template <typename... Args>
	layout( Args &&... args ) : attribs( sizeof...( Args ) / 2 ) { init( 0, args... ); }

private:
	template <typename T> struct type { };

	void fill( attr &a, unsigned loc, unsigned off, type<int> ) { a = { loc, off, 1, gl_enum::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<float> ) { a = { loc, off, 1, gl_enum::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec2> ) { a = { loc, off, 2, gl_enum::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec2> ) { a = { loc, off, 2, gl_enum::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec3> ) { a = { loc, off, 3, gl_enum::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec3> ) { a = { loc, off, 3, gl_enum::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec4> ) { a = { loc, off, 4, gl_enum::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec4> ) { a = { loc, off, 4, gl_enum::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<byte_vec4> ) { a = { loc, off, 4, gl_enum::UNSIGNED_BYTE }; }

	template <typename T1, typename T2, typename... Args>
	void init( unsigned index, unsigned location, T1 T2::*member, Args &&... args )
	{
		fill( attribs[index], location, unsigned( size_t( &( ( ( T2 * )0 )->*member ) ) ), type<T1>() );
		mask |= ( 1u << location );
		stride = sizeof( T2 );

		if constexpr ( sizeof...( Args ) >= 2 )
			init( index + 1, args... );
	}
};

//---------------------------------------------------------------------------------------------------------------------
class basic_object
{
public:
	using ptr = std::shared_ptr<basic_object>;
};

//---------------------------------------------------------------------------------------------------------------------
class gl_object : public basic_object
{
public:
	using ptr = std::shared_ptr<gl_object>;

	unsigned id() const { return _id; }

protected:
	gl_object() = default;

	unsigned _id = 0;
};

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
enum class buffer_usage
{
	immutable,
	dynamic,
	persistent,
	persistent_coherent
};

//---------------------------------------------------------------------------------------------------------------------
class buffer : public detail::gl_object
{
public:
	using ptr = std::shared_ptr<buffer>;

	template <typename... Args>
	static ptr create( Args &&... args ) { return std::make_shared<buffer>( args... ); }

	buffer( buffer_usage usage, const void *data, size_t size, bool makeCopy = true );

	buffer( buffer_usage usage, const detail::raw_data_range &initialData, bool makeCopy = true )
		: buffer( usage, initialData.data, initialData.size, makeCopy )
	{

	}

	virtual ~buffer();

	buffer_usage usage() const { return _usage; }

	size_t size() const { return _size; }

	void synchronize();

	void *map() const;

	void unmap() const;

protected:
	buffer_usage _usage;
	uint8_t *_data = nullptr;
	size_t _size = 0;
	bool _owner = false;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
enum class shader_stage { vertex, geometry, fragment, compute, __count };
GL3D_ENUM_PLUS( shader_stage )

//---------------------------------------------------------------------------------------------------------------------
class shader_code
{
public:
	using ptr = std::shared_ptr<shader_code>;

	const std::filesystem::path &path() const { return _path; }
	const std::string &source() const { return _source; }
	const std::string &unrolled_source() const { return _unrolledSource; }

	const std::string &stage_source( shader_stage stage ) const { return _stageSources[+stage]; }

	bool source( std::string_view sourceCode, const std::filesystem::path &cwd = std::filesystem::path() );
	bool load( std::istream &is, const std::filesystem::path &cwd = std::filesystem::path() );
	bool load( const std::filesystem::path &path );

protected:
	std::filesystem::path _path;
	std::string _source;
	std::string _unrolledSource;
	std::string _stageSources[+shader_stage::__count];
};

//---------------------------------------------------------------------------------------------------------------------
class shader : public detail::gl_object
{
public:
	using ptr = std::shared_ptr<shader>;

	template <typename... Args>
	static ptr create( Args &&... args ) { return std::make_shared<shader>( args... ); }

	shader( shader_code::ptr code, std::string_view defines = std::string_view() );
	virtual ~shader();

	void clear();
	bool compile();

protected:
	shader_code::ptr _shaderCode;
	std::string _defines;
	unsigned _stageIDs[+shader_stage::__count] = { 0, 0, 0, 0 };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class texture : public detail::gl_object
{
public:
	using ptr = std::shared_ptr<texture>;

	template <typename... Args>
	static ptr create( Args &&... args ) { return std::make_shared<texture>( args... ); }

	texture( gl_enum type, gl_format format, const uvec4 &dimensions, bool hasMips = false );

	texture( gl_format format, const uvec2 &dimensions, bool hasMips = false )
		: texture( gl_enum::TEXTURE_2D, format, { dimensions.x, dimensions.y, 1, 1 }, hasMips )
	{

	}

	struct part
	{
		unsigned layer = 0;
		unsigned mip_level = 0;
		const void *data = nullptr;
	};

	texture( gl_enum type, gl_format format, const uvec4 &dimensions,
	         const detail::type_range<part> &parts,
	         bool buildMips = true, bool makeCopy = true );

	texture( gl_format format, const uvec2 &dimensions,
	         const detail::type_range<part> &parts,
	         bool buildMips = true, bool makeCopy = true )
		: texture( gl_enum::TEXTURE_2D, format, { dimensions.x, dimensions.y, 1, 1 }, parts, buildMips, makeCopy )
	{

	}

	texture( gl_format format, const uvec2 &dimensions,
	         const void *data,
	         bool buildMips = true, bool makeCopy = true )
		: texture( format, dimensions, part{ 0, 0, data }, buildMips, makeCopy )
	{

	}

	virtual ~texture();

	gl_enum type() const { return _type; }
	gl_format format() const { return _format; }
	const uvec4 &dimensions() const { return _dimensions; }

	unsigned width( unsigned mipLevel = 0 ) const { return maximum( 1, _dimensions.x >> mipLevel ); }
	unsigned height( unsigned mipLevel = 0 ) const { return maximum( 1, _dimensions.y >> mipLevel ); }
	unsigned depth( unsigned mipLevel = 0 ) const { return maximum( 1, _dimensions.z >> mipLevel ); }
	unsigned array_size() const { return _dimensions.w; }

	void synchronize();

protected:
	void clear();

	gl_enum _type = gl_enum::NONE;
	gl_format _format = gl_format::NONE;
	uvec4 _dimensions;

	std::unique_ptr<part[]> _parts;
	unsigned _numParts = 0;
	bool _owner = false;
	bool _buildMips = false;

	buffer::ptr _buffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
struct blend_state
{
	uint8_t mask = 0xFFu;
};

//---------------------------------------------------------------------------------------------------------------------
struct depth_stencil_state
{
	gl_enum depth_func = gl_enum::LESS;
	uint8_t stencil_read_mask = 0;
	uint8_t stencil_write_mask = 0;
	unsigned stencil_test : 1;
	unsigned depth_test : 1;
	unsigned depth_write : 1;
};

//---------------------------------------------------------------------------------------------------------------------
struct rasterizer_state
{
	gl_enum face_cull_mode = gl_enum::NONE;
	unsigned front_ccw : 1;
	unsigned wireframe : 1;
	unsigned depth_camp : 1;
	unsigned scissor_test : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class cmd_queue : public detail::basic_object
{
public:
	using ptr = std::shared_ptr<cmd_queue>;

	template <typename... Args>
	static ptr create() { return std::make_shared<cmd_queue>(); }

	cmd_queue(): cmd_queue( nullptr ) { }
	virtual ~cmd_queue();

	bool recording() const { return _recording; }

	void reset();

	void clear_color( const vec4 &color );
	void clear_depth( float depth );

	void update_texture( texture::ptr tex, const void *data, unsigned layer = 0, unsigned mipLevel = 0, size_t rowStride = 0 );

	void bind_state( const blend_state &bs );
	void bind_state( const depth_stencil_state &ds );
	void bind_state( const rasterizer_state &rs );

	void bind_shader( shader::ptr sh );
	void bind_vertex_buffer( buffer::ptr vertices, const detail::layout &layout, size_t offset = 0 );
	void bind_vertex_attribute( buffer::ptr attribs, unsigned slot, gl_enum glType, size_t offset = 0, size_t stride = 0 );
	void bind_index_buffer( buffer::ptr indices, bool use16bits, size_t offset = 0 );

	void bind_texture( texture::ptr tex, unsigned slot );
	void bind_render_target( texture::ptr tex, unsigned slot, unsigned layer = 0, unsigned mipLevel = 0 );

	void set_uniform_block( const detail::location_variant &location, const void *data, size_t size );

	template <typename T>
	void set_uniform_block( const detail::location_variant &location, const T &block )
	{
		set_uniform_block( location, &block, sizeof( T ) );
	}

	void set_uniform( const detail::location_variant &location, bool value );
	void set_uniform( const detail::location_variant &location, int value );
	void set_uniform( const detail::location_variant &location, float value );

	void draw( gl_enum primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );
	void draw_indexed( gl_enum primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );

	void execute( ptr cmdQueue );

protected:
	struct gl_state
	{
		buffer::ptr uniform_block_buffer;
		size_t uniform_block_cursor = 0;
		int uniform_block_alignment = 0;
	};

	cmd_queue( gl_state *state );

	template <typename... Args> static constexpr size_t types_size() { return ( sizeof( Args ) + ... + 0 ); }

	template <typename H, typename... T>
	void write( H &&head, T &&... tail )
	{
		auto len = types_size<H, T...>();
		if ( _position + len > _recordedData.size() )
			_recordedData.resize( _position + len );

		write_value( _recordedData.data() + _position, head, tail... );
		_position += len;
	}

	void write_data( const void *data, size_t size )
	{
		auto len = sizeof( unsigned ) + size;
		if ( _position + len > _recordedData.size() )
			_recordedData.resize( _position + len );

		memcpy( _recordedData.data() + _position, &size, sizeof( unsigned ) );
		memcpy( _recordedData.data() + _position + sizeof( unsigned ), data, size );
		_position += len;
	}

	template <typename H, typename... T>
	void write_value( uint8_t *cursor, H &&head, T &&... tail )
	{
		memcpy( cursor, &head, sizeof( H ) );
		if constexpr ( sizeof...( T ) > 0 )
			write_value( cursor + sizeof( H ), tail... );
	}

	void write_location_variant( const detail::location_variant &location )
	{
		if ( location.holds_name() )
		{
			auto size = location.size();
			auto len = sizeof( unsigned ) + size + 1;
			if ( _position + len > _recordedData.size() )
				_recordedData.resize( _position + len );

			memcpy( _recordedData.data() + _position, &location.size_or_id, sizeof( unsigned ) );
			_position += sizeof( unsigned );
			memcpy( _recordedData.data() + _position, location.data, size );
			_recordedData[_position + size++] = 0;
			_position += size;
		}
		else
			write( location.size_or_id );
	}

	template <typename T>
	const T &read()
	{
		_position += sizeof( T );
		return *reinterpret_cast<const T *>( _recordedData.data() + _position - sizeof( T ) );
	}

	template <typename T> void read( T &value ) { value = read<T>(); }

	std::pair<const void *, size_t> read_data()
	{
		auto size = read<unsigned>();
		auto data = _recordedData.data() + _position;
		_position += size;

		return { data, size };
	}

	detail::location_variant read_location_variant()
	{
		auto size_or_id = read<unsigned>();
		if ( size_or_id & 0x80000000u )
		{
			auto data = reinterpret_cast<const char *>( _recordedData.data() + _position );
			_position += ( size_or_id & 0x7FFFFFFFu ) + 1;
			return { data, size_or_id };
		}

		return static_cast<int>( size_or_id ) - 1;
	}

	void execute( gl_state *state );

	bool _recording = true;
	std::vector<uint8_t> _recordedData;
	std::vector<detail::basic_object::ptr> _resources;
	size_t _position = 0;
	gl_state *_state = nullptr;

	enum class cmd_type
	{
		clear_color, clear_depth,
		update_texture,
		bind_blend_state, bind_depth_stencil_state, bind_rasterizer_state,
		bind_shader, bind_vertex_buffer, bind_vertex_atrribute, bind_index_buffer,
		bind_texture, bind_render_target,
		set_uniform_block, set_uniform,
		draw, draw_indexed,
		execute,
	};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
class context : public cmd_queue
{
public:
	using ptr = std::shared_ptr<context>;

	context( void *windowNativeHandle );
	context( ptr sharedContext );
	virtual ~context();

	void *window_native_handle() const { return _window_native_handle; }
	void *native_handle() const { return _native_handle; }

	void make_current();

	unsigned bind_vao( buffer::ptr vb, const detail::layout &layout, size_t offset );

protected:
	void *_window_native_handle = nullptr;
	void *_native_handle = nullptr;

	gl_state _glState;
	std::unordered_map<std::uintptr_t, unsigned> _layoutVAOs;
};

//---------------------------------------------------------------------------------------------------------------------
class async_upload_context
{
public:
	using ptr = std::shared_ptr<async_upload_context>;

	async_upload_context( context::ptr mainContext );
	virtual ~async_upload_context();

protected:
	context::ptr _context;
};

} // namespace gl3d::detail

} // namespace gl3d

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
	#ifndef __GL3D_H_IMPL__
		#define __GL3D_H_IMPL__
		#include "gl3d.inl"
	#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
