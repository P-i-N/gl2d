#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

#include <chrono>

struct Vertex
{
	gl3d::vec3 pos;
	gl3d::vec4 color;

	GL3D_LAYOUT( 0, &Vertex::pos, 3, &Vertex::color );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	using namespace gl3d;

	on_event( [&]( event & e )
	{
		switch ( e.type )
		{
			case event_type::open:
				printf( "open(%d)\n", e.window_id );
				break;

			case event_type::close:
				printf( "close(%d)\n", e.window_id );
				break;

			case event_type::resize:
				printf( "resize(%d): %d %d\n", e.window_id, e.resize.x, e.resize.y );
				break;

			case event_type::move:
				printf( "move(%d): %d %d\n", e.window_id, e.move.x, e.move.y );
				break;


			case event_type::mouse_move:
				printf( "mouse_move(%d): %d %d %d %d\n",
				        e.window_id,
				        e.mouse.pos.x, e.mouse.pos.y,
				        e.mouse.delta.x, e.mouse.delta.y );
				break;
			/* */

			case event_type::mouse_down:
				printf( "mouse_down(%d): %d\n", e.window_id, +e.mouse.b );
				break;

			case event_type::mouse_up:
				printf( "mouse_up(%d): %d\n", e.window_id, +e.mouse.b );
				break;
		}
	} );

	auto q = std::make_shared<cmd_queue>();
	q->clear_color( { 0.1f, 0.2f, 0.4f, 1.0f } );

	on_tick( [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();

		ctx->execute( q );
	} );

	auto win = window::open( "Main Window", { 800, 600 } );
	auto ctx = win->context();

	auto s = sizeof( cmd_queue );

	gl3d::run();
	return 0;
}
