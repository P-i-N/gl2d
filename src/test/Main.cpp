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

	window::open( "Main Window", { 1920, 1200 }, { 1920 * 0 + 60, 60 } );

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

	//auto t = texture::create(gl_enum::TEXTURE_2D)

	auto q = cmd_queue::create();
	q->bind_shader( shader::create( sc ) );
	q->set_uniform_block( 0, fd );
	q->bind_vertex_buffer( buffer::create( buffer_usage::immutable, vertices ), Vertex::get_layout() );
	q->draw( gl_enum::TRIANGLES, 0, 3 );

	on_tick += [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();

		ctx->clear_color( { 0.1f, 0.2f, 0.3f, 1.0f } );
		ctx->execute( q );
	};

	run();
	return 0;
}
