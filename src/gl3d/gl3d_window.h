#ifndef __GL3D_WIN32_H__
#define __GL3D_WIN32_H__

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl3d.h"
#include "gl3d_quick_draw.h"

namespace gl3d {

GL3D_API extern const unsigned &frame_id;
GL3D_API extern const float &time;
GL3D_API extern const float &delta;
GL3D_API extern unsigned fps_limit;
GL3D_API void run();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
enum class window_flag
{
	none       = 0,
	resizable  = 0b00000001,
	fullscreen = 0b00000010,
	borderless = 0b00000100,
};

GL3D_ENUM_PLUS( window_flag )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GL3D_API window
{
public:
	using ptr = std::shared_ptr<window>;

	static const unsigned default_flags = +window_flag::resizable;

	static ptr create( std::string_view title, uvec2 size, ivec2 pos = { INT_MAX, INT_MAX }, unsigned flags = default_flags );
	static ptr from_id( unsigned id );

	virtual ~window();

	unsigned flags() const { return _flags; }
	unsigned id() const { return _id; }
	void *native_handle() const { return _native_handle; }
	auto context() const { return _context; }
	auto quick_draw() const { return _qd; }

	bool closed() const { return _native_handle == nullptr; }

	void title( std::string_view text );
	const std::string &title() const { return _title; }

	void adjust( uvec2 size, ivec2 pos );

	void size( uvec2 size ) { adjust( _pos, size ); }
	uvec2 size() const { return _size; }
	float aspect_ratio() const { return static_cast<float>( _size.x ) / _size.y; }

	void position( ivec2 pos ) { adjust( pos, _size ); }
	ivec2 position() const { return _pos; }

	void close();
	void present();

	void fullscreen( bool set );
	bool fullscreen() const { return _fullscreen; }

	void toggle_fullscreen() { fullscreen( !_fullscreen ); }

protected:
	friend struct detail::window_impl;

	unsigned _flags = 0;
	unsigned _id = UINT_MAX;
	void *_native_handle = nullptr;
	detail::context::ptr _context;
	quick_draw::ptr _qd;
	std::string _title;
	ivec2 _pos;
	uvec2 _size;

	bool _fullscreen = false;
	ivec2 _prevPos;
	uvec2 _prevSize;
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

GL3D_API extern detail::callback_chain<bool( window_event & )> on_window_event;

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
