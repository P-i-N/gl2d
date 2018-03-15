#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>
#include <gl3d/gl3d_cmd_list.h>

#include <chrono>

int main()
{
	using namespace gl3d;

	auto desc = vertex3d::layout_desc();

	window_open( "Example", 400, 300 );

	cmd_list cl;

	on_event( [&]( event & e )
	{
		if ( e.canceled )
			return;

		if ( e.type == event_type::paint )
		{
			auto size = get_window_size( e.window_id );
			float aspectRatio = static_cast<float>( size.x ) / size.y;

			cl.render();
		}
	} );

	on_tick( [&]()
	{
		cl.reset();

		cl.begin( cmd_list_primitive::lines );
		cl.vertex( { 0, 0 } );
		cl.vertex( { 1, 1 } );
		cl.end();

	} );

	run();
	return 0;
}
