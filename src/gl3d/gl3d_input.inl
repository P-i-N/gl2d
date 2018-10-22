#ifndef __GL3D_INPUT_H_IMPL__
	#define __GL3D_INPUT_H_IMPL__
#endif

#include "gl3d_input.h"

namespace gl3d {

static detail::keyboard_state keyboard;
static detail::mouse_state mouse;
static detail::gamepad_state gamepad[GL3D_MAX_GAMEPADS];
static detail::space_navigator_state space_navigator;

static decltype( on_tick ) on_tick;
static decltype( on_event ) on_event;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
void keyboard_state::change_key_state( key k, bool down, unsigned id )
{
	bool old = ( *this )[k];
	if ( old != down )
	{
		key_down[static_cast<size_t>( k )] = down;
		event e( down ? event_type::key_down : event_type::key_up, id );
		e.keyboard.k = k;
		e.keyboard.ch = 0;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_button_state( mouse_button b, bool down, unsigned id )
{
	bool old = ( *this )[b];
	if ( old != down )
	{
		button_down[static_cast<size_t>( b )] = down;
		event e( down ? event_type::mouse_down : event_type::mouse_up, id );
		e.mouse.b = b;
		e.mouse.x = this->x;
		e.mouse.y = this->y;
		e.mouse.dx = e.mouse.dy = 0;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_position( int mx, int my, unsigned id )
{
	if ( x != mx || y != my )
	{
		event e( event_type::mouse_move, id );
		e.mouse.b = mouse_button::unknown;
		e.mouse.x = mx;
		e.mouse.y = my;
		e.mouse.dx = mx - this->x;
		e.mouse.dy = my - this->y;
		this->x = mx;
		this->y = my;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_button_state( gamepad_button b, bool down )
{
	bool old = ( *this )[b];
	if ( old != down )
	{
		button_down[static_cast<size_t>( b )] = down;
		event e( down ? event_type::gamepad_down : event_type::gamepad_up, UINT_MAX );
		e.gamepad.port = port;
		e.gamepad.b = b;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_axis_state( gamepad_axis ax, float x, float y )
{
	float oldX = axis_x[static_cast<size_t>( ax )];
	float oldY = axis_y[static_cast<size_t>( ax )];
	if ( oldX != x || oldY != y )
	{
		axis_x[static_cast<size_t>( ax )] = x;
		axis_y[static_cast<size_t>( ax )] = y;
		event e( event_type::gamepad_move, UINT_MAX );
		e.gamepad.port = port;
		e.gamepad.axis = ax;
		e.gamepad.x = x;
		e.gamepad.y = y;
		e.gamepad.dx = x - oldX;
		e.gamepad.dy = y - oldY;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
int gamepad_state::allocate_port()
{
	for ( int i = 0; i < GL3D_MAX_GAMEPADS; ++i )
		if ( gamepad[i].port < 0 )
			return gamepad[i].port = i;

	return -1;
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::release_port( int port )
{
	if ( port >= 0 && port < GL3D_MAX_GAMEPADS && gamepad[port].port == port )
		gamepad[port].port = -1;
}

} // namespace gl3d::detail

} // namespace gl3d
