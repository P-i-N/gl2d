#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

#include <chrono>

namespace gl3d {

struct vertex_layout
{
	struct attr { unsigned location, offset, element_count, element_type; };

	std::vector<attr> attribs;
	unsigned mask = 0;

	template <typename... Args>
	vertex_layout( Args &&... args ): attribs( sizeof...( Args ) / 2 ) { init( 0, args... ); }

private:
	template <typename T> struct type { };

	void fill( attr &a, unsigned loc, unsigned off, type<int> ) { a = { loc, off, 1, GL_INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<float> ) { a = { loc, off, 1, GL_FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec2> ) { a = { loc, off, 2, GL_FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec2> ) { a = { loc, off, 2, GL_INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec3> ) { a = { loc, off, 3, GL_FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec3> ) { a = { loc, off, 3, GL_INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<vec4> ) { a = { loc, off, 4, GL_FLOAT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<ivec4> ) { a = { loc, off, 4, GL_INT }; }
	void fill( attr &a, unsigned loc, unsigned off, type<byte_vec4> ) { a = { loc, off, 4, GL_UNSIGNED_BYTE }; }

	template <typename T1, typename T2, typename... Args>
	void init( unsigned index, unsigned location, T1 T2::*member, Args &&... args )
	{
		fill( attribs[index], location, unsigned( size_t( &( ( ( T2 * )0 )->*member ) ) ), type<T1>() );
		mask |= ( 1u << location );

		if constexpr ( sizeof...( Args ) >= 2 )
			init( index + 1, args... );
	}
};

}

struct Vertex
{
	gl3d::vec3 pos;
	gl3d::vec4 color;

	static const gl3d::vertex_layout &get_layout()
	{
		static gl3d::vertex_layout layout { 0, &Vertex::pos, 3, &Vertex::color };
		return layout;
	}
};

int main()
{
	const auto &l = Vertex::get_layout();

	using namespace gl3d;

	/*
	// Triangle geometry
	auto geom = std::make_shared<geometry<vertex3d>>();
	auto vertices = geom->alloc_vertices( 3 );

	vertices->pos = vec3( 0,  1, 0 );
	vertices->color = vec4::red();
	++vertices;

	vertices->pos = vec3( -1, -1, 0 );
	vertices->color = vec4::green();
	++vertices;

	vertices->pos = vec3( 1, -1, 0 );
	vertices->color = vec4::blue();
	++vertices;

	window_open( "Example", 400, 300 );

	on_event( [&]( event & e )
	{
		if ( e.canceled )
			return;

		if ( e.type == event_type::paint )
		{
			auto size = get_window_size( e.window_id );
			float aspectRatio = static_cast<float>( size.x ) / size.y;

			auto ctx = state.ctx3d;
			ctx->bind_geometry( geom );
			ctx->set_uniform( GL3D_UNIFORM_PROJECTION_MATRIX, mat4::make_perspective( 60.0f, aspectRatio, 0.01f, 1000.0f ) );
			ctx->set_uniform( GL3D_UNIFORM_MODELVIEW_MATRIX, mat4::make_inverse( mat4::make_look_at( 5.0f * sin( state.time ), 2.0f, 5.0f * cos( state.time ), 0.0f, 0.0f, 0.0f ) ) );
			ctx->draw();
		}
	} );

	on_tick( [&]()
	{
		auto ctx = state.ctx2d;
		int y = 16;

		if ( gamepad[0].connected() )
		{
			if ( gamepad[0][gamepad_button::a] ) ctx->texti( 16, y += 16, "^AButton A" );
			if ( gamepad[0][gamepad_button::b] ) ctx->texti( 16, y += 16, "^CButton B" );
			if ( gamepad[0][gamepad_button::x] ) ctx->texti( 16, y += 16, "^9Button X" );
			if ( gamepad[0][gamepad_button::y] ) ctx->texti( 16, y += 16, "^EButton Y" );
			if ( gamepad[0][gamepad_button::thumb_left] ) ctx->texti( 16, y += 16, "Thumb Left" );
			if ( gamepad[0][gamepad_button::thumb_right] ) ctx->texti( 16, y += 16, "Thumb Right" );
			if ( gamepad[0][gamepad_button::shoulder_left] ) ctx->texti( 16, y += 16, "Shoulder Left" );
			if ( gamepad[0][gamepad_button::shoulder_right] ) ctx->texti( 16, y += 16, "Shoulder Right" );
			if ( gamepad[0][gamepad_button::up] ) ctx->texti( 16, y += 16, "UP" );
			if ( gamepad[0][gamepad_button::down] ) ctx->texti( 16, y += 16, "DOWN" );
			if ( gamepad[0][gamepad_button::left] ) ctx->texti( 16, y += 16, "LEFT" );
			if ( gamepad[0][gamepad_button::right] ) ctx->texti( 16, y += 16, "RIGHT" );

			auto size = get_window_size();
			float cx = size.x / 2.0f - size.x / 4.0f;
			float r = size.x / 8.0f;
			float cy = size.y / 2.0f + r * gamepad[0].axis_x[3];;

			ctx->line( cx, cy, cx + r * gamepad[0].axis_x[1], cy - r * gamepad[0].axis_y[1] );

			cx = size.x / 2.0f + size.x / 4.0f;
			cy = size.y / 2.0f + r * gamepad[0].axis_x[4];;
			ctx->line( cx, cy, cx + r * gamepad[0].axis_x[2], cy - r * gamepad[0].axis_y[2] );
		}

		ctx->texti( 8, 8, "Hello, world!" );
	} );
	*/

	run();
	return 0;
}
