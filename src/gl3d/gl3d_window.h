#ifndef __GL3D_WIN32_H__
#define __GL3D_WIN32_H__

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl3d.h"

namespace gl3d::detail { struct window_impl; }

namespace gl3d {

extern const unsigned &frame_id;
extern const float &time;
extern const float &delta;

void run();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
enum class window_flag
{
	none = 0,
	resizable = 0b00000001,
	fullscreen = 0b00000010,
	title = 0b00000100,
};

GL3D_ENUM_PLUS( window_flag )

static const unsigned default_window_flags = ( +window_flag::resizable ) | ( +window_flag::title );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class window
{
public:
	using ptr = std::shared_ptr<window>;

	static ptr open( std::string_view title, uvec2 size, ivec2 pos = { INT_MAX, INT_MAX }, unsigned flags = default_window_flags );
	static ptr from_id( unsigned id );

	virtual ~window();

	unsigned flags() const { return _flags; }
	unsigned id() const { return _id; }
	void *native_handle() const { return _native_handle; }
	auto context() const { return _context; }

	bool closed() const { return _native_handle == nullptr; }

	void title( std::string_view text );
	const std::string &title() const { return _title; }

	void adjust( uvec2 size, ivec2 pos );

	void size( uvec2 size ) { adjust( _pos, size ); }
	uvec2 size() const { return _size; }

	void position( ivec2 pos ) { adjust( pos, _size ); }
	ivec2 position() const { return _pos; }

	void close();
	void present();

protected:
	friend struct detail::window_impl;

	unsigned _flags = 0;
	unsigned _id = UINT_MAX;
	void *_native_handle = nullptr;
	detail::context::ptr _context;
	std::string _title;
	ivec2 _pos;
	uvec2 _size;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct window_event
{
	enum class type { open, close, resize, move, paint, drop_files };

	type event_type;
	unsigned window_id;

	union
	{
		uvec2 resize;
		ivec2 move;
		const detail::files_t *files;
	};

	window_event( type t, unsigned id ): event_type( t ), window_id( id ) { }
};

extern detail::callback_chain<bool( window_event & )> on_window_event;

} // namespace gl3d

#endif // __GL3D_WIN32_H__

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_WIN32_H_IMPL__
		#define __GL3D_WIN32_H_IMPL__
		#if defined(WIN32)
			#include "gl3d_window_win32.inl"
		#endif
	#endif // __GL3D_WIN32_H_IMPL__
#endif // GL2D_IMPLEMENTATION
