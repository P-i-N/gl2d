#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

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

	std::vector<Vertex> vertices;
	vertices.push_back( { {  0, -1, 0 }, vec4::red() } );
	vertices.push_back( { {  1,  1, 0 }, vec4::green() } );
	vertices.push_back( { { -1,  1, 0 }, vec4::blue() } );

	auto sc = std::make_shared<shader_code>();
	sc->load( "../../data/shaders/Test.shader" );

	auto q = std::make_shared<cmd_queue>();
	q->bind_shader( std::make_shared<shader>( sc ) );
	q->bind_vertex_buffer( std::make_shared<buffer>( vertices ), Vertex::get_layout() );
	q->draw( gl_enum::TRIANGLES, 0, 3 );

	on_tick( [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();

		ctx->clear_color( { 0.1f, 0.2f, 0.3f, 1.0f } );
		ctx->execute( q );
	} );

	window::open( "Main Window", { 800, 600 } /*, { 1920 * 0 + 60, 60 }*/ );
	run();
	return 0;
}
