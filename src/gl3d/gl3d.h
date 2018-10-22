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
	static constexpr unsigned NONE = 0;
	static constexpr unsigned BYTE = 0x1400;
	static constexpr unsigned UNSIGNED_BYTE = 0x1401;
	static constexpr unsigned SHORT = 0x1402;
	static constexpr unsigned UNSIGNED_SHORT = 0x1403;
	static constexpr unsigned INT = 0x1404;
	static constexpr unsigned UNSIGNED_INT = 0x1405;
	static constexpr unsigned FLOAT = 0x1406;
	static constexpr unsigned DOUBLE = 0x140A;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class buffer
{
public:

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context
{
public:
	using ptr = std::shared_ptr<context>;

	static ptr from_native_handle( void *nativeHandle );
	static ptr from_shared_context( const context &parent );

	void *native_handle() const { return _native_handle; }

	void make_current();

protected:
	void *_native_handle = nullptr;
};

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------


} // namespace gl3d

#endif // __GL3D_H__

#if defined(GL3D_IMPLEMENTATION)
	#ifndef __GL3D_H_IMPL__
		#define __GL3D_H_IMPL__

		#include "gl3d.inl"

	#endif // __GL3D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
