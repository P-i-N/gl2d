#ifndef __GL3D_H__
#define __GL3D_H__

#include <functional>
#include <vector>
#include <memory>

// Include base 3D math library
#include "gl3d_math.h"

#define GL3D_LAYOUT(...) \
	static const gl3d::detail::layout &get_layout() { \
		static gl3d::detail::layout layout { __VA_ARGS__ }; \
		return layout; }

namespace gl3d {

struct gl
{
	enum enum_t : unsigned
	{
		NONE = 0,
		BYTE = 0x1400,
		UNSIGNED_BYTE = 0x1401,
		SHORT = 0x1402,
		UNSIGNED_SHORT = 0x1403,
		INT = 0x1404,
		UNSIGNED_INT = 0x1405,
		FLOAT = 0x1406,
		DOUBLE = 0x140A,

#if defined(WIN32)
		CONTEXT_MAJOR_VERSION_ARB = 0x2091,
		CONTEXT_MINOR_VERSION_ARB = 0x2092,
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

class context
{
public:
	using ptr = std::shared_ptr<context>;

	static ptr create( void *windowNativeHandle );

	virtual ~context();

	void *window_native_handle() const { return _window_native_handle; }
	void *native_handle() const { return _native_handle; }

	void make_current();

protected:
	void *_window_native_handle = nullptr;
	void *_native_handle = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

class render_object
{
public:
	using ptr = std::shared_ptr<render_object>;

protected:
	render_object() = default;
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

class shader : public detail::render_object
{
public:
	using ptr = std::shared_ptr<shader>;


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class cmd_list
{
public:
	using ptr = std::shared_ptr<cmd_list>;

	cmd_list();
	virtual ~cmd_list();

	bool recording() const { return _recording; }

	void begin();
	void end();

	void bind_shader( shader::ptr sh );
	void bind_vertex_buffer( buffer::ptr vb );
	void bind_index_buffer( buffer::ptr ib );

	void draw( gl::enum_t primitive, unsigned first, unsigned count, unsigned instanceCount = 1, unsigned instanceBase = 0 );
	void draw_indexed( gl::enum_t primitive, unsigned first, unsigned count, unsigned instanceCount = 1, unsigned instanceBase = 0 );

protected:
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
	std::vector<detail::render_object::ptr> _renderObjects;
	size_t _position = 0;

	enum class cmd_type
	{
		end,
		bind_shader,
		bind_vertex_buffer,
		bind_index_buffer,
		draw,
		draw_indexed,
	};
};

} // namespace gl3d

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
	#ifndef __GL3D_H_IMPL__
		#define __GL3D_H_IMPL__
		#include "gl3d.inl"
	#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
