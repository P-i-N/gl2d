#ifndef __GL3D_BASE_H__
#define __GL3D_BASE_H__

#include <limits.h>

#include <functional>
#include <type_traits>

#define GL3D_ENUM_PLUS(_Type) \
	constexpr auto operator+( _Type t ) { return static_cast<std::underlying_type_t<_Type>>( t ); }

namespace gl3d {

namespace detail {

std::string_view trim( std::string_view text );
bool starts_with_nocase( std::string_view text, std::string_view start );
void for_each_line( std::string_view text, std::function<void( std::string_view, unsigned )> callback );
std::vector<char> load_all_chars( std::istream &is, bool addNullTerm = true, size_t size = 0 );

//---------------------------------------------------------------------------------------------------------------------
template <typename F> struct callback_chain
{
	using function_t = std::function<F>;

	struct callback_info
	{
		int priority = 0;
		function_t callback;
		bool operator<( const callback_info &rhs ) const { return priority < rhs.priority; }
		bool operator==( const function_t &f ) const { return callback == f; }
	};

	std::vector<callback_info> callbacks;

	callback_chain &operator()( function_t &&f, int priority = 0 )
	{
		callbacks.insert( callbacks.end(), { priority, f } );
		std::sort( callbacks.begin(), callbacks.end() );
		return *this;
	}

	template <typename... Args> bool call( Args &&... args ) const
	{
		if constexpr ( std::is_void_v<std::result_of_t<function_t( Args... )>> )
		{
			for ( auto && ci : callbacks )
				ci.callback( args... );
		}
		else
		{
			for ( auto && ci : callbacks )
				if ( !ci.callback( args... ) )
					return false;
		}

		return true;
	}
};

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct log
{
	enum class message_type { info, success, warning, error, fatal };

	static void info( const char *fmt, ... );
	static void success( const char *fmt, ... );
	static void warning( const char *fmt, ... );
	static void error( const char *fmt, ... );
	static void fatal( const char *fmt, ... );
};

extern detail::callback_chain<void( log::message_type, const char * )> on_log_message;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct filesystem
{

};

} // namespace gl3d

#endif // __GL3D_BASE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_BASE_H_IMPL__
		#define __GL3D_BASE_H_IMPL__
		#include "gl3d_base.inl"
	#endif // __GL3D_BASE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
