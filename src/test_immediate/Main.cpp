#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

#include <chrono>

namespace gl3d {

class cmd_list
{
public:
	using ptr = std::shared_ptr<cmd_list>;

	cmd_list();

	virtual ~cmd_list() = default;

protected:
	struct draw_call
	{
		size_t pipeline_state_index;
	};

	struct pipeline_state
	{
		rasterizer_state rs;
		blend_state bs;
		depth_stencil_state ds;
	};

	std::vector<pipeline_state> _pipeline_states;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
cmd_list::cmd_list()
{

}

}

int main()
{
	using namespace gl3d;

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
