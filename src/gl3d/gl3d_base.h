#ifndef __GL3D_BASE_H__
#define __GL3D_BASE_H__

#include <limits.h>

#include <functional>
#include <set>
#include <type_traits>

#define GL3D_ENUM_PLUS(_Type) \
	constexpr auto operator+( _Type t ) { return static_cast<std::underlying_type_t<_Type>>( t ); }

namespace gl3d {

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
template <typename F> struct callback_list
{
	using function_t = std::function<F>;

	struct callback_info
	{
		int priority = 0;
		function_t callback;
		bool operator<( const callback_info &rhs ) const { return priority < rhs.priority; }
		bool operator==( const function_t &f ) const { return callback == f; }
	};

	std::set<callback_info> callbacks;

	callback_list &operator()( function_t &&f, int priority = 0 )
	{
		callbacks.insert( callbacks.end(), { priority, f } );
		return *this;
	}

	template <typename... Args> void call( Args &&... args ) const
	{
		for ( auto && ci : callbacks )
			ci.callback( args... );
	}
};

} // namespace gl3d::detail

} // namespace gl3d

#endif // __GL3D_BASE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_BASE_H_IMPL__
		#define __GL3D_BASE_H_IMPL__
		#include "gl3d_base.inl"
	#endif // __GL3D_BASE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
