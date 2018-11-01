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

static constexpr char *s_lineSeparator = "\n";

//---------------------------------------------------------------------------------------------------------------------
std::string_view trim( std::string_view text )
{
	if ( text.empty() ) return text;

	size_t start = 0, end = text.length() - 1;
	while ( start < text.length() && isspace( text[start] ) ) ++start;
	while ( end > start && isspace( text[end] ) ) --end;

	return text.substr( start, end - start + 1 );
}

//---------------------------------------------------------------------------------------------------------------------
bool starts_with_nocase( std::string_view text, std::string_view start )
{
	if ( start.empty() ) return true;
	if ( start.length() > text.length() ) return false;

	for ( size_t i = 0, S = start.length(); i < S; ++i )
		if ( tolower( start[i] ) != tolower( text[i] ) )
			return false;

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void for_each_line( std::string_view text, std::function<void( std::string_view, unsigned )> callback )
{
	size_t cursor = 0;
	unsigned lineNum = 0;
	while ( cursor <= text.length() )
	{
		auto sepPos = text.find( s_lineSeparator, cursor );
		if ( sepPos == std::string::npos )
			sepPos = text.length();

		callback( text.substr( cursor, sepPos - cursor ), ++lineNum );
		cursor = sepPos + strlen( s_lineSeparator );
	}
}

//---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<char[]> load_all_char_string( std::istream &is, size_t size = -1 )
{
	if ( true )
	{
		is.seekg( 0, std::ios_base::end );
		size = is.tellg();
		is.seekg( 0, std::ios_base::beg );
	}

	std::unique_ptr<char[]> result( new char[size + 1] );
	is.read( result.get(), size );
	result[size] = 0;

	return std::move( result );
}

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
