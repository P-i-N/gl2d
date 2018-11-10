#ifndef __GL3D_BASE_H__
#define __GL3D_BASE_H__

#include <limits.h>

#include <functional>
#include <mutex>
#include <type_traits>

#define GL3D_ENUM_PLUS(_Type) \
	constexpr auto operator+( _Type t ) { return static_cast<std::underlying_type_t<_Type>>( t ); }

namespace gl3d {

namespace detail {

using bytes_t = std::vector<uint8_t>;

//---------------------------------------------------------------------------------------------------------------------
std::string_view trim( std::string_view text );
std::string_view to_string_view( bytes_t &bytes );
void for_each_line( std::string_view text, std::function<void( std::string_view, unsigned )> callback );
void read_all_bytes( std::istream &is, bytes_t &bytes, bool addNullTerm = false, size_t size = size_t( -1 ) );
bool unroll_includes( std::stringstream &ss, std::string_view sourceCode, const std::filesystem::path &cwd );

//---------------------------------------------------------------------------------------------------------------------
template <typename... Tail>
bool starts_with_nocase( std::string_view text, std::string_view head, Tail &&... tail )
{
	if ( head.empty() )
		return true;

	bool found = ( head.length() <= text.length() );
	if ( found )
	{
		for ( size_t i = 0, S = head.length(); i < S; ++i )
		{
			if ( tolower( head[i] ) != tolower( text[i] ) )
			{
				found = false;
				break;
			}
		}
	}

	if ( !found )
	{
		if constexpr ( sizeof...( Tail ) > 0 )
			return starts_with_nocase( text, tail... );
	}

	return found;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename F> class callback_chain final
{
public:
	using function_t = std::function<F>;

	callback_chain &operator()( function_t f, int priority = 0 )
	{
		std::scoped_lock lock( _mutex );
		_callbacks.insert( _callbacks.end(), { priority, f } );
		std::sort( _callbacks.begin(), _callbacks.end() );
		return *this;
	}

	void operator+=( function_t f )
	{
		( *this )( f, 0 );
	}

	bool operator-=( function_t f )
	{
		std::scoped_lock lock( _mutex );
		if ( auto iter = std::find( _callbacks.begin(), _callbacks.end(), f ); iter != _callbacks.end() )
		{
			_callbacks.erase( iter );
			std::sort( _callbacks.begin(), _callbacks.end() );
			return true;
		}

		return false;
	}

	template <typename... Args> std::result_of_t<function_t( Args... )> call( Args &&... args ) const
	{
		thread_local decltype( _callbacks ) callbacksCopy;

		size_t firstIndex = callbacksCopy.size();
		{
			std::scoped_lock lock( _mutex );
			std::copy( _callbacks.begin(), _callbacks.end(), std::back_inserter( callbacksCopy ) );
		}
		size_t lastIndex = callbacksCopy.size();

		if constexpr ( std::is_void_v<std::result_of_t<function_t( Args... )>> )
		{
			for ( size_t i = firstIndex; i < lastIndex; ++i )
				callbacksCopy[i].callback( args... );

			callbacksCopy.resize( firstIndex );
		}
		else
		{
			bool result = false;

			for ( size_t i = firstIndex; i < lastIndex; ++i )
				if ( result |= callbacksCopy[i].callback( args... ) )
					break;

			callbacksCopy.resize( firstIndex );
			return result;
		}
	}

private:
	struct callback_info
	{
		int priority = 0;
		function_t callback;
		bool operator<( const callback_info &rhs ) const { return priority < rhs.priority; }
		bool operator==( const function_t &f ) const { return callback == f; }
	};

	mutable std::mutex _mutex;
	std::vector<callback_info> _callbacks;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct location_variant
{
	const char *data;
	unsigned size_or_id;

	location_variant( int id ): size_or_id( static_cast<unsigned>( id + 1 ) ) { }
	location_variant( const char *text, unsigned size ): data( text ), size_or_id( 0x80000000u | size ) { }
	location_variant( const char *text ): location_variant( text, static_cast<unsigned>( strlen( text ) ) ) { }

	bool holds_name() const { return ( size_or_id & 0x80000000u ) != 0; }
	unsigned size() const { return size_or_id & 0x7FFFFFFFu; }
	int id() const { return static_cast<int>( size_or_id ) - 1; }
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

struct vfs
{
	static void mount( const std::filesystem::path &path );
	static bool unmount( const std::filesystem::path &path );
	static bool load( const std::filesystem::path &path, detail::bytes_t &bytes );
};

extern detail::callback_chain<bool( const std::filesystem::path &, detail::bytes_t & )> on_vfs_load;

} // namespace gl3d

#endif // __GL3D_BASE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_BASE_H_IMPL__
		#define __GL3D_BASE_H_IMPL__
		#include "gl3d_base.inl"
	#endif // __GL3D_BASE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
