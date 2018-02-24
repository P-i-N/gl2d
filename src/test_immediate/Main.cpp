#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

#include <chrono>

using namespace gl3d;

class cmd_list
{
public:

protected:
	struct draw_call
	{
		std::shared_ptr<buffer> vertex_buffer;
		std::shared_ptr<buffer> index_buffer;

		size_t rasterizer_state_index;
		size_t blend_state_index;
		size_t depth_stencil_state_index;
	};
};

int main()
{
	auto desc = vertex3d::layout_desc();

	window_open( "Example", 400, 300 );

	on_event( [&]( event & e )
	{
		if ( e.canceled )
			return;

		if ( e.type == event_type::paint )
		{
			auto size = get_window_size( e.window_id );
			float aspectRatio = static_cast<float>( size.x ) / size.y;

		}
	} );

	run();
	return 0;
}
