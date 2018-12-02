#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

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

	// Mount folder with example data
	vfs::mount( "../../data" );

	window::create( "Main Window", { 1280, 800 } );

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
	sc->load( "shaders/Test.shader" );

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

	q->bind_vertex_buffer( buffer::create( buffer_usage::immutable, vertices ), Vertex::layout() );
	q->draw( gl_enum::TRIANGLES, 0, 3 );

	auto qd3D = std::make_shared<quick_draw>();
	qd3D->begin( gl_enum::QUADS );
	{
		// Top
		qd3D->color( { 1, 1, 1 } );
		qd3D->vertex( { 1, 1, 1 } );
		qd3D->vertex( { 1, -1, 1 } );
		qd3D->vertex( { -1, -1, 1 } );
		qd3D->vertex( { -1, 1, 1 } );

		// Bottom
		qd3D->color( { 1, 1, 0 } );
		qd3D->vertex( { 1, 1, -1 } );
		qd3D->vertex( { 1, -1, -1 } );
		qd3D->vertex( { -1, -1, -1 } );
		qd3D->vertex( { -1, 1, -1 } );

		// Front
		qd3D->color( { 0, 1, 0 } );
		qd3D->vertex( { -1, 1, 1 } );
		qd3D->vertex( { -1, -1, 1 } );
		qd3D->vertex( { -1, -1, -1 } );
		qd3D->vertex( { -1, 1, -1 } );

		// Back
		qd3D->color( { 0, 0, 1 } );
		qd3D->vertex( { 1, 1, 1 } );
		qd3D->vertex( { 1, -1, 1 } );
		qd3D->vertex( { 1, -1, -1 } );
		qd3D->vertex( { 1, 1, -1 } );
	}
	qd3D->end();

	on_tick += [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();
	};

	on_window_event += [&]( window_event & e )->bool
	{
		switch ( e.event_type )
		{
			case window_event::type::paint:
			{
				auto w = window::from_id( e.window_id );
				auto ctx = w->context();

				ctx->clear_color( { 0.1f, 0.2f, 0.4f, 1.0f } );
				ctx->clear_depth( 1.0f );

				qd3D->render( ctx, mat4::make_inverse( mat4::make_look_at( -2, 0, 2, 0, 0, 0, 0, 0, 1 ) ), mat4::make_perspective( 100.0f, w->aspect_ratio(), 0.01f, 1000.0f ) );
			}
			break;
		}

		return false;
	};

	run();
	return 0;
}
