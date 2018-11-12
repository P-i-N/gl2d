#ifndef __GL3D_BASE_H__
#define __GL3D_BASE_H__

#include <limits.h>

#include <array>
#include <functional>
#include <mutex>
#include <type_traits>
#include <vector>

#include "gl3d_math.h"

#define GL3D_ENUM_PLUS(_Type) \
	constexpr auto operator+( _Type t ) { return static_cast<std::underlying_type_t<_Type>>( t ); }

namespace gl3d {

namespace detail {

using bytes_t = std::vector<uint8_t>;
using files_t = std::vector<std::filesystem::path>;

//---------------------------------------------------------------------------------------------------------------------
std::string_view trim( std::string_view text );
std::string_view to_string_view( bytes_t &bytes );
void for_each_line( std::string_view text, std::function<void( std::string_view, unsigned )> callback );
void read_all_bytes( std::istream &is, bytes_t &bytes, bool addNullTerm = false, size_t size = size_t( -1 ) );
bool unroll_includes( std::stringstream &ss, std::string_view sourceCode, const std::filesystem::path &cwd );
void *get_proc_address( const char *name );

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

//---------------------------------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------------------------
template <typename F> struct proc_wrapper
{
	void *ptr = nullptr;
	proc_wrapper( const char *name ) : ptr( get_proc_address( name ) ) { }

	template <typename... Args>
	std::result_of_t<std::function<F>( Args... )> operator()( Args... args ) const
	{
		auto f = static_cast<F *>( ptr );
		if constexpr ( std::is_void_v<std::result_of_t<std::function<F>( Args... )>> )
			f( args... );
		else
			return f( args... );
	}
};

//---------------------------------------------------------------------------------------------------------------------
struct raw_data_range
{
	const void *data;
	size_t size;

	raw_data_range( const void *ptr, size_t sizeInBytes ): data( ptr ), size( sizeInBytes ) { }

	template <typename T, typename A>
	raw_data_range( const std::vector<T, A> &v ): data( v.data() ), size( v.size() * sizeof( T ) ) { }

	template <typename T, size_t N>
	raw_data_range( const std::array<T, N> &arr ): data( arr.data() ), size( arr.size() * sizeof( T ) ) { }

	template <typename T, size_t N>
	raw_data_range( const T ( &arr )[N] ): data( &arr[0] ), size( N * sizeof( T ) ) { }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct type_range
{
	const T *data;
	size_t size;

	type_range( const T *ptr, size_t numElements ) : data( ptr ), size( numElements ) { }

	type_range( const T &item ) : data( &item ), size( 1 ) { }

	template <typename U, typename A>
	type_range( const std::vector<U, A> &v ) : data( v.data() ), size( v.size() ) { }

	template <typename U, size_t N>
	type_range( const std::array<U, N> &arr ) : data( arr.data() ), size( arr.size() ) { }

	template <typename U, size_t N>
	type_range( const U( &arr )[N] ) : data( &arr[0] ), size( N ) { }
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
enum class key
{
	unknown = 0,
	enter = 13,
	tab = 8, backspace = 9,
	escape = 27,
	space = ' ',
	num_0 = '0', num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,
	a = 'A', b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
	numpad_0, numpad_1, numpad_2, numpad_3, numpad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
	up, right, down, left,
	insert, del,
	home, end,
	page_up, page_down,
	f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
	ctrl, alt, shift,

	__count
};

GL3D_ENUM_PLUS( key )

//---------------------------------------------------------------------------------------------------------------------
enum class mouse_button
{
	unknown = 0,
	left, right, middle,
	special_0, special_1,
	back, forward,

	__count
};

GL3D_ENUM_PLUS( mouse_button )

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_button
{
	unknown = 0,
	a, b, x, y,
	up, right, down, left,
	shoulder_left, shoulder_right,
	thumb_left, thumb_right,

	__count
};

GL3D_ENUM_PLUS( gamepad_button )

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_axis
{
	unknown = 0,
	thumb_left, thumb_right,
	trigger_left, trigger_right,

	__count
};

GL3D_ENUM_PLUS( gamepad_axis )

//---------------------------------------------------------------------------------------------------------------------
enum class space_navigator_button
{
	unknown = 0,
	left, right,

	__count
};

GL3D_ENUM_PLUS( space_navigator_button )

//---------------------------------------------------------------------------------------------------------------------
enum class event_type
{
	unknown = 0,
	run, quit,
	paint,
	open, close, resize, move,
	key_down, key_up, key_press,
	mouse_down, mouse_up, mouse_move, mouse_wheel,
	gamepad_down, gamepad_up, gamepad_move, gamepad_connect, gamepad_disconnect
};

//---------------------------------------------------------------------------------------------------------------------
struct event
{
	event_type type;
	unsigned window_id;

	union
	{
		ivec2 resize, move, wheel;
		struct { key k; int ch; } keyboard;
		struct { ivec2 pos, delta; mouse_button b; } mouse;
		struct { unsigned port; vec2 pos, delta; gamepad_button b; gamepad_axis axis; } gamepad;
	};

	event( event_type et, unsigned id )
		: type( et )
		, window_id( id )
	{

	}
};

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
struct keyboard_state
{
	bool key_down[+key::__count];
	bool operator[]( key k ) const { return key_down[+k]; }

	void change_key_state( key k, bool down, unsigned id = UINT_MAX );
};
}

extern detail::keyboard_state keyboard;

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
struct mouse_state
{
	bool button_down[+mouse_button::__count];
	bool operator[]( mouse_button b ) const { return button_down[+b]; }
	ivec2 pos;

	void change_button_state( mouse_button b, bool down, unsigned id = UINT_MAX );
	void change_position( ivec2 pos, unsigned id = UINT_MAX );
};
}

extern detail::mouse_state mouse;

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
static constexpr unsigned max_gamepads = 8;

struct gamepad_state
{
	unsigned port = UINT_MAX;
	bool button_down[+gamepad_button::__count];
	vec2 pos[+gamepad_axis::__count];
	bool operator[]( gamepad_button b ) const { return button_down[+b]; }
	bool connected() const { return port < max_gamepads; }

	void change_button_state( gamepad_button b, bool down );
	void change_axis_state( gamepad_axis axis, vec2 pos );

	static unsigned allocate_port();
	static void release_port( unsigned port );
};
}

extern detail::gamepad_state gamepad[detail::max_gamepads];

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
struct space_navigator_state
{
	vec3 pos, rot;
	bool button_down[+space_navigator_button::__count];
};
}

extern detail::space_navigator_state space_navigator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern detail::callback_chain<void()> on_tick;
extern detail::callback_chain<bool( event & )> on_event;

} // namespace gl3d

#endif // __GL3D_BASE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_BASE_H_IMPL__
		#define __GL3D_BASE_H_IMPL__
		#include "gl3d_base.inl"
	#endif // __GL3D_BASE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
