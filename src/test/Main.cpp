#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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

class TrueTypeFont
{
public:
	TrueTypeFont( int defaultSize = 1024 )
		: m_DefaultSize( defaultSize )
	{
		m_Ranges.emplace_back( 32, 96 );
	}

	virtual ~TrueTypeFont()
	{
		Clear();
	}

	void Clear()
	{

	}

	void Rebuild()
	{

	}

	bool LoadFromFile( const std::filesystem::path &fileName )
	{
		gl3d::detail::bytes_t bytes;
		if ( !gl3d::vfs::load( fileName, bytes ) )
			return false;

		m_LoadedFont = std::make_unique<uint8_t[]>( bytes.size() );
		memcpy( m_LoadedFont.get(), bytes.data(), bytes.size() );

		Rebuild();
		return true;
	}

	int GetNumPackedCharacters() const
	{
		int result = 0;
		for ( auto &r : m_Ranges )
			result += r.second;

		return result;
	}

	int GetOffsetInPackedRanges( int ch ) const
	{
		int result = 0;
		for ( auto &r : m_Ranges )
		{
			if ( ch >= r.first && ch < ( r.first + r.second ) )
				return result + ( ch - r.first );

			result += r.second;
		}

		return -1;
	}

	bool EnsurePageExists( float size, int textureWidth = 0, int textureHeight = 0 )
	{
		auto iter = m_Pages.find( size );
		if ( iter != m_Pages.end() )
			return true;

		if ( !textureWidth )
			textureWidth = m_DefaultSize;

		if ( !textureHeight )
			textureHeight = m_DefaultSize;

		std::unique_ptr<Page> page( new Page( size, textureWidth, textureHeight, GetNumPackedCharacters() ) );
		RebuildPage( *page );

		m_Pages.insert( { size, std::move( page ) } );
		return false;
	}

	bool EmitVertices( float size, float x, float y, const char *text )
	{
		EnsurePageExists( size );

		auto iter = m_Pages.find( size );
		if ( iter == m_Pages.end() )
			return false;

		Page &page = *( iter->second );
		stbtt_aligned_quad q;

		while ( text )
		{
			auto off = GetOffsetInPackedRanges( *text );
			stbtt_GetPackedQuad( page.m_PackedChars.data(), page.m_Width, page.m_Height, off, &x, &y, &q, 0 );

			++text;
		}

		return true;
	}

protected:
	struct Page
	{
		float m_Size = 0.0f;
		int m_Width = 0;
		int m_Height = 0;
		std::unique_ptr<uint8_t[]> m_Pixels;
		std::vector<stbtt_packedchar> m_PackedChars;
		stbtt_pack_context m_PackContext;
		gl3d::texture::ptr m_Texture;

		Page( float size, int width, int height, int totalPackedCharsCount )
			: m_Size( size )
			, m_Width( width )
			, m_Height( height )
			, m_Pixels( new uint8_t[width * height] )
			, m_PackedChars( totalPackedCharsCount )
		{
			stbtt_PackBegin( &m_PackContext, m_Pixels.get(), m_Width, m_Height, 0, 0, nullptr );
		}

		~Page()
		{
			stbtt_PackEnd( &m_PackContext );
		}

		void Clear()
		{

		}
	};

	bool RebuildPage( Page &page )
	{
		int packedCharsOffset = 0;
		std::vector<stbtt_pack_range> packRanges;
		for ( auto &r : m_Ranges )
		{
			auto &packRange = packRanges.emplace_back();
			packRange.font_size = page.m_Size;
			packRange.first_unicode_codepoint_in_range = r.first;
			packRange.array_of_unicode_codepoints = nullptr;
			packRange.num_chars = r.second;
			packRange.chardata_for_range = page.m_PackedChars.data() + packedCharsOffset;

			packedCharsOffset += r.second;
		}

		if ( !stbtt_PackFontRanges(
		            &page.m_PackContext,
		            m_LoadedFont.get(), 0,
		            packRanges.data(), static_cast<int>( packRanges.size() ) ) )
		{
			return false;
		}

		page.m_Texture = gl3d::texture::create(
		                     gl3d::gl_format::R8,
		                     gl3d::uvec2{ page.m_Width, page.m_Height },
		                     page.m_Pixels.get(), false, false );

		return true;
	}

