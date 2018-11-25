#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>
#include <gl3d/gl3d_2d.h>

struct Vertex
{
	gl3d::vec3 pos;
	gl3d::vec4 color;

	GL3D_LAYOUT( 0, &Vertex::pos, 3, &Vertex::color );
};

struct FrameData
{
	gl3d::mat4 ProjectionMatrix;
	gl3d::mat4 ViewMatrix;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	using namespace gl3d;

	window::create( "Main Window", { 1280, 800 }, { 1920 * 0 + 60, 60 } );

	Vertex vertices[] =
	{
		{ {  0, -1, 0 }, vec4::red() },
		{ {  1,  1, 0 }, vec4::green() },
		{ { -1,  1, 0 }, vec4::blue() }
	};

	FrameData fd;
	fd.ProjectionMatrix = gl3d::mat4();
	fd.ViewMatrix = gl3d::mat4();

	auto sc = std::make_shared<shader_code>();
	sc->load( "../../data/shaders/Test.shader" );

	uint8_t checkerboard[] =
	{
		0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF
	};

	auto t = texture::create( gl_format::RGB8, uvec2{ 4, 4 }, checkerboard );

	auto q = cmd_queue::create();
	q->bind_shader( shader::create( sc ) );
	q->bind_texture( t, 0 );

	q->set_uniform_block( 0, fd );
	q->set_uniform( "u_Diffuse", 0 );

	q->bind_vertex_buffer( buffer::create( buffer_usage::immutable, vertices ), Vertex::get_layout() );
	q->draw( gl_enum::TRIANGLES, 0, 3 );

	on_tick += [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();

		ctx->clear_color( { 0.0f, 0.0f, 0.0f, 1.0f } );
		ctx->execute( q );
	};

	on_window_event += [&]( window_event & e )->bool
	{
		switch ( e.event_type )
		{
			case window_event::type::paint:
			{
				auto w = window::from_id( e.window_id );
				auto imm = w->immediate();

				imm->begin( gl_enum::LINES );
				imm->vertex( { 0, 0 } );
				imm->vertex( w->size() );
				imm->end();

				printf( "%f (%.1f fps)\n", gl3d::time, 1.0f / gl3d::delta );
			}
			break;
		}

		return false;
	};

	run();
	return 0;
}
