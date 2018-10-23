#ifndef __GL3D_INPUT_H_IMPL__
	#define __GL3D_INPUT_H_IMPL__
#endif

#include "gl3d_input.h"

namespace gl3d {

static detail::keyboard_state keyboard;
static detail::mouse_state mouse;
static detail::gamepad_state gamepad[detail::max_gamepads];
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
		e.mouse.pos = pos;
		e.mouse.delta = { 0, 0 };
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_position( ivec2 pos, unsigned id )
{
	if ( this->pos != pos )
	{
		event e( event_type::mouse_move, id );
		e.mouse.b = mouse_button::unknown;
		e.mouse.pos = pos;
		e.mouse.delta = pos - this->pos;
		this->pos = pos;
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
void gamepad_state::change_axis_state( gamepad_axis axis, vec2 pos )
{
	auto oldPos = this->pos[static_cast<size_t>( axis )];
	if ( oldPos != pos )
	{
		this->pos[static_cast<size_t>( axis )] = pos;
		event e( event_type::gamepad_move, UINT_MAX );
		e.gamepad.port = port;
		e.gamepad.axis = axis;
		e.gamepad.pos = pos;
		e.gamepad.delta = pos - oldPos;
		on_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
int gamepad_state::allocate_port()
{
	for ( int i = 0; i < max_gamepads; ++i )
		if ( gamepad[i].port < 0 )
			return gamepad[i].port = i;

	return -1;
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::release_port( int port )
{
	if ( port >= 0 && port < max_gamepads && gamepad[port].port == port )
		gamepad[port].port = -1;
}

} // namespace gl3d::detail

} // namespace gl3d
