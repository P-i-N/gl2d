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

class buffer
{
public:

};

} // namespace gl3d

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
	#ifndef __GL3D_H_IMPL__
		#define __GL3D_H_IMPL__
		#include "gl3d.inl"
	#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
