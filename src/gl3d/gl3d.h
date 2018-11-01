#ifndef __GL3D_H__
#define __GL3D_H__

#include <vector>
#include <memory>
#include <variant>

#include <filesystem>

#include "gl3d_math.h"

#define GL3D_LAYOUT(...) \
	static const gl3d::detail::layout &get_layout() { \
		static gl3d::detail::layout layout { __VA_ARGS__ }; \
		return layout; }

namespace gl3d {

/* Forward declarations */
class buffer;
class cmd_queue;
class compiled_shader;
class context;
class shader;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct gl
{
	enum enum_t : unsigned
	{
		NONE = 0, ZERO = 0,
		ONE = 1,

		BYTE = 0x1400, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT, FLOAT,
		DOUBLE = 0x140A,

		NEVER = 0x0200, LESS, EQUAL, LEQUAL, GREATER, NOTEQUAL, GEQUAL, ALWAYS,

		SRC_COLOR = 0x0300, ONE_MINUS_SRC_COLOR, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, DST_ALPHA, ONE_MINUS_DST_ALPHA,
		DST_COLOR = 0x0306, ONE_MINUS_DST_COLOR, SRC_ALPHA_SATURATE,

		CW = 0x0900, CCW,

#if defined(WIN32)
		CONTEXT_MAJOR_VERSION_ARB = 0x2091, CONTEXT_MINOR_VERSION_ARB,

		CONTEXT_PROFILE_MASK_ARB = 0x9126,
		CONTEXT_CORE_PROFILE_BIT_ARB = 0x0001,
#endif
	};

#if defined(WIN32)
	static void *( __stdcall *CreateContextAttribsARB )( void *, void *, const int * );
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

struct layout
{
	struct attr { unsigned location, offset, element_count, element_type; };

	std::vector<attr> attribs;
	unsigned mask = 0;

	template <typename... Args>
	layout( Args &&... args ) : attribs( sizeof...( Args ) / 2 ) { init( 0, args... ); }

private:
	template <typename T> struct type { };

	void fill( attr &a, unsigned loc, unsigned off, type<int> ) { a = { loc, off, 1, gl::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<float> ) { a = { loc, off, 1, gl::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec2> ) { a = { loc, off, 2, gl::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec2> ) { a = { loc, off, 2, gl::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec3> ) { a = { loc, off, 3, gl::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec3> ) { a = { loc, off, 3, gl::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec4> ) { a = { loc, off, 4, gl::FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec4> ) { a = { loc, off, 4, gl::INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<byte_vec4> ) { a = { loc, off, 4, gl::UNSIGNED_BYTE }; }

	template <typename T1, typename T2, typename... Args>
	void init( unsigned index, unsigned location, T1 T2::*member, Args &&... args )
	{
		fill( attribs[index], location, unsigned( size_t( &( ( ( T2 * )0 )->*member ) ) ), type<T1>() );
		mask |= ( 1u << location );

		if constexpr ( sizeof...( Args ) >= 2 )
			init( index + 1, args... );
	}
};

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

class resource
{
public:
	using ptr = std::shared_ptr<resource>;
};

class render_object : public resource
{
public:
	using ptr = std::shared_ptr<render_object>;

	unsigned id() const { return _id; }

protected:
	render_object() = default;

	unsigned _id = 0;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class buffer : public detail::render_object
{
public:
	using ptr = std::shared_ptr<buffer>;

	buffer( gl::enum_t type );
	virtual ~buffer();

	gl::enum_t type() const { return _type; }

	void allocate_data( const void *data, size_t size, bool releaseAfterUpload = true );
	void set_data( const void *data, size_t size );

protected:
	gl::enum_t _type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class compiled_shader : public detail::render_object
{
public:
	using ptr = std::shared_ptr<compiled_shader>;

	virtual ~compiled_shader();

protected:
	compiled_shader();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class shader : public detail::resource
{
public:
	using ptr = std::shared_ptr<shader>;

	const std::filesystem::path &path() const { return _path; }
	const std::string &source() const { return _source; }
	const std::string &unrolled_source() const { return _unrolledSource; }

	bool source( std::string_view sourceCode, const std::filesystem::path &cwd = std::filesystem::path() );
	bool load( std::istream &is, const std::filesystem::path &cwd = std::filesystem::path() );
	bool load( const std::filesystem::path &path );

	compiled_shader::ptr compile();

protected:
	std::filesystem::path _path;
	std::string _source;
	std::string _unrolledSource;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct blend_state
{
	uint8_t mask = 0xFFu;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct depth_stencil_state
{
	gl::enum_t depth_func = gl::LESS;
	uint8_t stencil_read_mask = 0;
	uint8_t stencil_write_mask = 0;
	unsigned stencil_test : 1;
	unsigned depth_test : 1;
	unsigned depth_write : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct rasterizer_state
{
	gl::enum_t face_cull_mode = gl::NONE;
	unsigned front_ccw : 1;
	unsigned wireframe : 1;
	unsigned depth_camp : 1;
	unsigned scissor_test : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class cmd_queue : public detail::resource
{
public:
	using ptr = std::shared_ptr<cmd_queue>;
	using location_variant_t = std::variant<unsigned, size_t, std::string_view>;

	cmd_queue(): cmd_queue( true ) { }
	virtual ~cmd_queue();

	bool recording() const { return _recording; }

	void reset();

	void clear_color( const vec4 &color );
	void clear_depth( float depth );

	void bind_state( const blend_state &bs );
	void bind_state( const depth_stencil_state &ds );
	void bind_state( const rasterizer_state &rs );

	void bind_shader( compiled_shader::ptr sh );
	void bind_vertex_buffer( buffer::ptr vertices, const detail::layout &layout, size_t offset = 0 );
	void bind_index_buffer( buffer::ptr indices, bool use16bits, size_t offset = 0 );

	void uniform_block( location_variant_t location, const void *data, size_t size );

	void draw( gl::enum_t primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );
	void draw_indexed( gl::enum_t primitive, size_t first, size_t count, size_t instanceCount = 1, size_t instanceBase = 0 );

	void execute( ptr cmdQueue );

protected:
	cmd_queue( bool record );

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

	template <typename H, typename... T>
	void write_value( uint8_t *cursor, H &&head, T &&... tail )
	{
		memcpy( cursor, &head, sizeof( H ) );
		if constexpr ( sizeof...( T ) > 0 )
			write_value( cursor + sizeof( H ), tail... );
	}

	template <typename T>
	const T &read()
	{
		_position += sizeof( T );
		return *reinterpret_cast<const T *>( _recordedData.data() + _position - sizeof( T ) );
	}

	template <typename T> void read( T &value ) { value = read<T>(); }

	void execute();

	bool _recording = true;
	std::vector<uint8_t> _recordedData;
	std::vector<detail::resource::ptr> _resources;
	size_t _position = 0;

	enum class cmd_type
	{
		clear_color,
		clear_depth,
		bind_blend_state,
		bind_depth_stencil_state,
		bind_rasterizer_state,
		bind_shader,
		bind_vertex_buffer,
		bind_index_buffer,
		draw,
		draw_indexed,
		execute,
	};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

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

protected:
	void *_window_native_handle = nullptr;
	void *_native_handle = nullptr;
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
