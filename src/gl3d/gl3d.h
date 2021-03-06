#ifndef __GL3D_H__
#define __GL3D_H__

#include <initializer_list>
#include <vector>
#include <memory>

#include <filesystem>

#include "gl3d_base.h"
#include "gl3d_math.h"

#define GL3D_LAYOUT(...) \
	static const gl3d::detail::layout &layout() { \
		static gl3d::detail::layout l { __VA_ARGS__ }; \
		return l; }

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
	GL_PROC(    void, GetProgramInfoLog, unsigned, int, int *, char *)
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
	GL_PROC(void, Uniform2fv, int, unsigned, const float *)
	GL_PROC(void, Uniform3fv, int, unsigned, const float *)
	GL_PROC(void, Uniform3uiv, int, unsigned, const unsigned *)
	GL_PROC(void, Uniform4iv, int, unsigned, const int *)
	GL_PROC(void, Uniform4fv, int, unsigned, const float *)
	GL_PROC(void, UniformMatrix3fv, int, unsigned, unsigned char, const float *)
	GL_PROC(void, UniformMatrix4fv, int, unsigned, unsigned char, const float *)
	GL_PROC(void, UniformHandleui64ARB, int, uint64_t)
	GL_PROC(void, UniformHandleui64vARB, int, unsigned, const uint64_t *)

	/// Buffers
	GL_PROC(   void, CreateBuffers, unsigned, unsigned *)
	GL_PROC(   void, DeleteBuffers, unsigned, const unsigned *)
	GL_PROC(   void, NamedBufferData, unsigned, int, const void *, gl_enum)
	GL_PROC(   void, NamedBufferStorage, unsigned, int, const void *, unsigned)
	GL_PROC( void *, MapNamedBuffer, unsigned, unsigned)
	GL_PROC( void *, MapNamedBufferRange, unsigned, ptrdiff_t, unsigned, unsigned)
	GL_PROC(uint8_t, UnmapNamedBuffer, unsigned)
	GL_PROC(   void, FlushMappedNamedBufferRange, unsigned, ptrdiff_t, unsigned)
	GL_PROC(   void, BindBuffer, gl_enum, unsigned)
	GL_PROC(   void, BindBufferRange, gl_enum, unsigned, unsigned, ptrdiff_t, size_t)

	/// Vertex array objects
	GL_PROC(void, CreateVertexArrays, unsigned, unsigned *)
	GL_PROC(void, DeleteVertexArrays, unsigned, const unsigned *)
	GL_PROC(void, BindVertexArray, unsigned)
	GL_PROC(void, EnableVertexArrayAttrib, unsigned, unsigned)
	GL_PROC(void, DisableVertexArrayAttrib, unsigned, unsigned)
	GL_PROC(void, VertexArrayAttribFormat, unsigned, unsigned, int, gl_type, unsigned char, unsigned)
	GL_PROC(void, VertexArrayAttribIFormat, unsigned, unsigned, int, gl_type, unsigned)
	GL_PROC(void, VertexArrayAttribBinding, unsigned, unsigned, unsigned)
	GL_PROC(void, VertexArrayVertexBuffer, unsigned, unsigned, unsigned, const void *, int)
	GL_PROC(void, VertexArrayElementBuffer, unsigned, unsigned)

	/// Textures
	GL_PROC(    void, CreateTextures, gl_enum, unsigned, unsigned *)
	GL_PROC(    void, TextureParameteri, unsigned, gl_enum, int)
	GL_PROC(    void, TextureParameterf, unsigned, gl_enum, float)
	GL_PROC(    void, TextureStorage2D, unsigned, unsigned, gl_internal_format, unsigned, unsigned)
	GL_PROC(    void, TextureSubImage2D, unsigned, int, int, int, unsigned, unsigned, gl_format, gl_type, const void *)
	GL_PROC(    void, BindTextureUnit, unsigned, unsigned)
	GL_PROC(uint64_t, GetTextureHandleARB, unsigned)
	GL_PROC(    void, MakeTextureHandleResidentARB, uint64_t)
	GL_PROC(    void, MakeTextureHandleNonResidentARB, uint64_t)
	GL_PROC(    void, GenerateTextureMipmap, unsigned)

	/// Frame buffer objects
	GL_PROC(   void, CreateFramebuffers, unsigned, unsigned *)
	GL_PROC(   void, DeleteFramebuffers, unsigned, const unsigned *)
	GL_PROC(   void, BindFramebuffer, gl_enum, unsigned)
	GL_PROC(   void, NamedFramebufferTexture, unsigned, gl_enum, unsigned, int)
	GL_PROC(   void, NamedFramebufferTextureLayer, unsigned, gl_enum, unsigned, int, int)
	GL_PROC(gl_enum, CheckNamedFramebufferStatus, unsigned, gl_enum)

	/// Draw calls
	GL_PROC(void, DrawArraysInstancedBaseInstance, gl_enum, int, unsigned, unsigned, unsigned)
	GL_PROC(void, DrawElementsInstancedBaseInstance, gl_enum, unsigned, gl_type, const void *, unsigned, unsigned)
	GL_PROC(void, MultiDrawArraysIndirect, gl_enum, const void *, unsigned, unsigned)
	GL_PROC(void, MultiDrawElementsIndirect, gl_enum, gl_type, const void *, unsigned, unsigned)

	// *INDENT-ON*
};

