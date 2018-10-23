#ifndef __GL3D_INPUT_H__
#define __GL3D_INPUT_H__

#include <limits.h>
#include <type_traits>

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
enum class window_flag
{
	none = 0,
	resizable = 0b00000001,
	fullscreen = 0b00000010,
	title = 0b00000100,
};

inline auto operator +( window_flag t ) { return static_cast<std::underlying_type_t <window_flag>>( t ); }

static const unsigned default_window_flags = ( +window_flag::resizable ) | ( +window_flag::title );

//---------------------------------------------------------------------------------------------------------------------
struct framestamp
{
	unsigned frame_id;
	float time, delta;
};

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

	last
};

//---------------------------------------------------------------------------------------------------------------------
enum class mouse_button
{
	unknown = 0,
	left, right, middle,
	special_0, special_1,
	back, forward,

	last
};

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_button
{
	unknown = 0,
	a, b, x, y,
	up, right, down, left,
	shoulder_left, shoulder_right,
	thumb_left, thumb_right,

	last
};

//---------------------------------------------------------------------------------------------------------------------
enum class gamepad_axis
{
	unknown = 0,
	thumb_left, thumb_right,
	trigger_left, trigger_right,

	last
};

//---------------------------------------------------------------------------------------------------------------------
enum class space_navigator_button
{
	unknown = 0,
	left, right,

	last
};

//---------------------------------------------------------------------------------------------------------------------
enum class event_type
{
	unknown = 0,
	run, quit,
	paint,
	open, close, resize,
	key_down, key_up, key_press,
	mouse_down, mouse_up, mouse_move, mouse_wheel,
	gamepad_down, gamepad_up, gamepad_move, gamepad_connect, gamepad_disconnect
};

//---------------------------------------------------------------------------------------------------------------------
struct event
{
	bool canceled = false;
	event_type type;
	unsigned window_id;

	union
	{
		struct { key k; int ch; } keyboard;
		struct { int x, y; } resize;
		struct { int x, y, dx, dy; mouse_button b; } mouse;
		struct { int dx, dy; } wheel;
		struct { int port; float x, y, dx, dy; gamepad_button b; gamepad_axis axis; } gamepad;
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
	bool key_down[static_cast<size_t>( key::last )];
	bool operator[]( key k ) const { return key_down[static_cast<size_t>( k )]; }

	void change_key_state( key k, bool down, unsigned id = UINT_MAX );
};
}

extern detail::keyboard_state keyboard;

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
struct mouse_state
{
	bool button_down[static_cast<size_t>( mouse_button::last )];
	bool operator[]( mouse_button b ) const { return button_down[static_cast<size_t>( b )]; }
	int x, y;

	void change_button_state( mouse_button b, bool down, unsigned id = UINT_MAX );
	void change_position( int mx, int my, unsigned id = UINT_MAX );
};
}

extern detail::mouse_state mouse;

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
static constexpr unsigned max_gamepads = 8;

struct gamepad_state
{
	int port = -1;
	bool button_down[static_cast<size_t>( gamepad_button::last )];
	float axis_x[static_cast<size_t>( gamepad_axis::last )];
	float axis_y[static_cast<size_t>( gamepad_axis::last )];
	bool operator[]( gamepad_button b ) const { return button_down[static_cast<size_t>( b )]; }
	bool connected() const { return port >= 0 && port < max_gamepads; }

	void change_button_state( gamepad_button b, bool down );
	void change_axis_state( gamepad_axis ax, float x, float y );

	static int allocate_port();
	static void release_port( int port );
};
}

extern detail::gamepad_state gamepad[detail::max_gamepads];

//---------------------------------------------------------------------------------------------------------------------
namespace detail {
struct space_navigator_state
{
	float pos_x, pos_y, pos_z;
	float rot_x, rot_y, rot_z;
	bool button_down[static_cast<size_t>( space_navigator_button::last )];
};
}

extern detail::space_navigator_state space_navigator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

}

extern detail::callback_list<void()> on_tick;
extern detail::callback_list<void( event & )> on_event;

}

#endif // __GL3D_INPUT_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_INPUT_H_IMPL__
		#define __GL3D_INPUT_H_IMPL__
		#include "gl3d_input.inl"
	#endif // __GL3D_INPUT_H_IMPL__
#endif // GL3D_IMPLEMENTATION
