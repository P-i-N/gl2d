#ifndef __GL3D_CMD_LIST_H__
#define __GL3D_CMD_LIST_H__

#include "gl3d.h"

namespace gl3d {

#pragma region State objects

struct rasterizer_state
{
	GLenum face_cull_mode = GL_NONE;
	bool front_ccw = false;
	bool wireframe = false;
	bool depth_clamp = false;
	bool scissor_test = false;

	void bind();
};

struct blend_state
{
	struct slot_desc
	{
		GLenum src = GL_ONE;
		GLenum dst = GL_ZERO;
		GLenum op = detail::gl_api::FUNC_ADD;
		GLenum src_alpha = GL_ONE;
		GLenum dst_alpha = GL_ZERO;
		GLenum op_alpha = GL_NONE;
	} slot[8];

	std::bitset<8> enabled_slots = { 0 };

	void bind();
};

struct depth_stencil_state
{
	GLenum depth_func = GL_LESS;
	uint8_t stencil_read_mask = 0;
	uint8_t stencil_write_mask = 0;
	bool stencil_test = false;
	bool depth_test = true;
	bool depth_write = true;

	void bind();
};

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class cmd_list_primitive
{
	none = 0,
	points,
	lines,
	line_strip,
	triangles,
	textured_triangles,
	triangle_strip,
	textured_triangle_strip,
	quads,
	textured_quads,
	quad_strip,
	textured_quad_strip
};

enum class cmd_list_shading
{
	none = 0,
	flat,
	smooth
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class cmd_list
{
public:
	using ptr = std::shared_ptr<cmd_list>;

	cmd_list() = default;
	virtual ~cmd_list() = default;

	void render();

	void reset();

	void set_rasterizer_state( const rasterizer_state &rs );
	void set_blend_state( const blend_state &bs );
	void set_depth_stencil_state( const depth_stencil_state &ds );

	void begin( cmd_list_primitive primitive );
	void end();

	void set_shading( cmd_list_shading shading );
	cmd_list_shading shading() const { return _current_shading; }

	void vertex( const vec4 &v );
	void vertex( const vec3 &v, float w = 0.0f ) { vertex( { v.x, v.y, v.z, w } ); }
	void vertex( const vec2 &v, float z = 0.0f, float w = 0.0f ) { vertex( { v.x, v.y, z, w } ); }
	void vertexi( const ivec4 &v ) { return vertex( vec4( v ) ); }
	void vertexi( const ivec3 &v, int w = 0 ) { return vertex( vec4( v.x, v.y, v.z, w ) ); }
	void vertexi( const ivec2 &v, int z = 0, int w = 0 ) { return vertex( vec4( v.x, v.y, z, w ) ); }

	void color( const vec4 &c );
	void color( const vec3 &c, float a = 1.0f ) { color( { c.x, c.y, c.z, a } ); }

	void uv( const vec2 &coord );

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

	pipeline_state _current_pipeline_state;
	bool _dirty_pipeline_state = true;

	struct internal_vertex : layout<vertex_pos<vec3>, vertex_normal<vec3>, vertex_color<vec4>, vertex_uv0<vec2>> { };

	cmd_list_primitive _current_primitive = cmd_list_primitive::none;
	cmd_list_shading _current_shading = cmd_list_shading::none;
	internal_vertex _current_vertex;

	vertex_buffer<internal_vertex>::ptr _vertices;
	index_buffer::ptr _indices;
};

}

#endif // __GL3D_CMD_LIST_H__

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_CMD_LIST_H_IMPL__
#define __GL3D_CMD_LIST_H_IMPL__

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::render()
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::reset()
{
	_pipeline_states.clear();

	_vertices->clear();
	_indices->clear();
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::begin( cmd_list_primitive primitive )
{
	assert( _current_primitive == cmd_list_primitive::none );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::end()
{
	_current_primitive = cmd_list_primitive::none;
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::set_shading( cmd_list_shading shading )
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::vertex( const vec4 &v )
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::color( const vec4 &c )
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::uv( const vec2 &coord )
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::set_rasterizer_state( const rasterizer_state &rs )
{
	_current_pipeline_state.rs = rs;
	_dirty_pipeline_state = true;
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::set_blend_state( const blend_state &bs )
{
	_current_pipeline_state.bs = bs;
	_dirty_pipeline_state = true;
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::set_depth_stencil_state( const depth_stencil_state &ds )
{
	_current_pipeline_state.ds = ds;
	_dirty_pipeline_state = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void rasterizer_state::bind()
{
	glFrontFace( front_ccw ? GL_CCW : GL_CW );
	face_cull_mode != GL_NONE ? ( glEnable( GL_CULL_FACE ), glCullFace( face_cull_mode ) ) : glDisable( GL_CULL_FACE );
	glPolygonMode( GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL );
	depth_clamp ? glEnable( gl.DEPTH_CLAMP ) : glDisable( gl.DEPTH_CLAMP );
	scissor_test ? glEnable( GL_SCISSOR_TEST ) : glDisable( GL_SCISSOR_TEST );
}

//---------------------------------------------------------------------------------------------------------------------
void blend_state::bind()
{
	for ( GLuint i = 0; i < 8; ++i )
	{
		enabled_slots[i] ? gl.Enablei( GL_BLEND, i ) : gl.Disablei( GL_BLEND, i );
		gl.BlendFunci( i, slot[i].src, slot[i].dst );
		gl.BlendEquationi( i, slot[i].op );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void depth_stencil_state::bind()
{
	depth_test ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
	stencil_test ? glEnable( GL_STENCIL_TEST ) : glDisable( GL_STENCIL_TEST );
	glDepthMask( depth_write ? GL_TRUE : GL_FALSE );
	glDepthFunc( depth_func );
}

}

#endif // __GL3D_CMD_LIST_H_IMPL__
#endif // GL3D_IMPLEMENTATION