	int m_DefaultSize = 0;
	std::unique_ptr<uint8_t[]> m_LoadedFont;
	std::vector<std::pair<int, int>> m_Ranges;
	std::unordered_map<float, std::unique_ptr<Page>> m_Pages;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	using namespace gl3d;

	// Mount folder with example data
	vfs::mount( "../../data" );

	fps_limit = 125;

	/*
	TrueTypeFont ttf;
	ttf.LoadFromFile( "fonts/OpenSans-Regular.ttf" );
	ttf.EmitVertices( 16.0f, 0, 0, "Hello, world!" );
	*/

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

	auto q = cmd_queue::create();
	q->bind_shader( shader::create( sc ) );
	q->bind_texture( texture::checkerboard(), 0 );

	q->set_uniform_block( 0, fd );
	q->set_uniform( "u_Diffuse", 0 );

	q->bind_vertex_buffer( buffer::create( buffer_usage::immutable, vertices ), Vertex::layout() );
	q->draw( gl_enum::TRIANGLES, 0, 3 );

	auto qd3D = std::make_shared<quick_draw>();

	qd3D->bind_texture( texture::checkerboard() );
	qd3D->begin( gl_enum::QUADS );
	{
		// Top
		qd3D->color( { 1, 1, 1 } );
		qd3D->uv( { 0, 0 } );
		qd3D->vertex( { 1, 1, 1 } );
		qd3D->uv( { 1, 0 } );
		qd3D->vertex( { 1, -1, 1 } );
		qd3D->uv( { 1, 1 } );
		qd3D->vertex( { -1, -1, 1 } );
		qd3D->uv( { 0, 1 } );
		qd3D->vertex( { -1, 1, 1 } );
	}
	qd3D->end();

	qd3D->bind_texture( nullptr );
	qd3D->begin( gl_enum::QUADS );
	{
		// Bottom
		qd3D->color( { 1, 1, 0 } );
		qd3D->vertex( { 1, 1, -1 } );
		qd3D->vertex( { -1, 1, -1 } );
		qd3D->vertex( { -1, -1, -1 } );
		qd3D->vertex( { 1, -1, -1 } );

		// Front
		qd3D->color( { 0, 1, 0 } );
		qd3D->vertex( { -1, 1, 1 } );
		qd3D->vertex( { -1, -1, 1 } );
		qd3D->vertex( { -1, -1, -1 } );
		qd3D->vertex( { -1, 1, -1 } );

		// Back
		qd3D->color( { 0, 0, 1 } );
		qd3D->vertex( { 1, 1, -1 } );
		qd3D->vertex( { 1, -1, -1 } );
		qd3D->vertex( { 1, -1, 1 } );
		qd3D->vertex( { 1, 1, 1 } );

		// Left
		qd3D->color( { 1, 0, 1 } );
		qd3D->vertex( { 1, -1, -1 } );
		qd3D->vertex( { -1, -1, -1 } );
		qd3D->vertex( { -1, -1, 1 } );
		qd3D->vertex( { 1, -1, 1 } );

		// Right
		qd3D->color( { 1, 0, 0 } );
		qd3D->vertex( { 1, 1, 1 } );
		qd3D->vertex( { -1, 1, 1 } );
		qd3D->vertex( { -1, 1, -1 } );
		qd3D->vertex( { 1, 1, -1 } );
	}
	qd3D->end();

	vec2 rot = { 0, 0 };

	on_tick += [&]()
	{
		auto w = window::from_id( 0 );
		auto ctx = w->context();

		rot.y = 0;

		for ( auto &g : gamepad )
		{
			if ( !g.connected() )
				continue;

			rot.x = rot.x + g.axis[+gamepad_axis::thumb_right].pos.x * delta * 5.0f;
			rot.y += g.axis[+gamepad_axis::thumb_right].pos.y;
		}
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

				float x = 3.0f * sin( rot.x );
				float y = 3.0f * cos( rot.x );
				float z = 3.0f + 2.0f * rot.y;

				qd3D->render( ctx,
				              mat4::make_inverse( mat4::make_look_at( vec3{ x, y, z }, vec3(), vec3::unit_z() ) ),
				              mat4::make_perspective( 90.0f, w->aspect_ratio(), 0.01f, 1000.0f ) );
			}
			break;
		}

		return false;
	};

	run();
	return 0;
}
