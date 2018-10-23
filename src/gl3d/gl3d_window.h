#ifndef __GL3D_WIN32_H__
#define __GL3D_WIN32_H__

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "gl3d.h"
#include "gl3d_input.h"

namespace gl3d {

void run();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail { struct window_impl; }

class window
{
public:
	using ptr = std::shared_ptr<window>;

	static ptr open( const std::string &title, ivec2 pos, ivec2 size, unsigned flags );
	static ptr open( const std::string &title, ivec2 size )
	{
		return open( title, { INT_MAX, INT_MAX }, size, default_window_flags );
	}

	virtual ~window();

	unsigned id() const { return _id; }
	void *native_handle() const { return _native_handle; }
	auto context() const { return _context; }

	bool closed() const { return _native_handle == nullptr; }

	void title( const std::string &text );
	const std::string &title() const { return _title; }

	void adjust( ivec2 pos, ivec2 size );

	void size( ivec2 size ) { adjust( _pos, size ); }
	ivec2 size() const { return _size; }

	void position( ivec2 pos ) { adjust( pos, _size ); }
	ivec2 position() const { return _pos; }

	void close();
	void present();

protected:
	friend struct detail::window_impl;

	unsigned _id = UINT_MAX;
	void *_native_handle = nullptr;
	context::ptr _context;
	std::string _title;
	ivec2 _pos;
	ivec2 _size;
};

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