#undef GL_PROC

} // namespace gl3d::detail

GL3D_API extern detail::gl_api gl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class gl_enum : unsigned
{
	NONE = 0, ZERO = 0,
	ONE = 1,

	POINTS = 0x0000, LINES,
	LINE_STRIP = 0x0003, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, QUADS,

	TEXTURE = 0x1702,

	TEXTURE_1D = 0x0DE0, TEXTURE_2D,
	TEXTURE_3D = 0x806F,
	TEXTURE_2D_ARRAY = 0x8C1A,
	TEXTURE_CUBE_MAP = 0x8513,
	TEXTURE_CUBE_MAP_ARRAY = 0x9009,

	TEXTURE_CUBE_MAP_POSITIVE_X_EXT = 0x8515, TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
	TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,

	CLAMP = 0x2900, REPEAT,
	CLAMP_TO_EDGE = 0x812F,
	MIRRORED_REPEAT = 0x8370,

	NEAREST = 0x2600, LINEAR,
	NEAREST_MIPMAP_NEAREST = 0x2700, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR,

	TEXTURE_MAG_FILTER = 0x2800, TEXTURE_MIN_FILTER, TEXTURE_WRAP_S, TEXTURE_WRAP_T,
	TEXTURE_WRAP_R = 0x8072,

	NEVER = 0x0200, LESS, EQUAL, LEQUAL, GREATER, NOTEQUAL, GEQUAL, ALWAYS,

	SRC_COLOR = 0x0300, ONE_MINUS_SRC_COLOR, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, DST_ALPHA, ONE_MINUS_DST_ALPHA,
	DST_COLOR = 0x0306, ONE_MINUS_DST_COLOR, SRC_ALPHA_SATURATE,

	FRONT = 0x0404, BACK,
	CW = 0x0900, CCW,

	TEXTURE0 = 0x84C0,
	COLOR_ATTACHMENT0 = 0x8CE0,
	DEPTH_STENCIL_ATTACHMENT = 0x821A,
	DEPTH_ATTACHMENT = 0x8D00,
	STENCIL_ATTACHMENT = 0x8D20,

	READ_FRAMEBUFFER = 0x8CA8, DRAW_FRAMEBUFFER,
	FRAMEBUFFER_COMPLETE = 0x8CD5,
	FRAMEBUFFER = 0x8D40,

	ARRAY_BUFFER = 0x8892, ELEMENT_ARRAY_BUFFER,

	STREAM_DRAW = 0x88E0, STREAM_READ, STREAM_COPY,
	STATIC_DRAW = 0x88E4, STATIC_READ, STATIC_COPY,
	DYNAMIC_DRAW = 0x88E8, DYNAMIC_READ, DYNAMIC_COPY,

	FRAGMENT_SHADER = 0x8B30, VERTEX_SHADER,
	GEOMETRY_SHADER = 0x8DD9,
	COMPUTE_SHADER = 0x91B9,

	TEXTURE_MAX_ANISOTROPY = 0x84FE,
	VERTEX_ARRAY_BINDING = 0x85B5,

	DEPTH_CLAMP = 0x864F,

	READ_ONLY = 0x88B8, WRITE_ONLY, READ_WRITE,

	UNIFORM_BUFFER = 0x8A11,
	UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34,

	COMPILE_STATUS = 0x8B81, LINK_STATUS, VALIDATE_STATUS, INFO_LOG_LENGTH,
	CURRENT_PROGRAM = 0x8B8D,

	DRAW_INDIRECT_BUFFER = 0x8F3F,
	SHADER_STORAGE_BUFFER = 0x90D2,

	MAP_READ_BIT = 0x0001,
	MAP_WRITE_BIT = 0x0002,
	MAP_INVALIDATE_RANGE_BIT = 0x0004,
	MAP_INVALIDATE_BUFFER_BIT = 0x0008,
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

enum class gl_internal_format : unsigned
{
	NONE = 0,
	RGB8 = 0x8051,
	RGBA8 = 0x8058,

	R8 = 0x8229, R16, RG8, RG16,
	R16F, R32F, RG16F, RG32F,
	R8I, R8UI, R16I, R16UI, R32I, R32UI,
	RG8I, RG8UI, RG16I, RG16UI, RG32I, RG32UI,

	DEPTH_COMPONENT32F = 0x8CAC,
	DEPTH24_STENCIL8 = 0x88F0,
	DEPTH32F_STENCIL8 = 0x8CAD,
};

GL3D_ENUM_PLUS( gl_internal_format )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class gl_format : unsigned
{
	NONE = 0,
	RED = 0x1903,
	RG = 0x8227, RGB = 0x1907, BGR = 0x80E0, BGRA, RGBA = 0x1908, STENCIL_INDEX = 0x1901, DEPTH_COMPONENT,
	DEPTH_STENCIL = 0x84F9,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class gl_type : unsigned
{
	NONE = 0,

	BYTE = 0x1400, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT, FLOAT,
	DOUBLE = 0x140A,
	UNSIGNED_INT64 = 0x140F,

	FLOAT_VEC2 = 0x8B50, FLOAT_VEC3, FLOAT_VEC4, INT_VEC2, INT_VEC3, INT_VEC4, BOOL,
	FLOAT_MAT2 = 0x8B5A, FLOAT_MAT3, FLOAT_MAT4,

	UNSIGNED_INT_VEC2 = 0x8DC6, UNSIGNED_INT_VEC3, UNSIGNED_INT_VEC4,

	FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
	UNSIGNED_INT_24_8 = 0x84FA,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
struct layout
{
	struct attr
	{
		unsigned location, offset, element_count;
		gl_type element_type;
		bool is_integer;
	};

	std::vector<attr> attribs;
	unsigned mask = 0;
	unsigned stride = 0;

	template <typename... Args>
	layout( Args &&... args ) : attribs( sizeof...( Args ) / 2 ) { init( 0, args... ); }

private:
	template <typename T> struct type { };

	void fill( attr &a, unsigned loc, unsigned off, type<int> ) { a = { loc, off, 1, gl_type::INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<float> ) { a = { loc, off, 1, gl_type::FLOAT, false }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec2> ) { a = { loc, off, 2, gl_type::FLOAT, false }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec2> ) { a = { loc, off, 2, gl_type::INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec3> ) { a = { loc, off, 3, gl_type::FLOAT, false }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec3> ) { a = { loc, off, 3, gl_type::INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<uvec3> ) { a = { loc, off, 3, gl_type::UNSIGNED_INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec4> ) { a = { loc, off, 4, gl_type::FLOAT, false }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec4> ) { a = { loc, off, 4, gl_type::INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<uvec4> ) { a = { loc, off, 4, gl_type::UNSIGNED_INT, true }; }
	void fill( attr &a, unsigned loc, unsigned off, type<byte_vec4> ) { a = { loc, off, 4, gl_type::UNSIGNED_BYTE, false }; }

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
class GL3D_API basic_object
{
public:
	using ptr = std::shared_ptr<basic_object>;
};

//---------------------------------------------------------------------------------------------------------------------
class GL3D_API gl_object : public basic_object
{
public:
	using ptr = std::shared_ptr<gl_object>;

	/// @brief OpenGL object ID
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
	dynamic_resizable,
	persistent,
	persistent_coherent
};

//---------------------------------------------------------------------------------------------------------------------
class GL3D_API buffer : public detail::gl_object
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

	void *map( unsigned accessFlags = +gl_enum::WRITE_ONLY ) const;

	void *map( size_t offset, size_t length, unsigned accessFlags = +gl_enum::MAP_WRITE_BIT | +gl_enum::MAP_INVALIDATE_RANGE_BIT ) const;

	void unmap() const;

	void resize( const void *data, size_t length );

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
class GL3D_API shader_code
{
public:
	using ptr = std::shared_ptr<shader_code>;

	template <typename... Args>
	static ptr create( Args &&... args ) { return std::make_shared<shader_code>( args... ); }

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
class GL3D_API shader : public detail::gl_object
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

class GL3D_API texture : public detail::gl_object
{
public:
	using ptr = std::shared_ptr<texture>;

	template <typename... Args>
	static ptr create( Args &&... args ) { return std::make_shared<texture>( args... ); }

	static ptr white_pixel();
	static ptr checkerboard();
	static ptr debug_grid();

	texture( gl_enum type, gl_internal_format format, const uvec3 &dimensions, bool hasMips = false );

	texture( gl_internal_format format, const uvec2 &dimensions, bool hasMips = false )
		: texture( gl_enum::TEXTURE_2D, format, { dimensions.x, dimensions.y, 1 }, hasMips )
	{

	}

	struct part
	{
		unsigned layer = 0;
		unsigned mip_level = 0;
		unsigned array_index = 0;
		const void *data = nullptr;
	};

	texture( gl_enum type, gl_internal_format format, const uvec3 &dimensions,
	         const detail::type_range<part> &parts,
	         bool buildMips = true, bool makeCopy = true );

	texture( gl_internal_format format, const uvec2 &dimensions,
	         const detail::type_range<part> &parts,
	         bool buildMips = true, bool makeCopy = true )
		: texture( gl_enum::TEXTURE_2D, format, { dimensions.x, dimensions.y, 1 }, parts, buildMips, makeCopy )
	{

	}

	texture( gl_internal_format format, const uvec2 &dimensions,
	         const void *data,
	         bool buildMips = true, bool makeCopy = true )
		: texture( format, dimensions, part{ 0, 0, 0, data }, buildMips, makeCopy )
	{

	}

	virtual ~texture();

	gl_enum type() const { return _type; }
	gl_internal_format format() const { return _format; }
	const uvec3 &dimensions() const { return _dimensions; }

	unsigned width( unsigned mipLevel = 0 ) const { return maximum( 1, _dimensions.x >> mipLevel ); }
	unsigned height( unsigned mipLevel = 0 ) const { return maximum( 1, _dimensions.y >> mipLevel ); }

	unsigned layers( unsigned mipLevel = 0 ) const
	{
		if ( _type == gl_enum::TEXTURE_2D_ARRAY )
			return _dimensions.z;
		else if ( _type == gl_enum::TEXTURE_CUBE_MAP || _type == gl_enum::TEXTURE_CUBE_MAP_ARRAY )
			return 6;

		return maximum( 1, _dimensions.z >> mipLevel );
	}

	unsigned array_size() const
	{
		if ( _type == gl_enum::TEXTURE_2D_ARRAY )
			return _dimensions.z;
		else if ( _type == gl_enum::TEXTURE_CUBE_MAP_ARRAY )
			return _dimensions.z / 6;

		return 1;
	}

	float aspect_ratio() const { return static_cast<float>( _dimensions.x ) / _dimensions.y; }

	void wrap( gl_enum u, gl_enum v, gl_enum w );
	void filter( gl_enum minFilter, gl_enum magFilter );

	void wrap_u( gl_enum value ) { wrap( value, _wrap[1], _wrap[2] ); }
	gl_enum wrap_u() const { return _wrap[0]; }

	void wrap_v( gl_enum value ) { wrap( _wrap[0], value, _wrap[2] ); }
	gl_enum wrap_v() const { return _wrap[1]; }

	void wrap_w( gl_enum value ) { wrap( _wrap[0], _wrap[1], value ); }
	gl_enum wrap_w() const { return _wrap[2]; }

	void filter_min( gl_enum value ) { filter( value, _filter[1] ); }
	gl_enum filter_min() const { return _filter[0]; }

	void filter_mag( gl_enum value ) { filter( _filter[0], value ); }
	gl_enum filter_mag() const { return _filter[1]; }

	uint64_t synchronize();

protected:
	void clear();

	gl_enum _type = gl_enum::NONE;
	gl_internal_format _format = gl_internal_format::NONE;
	uvec3 _dimensions;

	std::unique_ptr<part[]> _parts;
	unsigned _numParts = 0;
	bool _owner = false;
	bool _buildMips = false;

	gl_enum _wrap[3] = { gl_enum::REPEAT, gl_enum::REPEAT, gl_enum::REPEAT };
	gl_enum _filter[2] = { gl_enum::LINEAR_MIPMAP_LINEAR, gl_enum::LINEAR };
	bool _dirtySampler = true;

	uint64_t _bindlessHandle = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
struct GL3D_API blend_state
{
	uint8_t mask = 0xFFu;

	blend_state()
	{

	}
};

//---------------------------------------------------------------------------------------------------------------------
struct GL3D_API depth_stencil_state
{
	gl_enum depth_func = gl_enum::LESS;
	uint8_t stencil_read_mask = 0xFFu;
	uint8_t stencil_write_mask = 0xFFu;
	unsigned stencil_test : 1;
	unsigned depth_test : 1;
	unsigned depth_write : 1;

	depth_stencil_state()
		: stencil_test( 0 )
		, depth_test( 1 )
		, depth_write( 1 )
	{

	}
};

//---------------------------------------------------------------------------------------------------------------------
struct GL3D_API rasterizer_state
{
	gl_enum face_cull_mode = gl_enum::BACK;
	unsigned front_ccw : 1;
	unsigned wireframe : 1;
	unsigned depth_clamp : 1;
	unsigned scissor_test : 1;

	rasterizer_state()
		: front_ccw( 0 )
		, wireframe( 0 )
		, depth_clamp( 1 )
		, scissor_test( 0 )
	{

	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GL3D_API cmd_queue : public detail::basic_object
{
public:
	using ptr = std::shared_ptr<cmd_queue>;

	template <typename... Args>
	static ptr create() { return std::make_shared<cmd_queue>(); }

	cmd_queue(): cmd_queue( nullptr ) { }
	virtual ~cmd_queue();

	bool deferred() const { return _deferred; }

	void reset();

	void clear_color( const vec4 &color );
	void clear_depth( float depth );

	void update_texture( texture::ptr tex, const void *data, unsigned layer = 0, unsigned mipLevel = 0, size_t rowStride = 0 );
	void update_buffer( buffer::ptr buff, const void *data, size_t size, size_t offset = 0, bool preserveContent = false );
	void resize_buffer( buffer::ptr buff, const void *data, size_t size );
	void resize_buffer( buffer::ptr buff, size_t size ) { resize_buffer( buff, nullptr, size ); }

	void bind_shader( shader::ptr sh );
	void bind_vertex_buffer( buffer::ptr vertices, const detail::layout &layout, size_t offset = 0 );
	void bind_vertex_attribute( buffer::ptr attribs, unsigned slot, gl_enum glType, bool perInstance = false, size_t offset = 0, size_t stride = 0 );
	void bind_index_buffer( buffer::ptr indices, bool use16bits = false, size_t offset = 0 );

	void bind_texture( texture::ptr tex, unsigned slot );
	void bind_storage_buffer( buffer::ptr buff, unsigned slot, size_t offset = 0, size_t length = size_t( -1 ) );

	struct GL3D_API render_target
	{
		texture::ptr target;
		unsigned layer = 0;
		unsigned mip_level = 0;

		render_target() = default;

		render_target( std::nullptr_t ): target( nullptr ) { }

		render_target( texture::ptr tex, unsigned layerIndex = 0, unsigned mip = 0 )
			: target( tex )
			, layer( layerIndex )
			, mip_level( mip )
		{

		}

		bool operator==( const render_target &rhs ) const { return target == rhs.target && layer == rhs.layer && mip_level == rhs.mip_level; }
		bool operator!=( const render_target &rhs ) const { return !( ( *this ) == rhs ); }
	};

	void bind_render_targets( const render_target *colorTargets, size_t count, const render_target &depthStencilTarget, bool adjustViewport = true );

	void bind_render_targets( const std::initializer_list<render_target> &colorTargets, const render_target &depthStencilTarget, bool adjustViewport = true );

	void bind_render_targets( const render_target &colorTarget, const render_target &depthStencilTarget, bool adjustViewport = true )
	{
		bind_render_targets( &colorTarget, 1, depthStencilTarget, adjustViewport );
	}

	void unbind_render_targets( bool adjustViewport = true ) { bind_render_targets( {}, nullptr, adjustViewport ); }

	void set_uniform_block( const detail::location_variant &location, const void *data, size_t size );

	template <typename T>
	void set_uniform_block( const detail::location_variant &location, const T &block )
	{
		set_uniform_block( location, &block, sizeof( T ) );
	}

	void set_uniform( const detail::location_variant &location, bool value );
	void set_uniform( const detail::location_variant &location, int value );
	void set_uniform( const detail::location_variant &location, float value );
	void set_uniform( const detail::location_variant &location, const vec2 &value );
	void set_uniform( const detail::location_variant &location, const vec3 &value );
	void set_uniform( const detail::location_variant &location, const vec4 &value );
	void set_uniform( const detail::location_variant &location, const mat3 &value, bool transpose = false );
	void set_uniform( const detail::location_variant &location, const mat4 &value, bool transpose = false );
	void set_uniform( const detail::location_variant &location, texture::ptr tex );

	void set_uniform( const detail::location_variant &location, const uint64_t *values, size_t count );
	void set_uniform( const detail::location_variant &location, const uvec3 *values, size_t count );
	void set_uniform( const detail::location_variant &location, const ivec4 *values, size_t count );
	void set_uniform( const detail::location_variant &location, const mat4 *values, size_t count, bool transpose = false );

	void set_uniform( const detail::location_variant &location, const detail::type_range<uint64_t> &values )
	{
		set_uniform( location, values.data, values.size );
	}

	void set_uniform( const detail::location_variant &location, const detail::type_range<uvec3> &values )
	{
		set_uniform( location, values.data, values.size );
	}

	void set_uniform( const detail::location_variant &location, const detail::type_range<ivec4> &values )
	{
		set_uniform( location, values.data, values.size );
	}

	void set_uniform( const detail::location_variant &location, const detail::type_range<mat4> &values, bool transpose = false )
	{
		set_uniform( location, values.data, values.size, transpose );
	}

	void set_state( const blend_state &bs );
	void set_state( const depth_stencil_state &ds );
	void set_state( const rasterizer_state &rs );

	void draw( gl_enum primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );
	void draw_indexed( gl_enum primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );

	void execute( ptr cmdQueue );

protected:
	struct gl_state
	{
		buffer::ptr temp_buffer;
		uint8_t *mapped_temp_buffer = nullptr;
		size_t temp_buffer_cursor = 0;
		int uniform_block_alignment = 0;

		buffer::ptr current_vb;
		const detail::layout *current_vb_layout = nullptr;
		size_t current_vb_offset = 0;

		buffer::ptr current_ib;
		bool current_ib_16bits = false;
		size_t current_ib_offset = 0;

		bool dirty_input_assembly = true;

		void reset();
		size_t write_temp_data( const void *data, size_t size );
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

	bool synchronize_input_assembly();
	void execute( gl_state *state );

	bool _deferred = true;
	std::vector<uint8_t> _recordedData;
	std::vector<detail::basic_object::ptr> _resources;
	size_t _position = 0;
	gl_state *_state = nullptr;

	enum class cmd_type
	{
		clear_color, clear_depth,
		update_texture, update_buffer, resize_buffer,
		bind_blend_state, bind_depth_stencil_state, bind_rasterizer_state,
		bind_shader, bind_vertex_buffer, bind_vertex_attribute, bind_index_buffer,
		bind_texture, bind_storage_buffer, bind_render_targets,
		set_uniform_block, set_uniform, set_uniform_array,
		draw, draw_indexed,
		execute,
	};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
class GL3D_API context : public cmd_queue
{
public:
	using ptr = std::shared_ptr<context>;

	context( void *windowNativeHandle, ptr sharedContext = nullptr );
	context( ptr sharedContext );
	virtual ~context();

	void *window_native_handle() const { return _window_native_handle; }
	void *native_handle() const { return _native_handle; }

	void make_current( const uvec2 &defaultFBSize = { 0, 0 } );

	unsigned get_or_create_layout_vao( const detail::layout *layout );
	unsigned get_or_create_fbo( const render_target *colorTargets, size_t count, const render_target &depthStencilTarget );

	uvec2 default_framebuffer_size() const { return _defaultFramebufferSize; }

protected:
	void *_window_native_handle = nullptr;
	void *_native_handle = nullptr;

	struct vao_desc
	{
		const detail::layout *layout = nullptr;
		unsigned vao_id = 0;
		unsigned unused_frames = 0;
	};

	std::unordered_map<std::uintptr_t, vao_desc> _layoutVAOs;

	struct fbo_desc
	{
		render_target color_targets[max_render_targets];
		render_target depth_stencil_target;
		unsigned num_color_targets = 0;
		unsigned fbo_id = 0;
		unsigned unused_frames = 0;
	};

	std::vector<fbo_desc> _fboDescs;

	gl_state _glState;
	uvec2 _defaultFramebufferSize;
};

//---------------------------------------------------------------------------------------------------------------------
class GL3D_API async_upload_context
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
