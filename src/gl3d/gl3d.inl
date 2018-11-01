#ifndef __GL3D_H_IMPL__
	#define __GL3D_H_IMPL__
#endif

#include "gl3d.h"
#include "gl3d_base.h"

#if defined(_MSC_VER)
	#pragma comment(lib, "opengl32.lib")
#endif

#if defined(WIN32)
	#ifndef VC_EXTRALEAN
		#define VC_EXTRALEAN
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#error Not implemented!
#endif

#include <gl/GL.h>

#include <cassert>
#include <fstream>
#include <sstream>

namespace gl3d {

decltype( gl::CreateContextAttribsARB ) gl::CreateContextAttribsARB;

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
bool unroll_includes( std::stringstream &ss, std::string_view sourceCode, const std::filesystem::path &cwd )
{
	bool result = true;

	for_each_line( sourceCode, [&]( std::string_view line, unsigned lineNum )
	{
		if ( !result ) return;

		bool addLine = true;

		if ( auto dir = trim( line ); !dir.empty() && dir[0] == '#' )
		{
			addLine = false;
			dir = trim( line.substr( 1 ) ); // Cut away '#' & trim
			if ( starts_with_nocase( dir, "include" ) )
			{
				dir = trim( dir.substr( 7 ) ); // Cut away "include" & trim

				bool isRelative = false;

				if ( dir[0] == '"' && dir.back() == '"' )
					isRelative = true;
				else if ( dir[0] == '<' && dir.back() == '>' )
					isRelative = false;
				else
				{
					log::error( "Invalid include directive at line %d", lineNum );
					result = false;
					return;
				}

				std::filesystem::path path = trim( dir.substr( 1, dir.length() - 2 ) );
				if ( isRelative )
					path = cwd / path;

				std::ifstream ifs( path.c_str(), std::ios_base::in | std::ios_base::binary );
				if ( !ifs.is_open() )
				{
					log::error( "Could not open file stream: %s", path.c_str() );
					result = false;
					return;
				}

				auto bytes = load_all_chars( ifs );
				if ( !unroll_includes( ss, std::string_view( bytes.data(), bytes.size() ), path.parent_path() ) )
					return;
			}
		}

		if ( addLine )
			ss << line << std::endl;
	} );

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
void *get_gl_proc_address( const char *name )
{
#if defined(WIN32)
	return wglGetProcAddress( name );
#else
#error Not implemented!
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void init_gl_api()
{
	static bool s_initialized = false;
	if ( s_initialized )
		return;

	gl::CreateContextAttribsARB = decltype( gl::CreateContextAttribsARB )( get_gl_proc_address( "wglCreateContextAttribsARB" ) );
	s_initialized = true;
}

} // namespace gl3d::detail

//---------------------------------------------------------------------------------------------------------------------
buffer::buffer( gl::enum_t type )
	: _type( type )
{

}

//---------------------------------------------------------------------------------------------------------------------
buffer::~buffer()
{

}

//---------------------------------------------------------------------------------------------------------------------
void buffer::allocate_data( const void *data, size_t size, bool releaseAfterUpload /* = true */ )
{

}

//---------------------------------------------------------------------------------------------------------------------
void buffer::set_data( const void *data, size_t size )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
compiled_shader::compiled_shader()
{

}

//---------------------------------------------------------------------------------------------------------------------
compiled_shader::~compiled_shader()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
bool shader::source( std::string_view sourceCode, const std::filesystem::path &cwd )
{
	std::stringstream ss;
	if ( !detail::unroll_includes( ss, sourceCode, cwd ) )
		return false;

	_unrolledSource = ss.str();
	detail::for_each_line( _unrolledSource, [&]( std::string_view line, unsigned lineNum )
	{
		bool addLine = true;

		if ( auto dir = detail::trim( line ); !dir.empty() && dir[0] == '#' )
		{
			addLine = false;
			dir = detail::trim( dir.substr( 1 ) ); // Cut away '#' & trim
			if ( detail::starts_with_nocase( dir, "vertex", "vert", "vs" ) )
			{

			}
			else if ( detail::starts_with_nocase( dir, "fragment", "frag", "fs", "pixel" ) )
			{

			}
		}

		if ( addLine )
			printf( "%s\n", std::string( line ).c_str() );
	} );

	_path.clear();
	_source = sourceCode;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool shader::load( std::istream &is, const std::filesystem::path &cwd )
{
	auto bytes = detail::load_all_chars( is );
	return source( std::string_view( bytes.data(), bytes.size() ), cwd );
}

//---------------------------------------------------------------------------------------------------------------------
bool shader::load( const std::filesystem::path &path )
{
	std::ifstream ifs( path.c_str(), std::ios_base::in | std::ios_base::binary );
	if ( !ifs.is_open() )
		return false;

	if ( !load( ifs, path.parent_path() ) )
		return false;

	_path = path;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
compiled_shader::ptr shader::compile()
{
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
cmd_queue::cmd_queue( bool record )
	: _recording( record )
{

}

//---------------------------------------------------------------------------------------------------------------------
cmd_queue::~cmd_queue()
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::reset()
{
	_position = 0;
	_resources.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::clear_color( const vec4 &color )
{
	if ( _recording )
		write( cmd_type::clear_color, color );
	else
	{
		glClearColor( color.x, color.y, color.z, color.w );
		glClear( GL_COLOR_BUFFER_BIT );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::clear_depth( float depth )
{
	if ( _recording )
		write( cmd_type::clear_depth, depth );
	else
	{
		glClearDepth( depth );
		glClear( GL_DEPTH_BUFFER_BIT );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_state( const blend_state &bs )
{
	if ( _recording )
		write( cmd_type::bind_blend_state, bs );
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_state( const depth_stencil_state &ds )
{
	if ( _recording )
		write( cmd_type::bind_depth_stencil_state, ds );
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_state( const rasterizer_state &rs )
{
	if ( _recording )
		write( cmd_type::bind_rasterizer_state, rs );
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_shader( compiled_shader::ptr sh )
{
	if ( _recording )
	{
		write( cmd_type::bind_shader );
		_resources.push_back( sh );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_vertex_buffer( buffer::ptr vb, const detail::layout &layout, size_t offset )
{
	if ( _recording )
	{
		write( cmd_type::bind_vertex_buffer, &layout, offset );
		_resources.push_back( vb );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_index_buffer( buffer::ptr ib, bool use16bits, size_t offset )
{
	if ( _recording )
	{
		write( cmd_type::bind_index_buffer, use16bits, offset );
		_resources.push_back( ib );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::uniform_block( location_variant_t location, const void *data, size_t size )
{
	assert( data && size );

	if ( _recording )
	{

	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::draw( gl::enum_t primitive, size_t first, size_t count, size_t instanceCount, size_t instanceBase )
{
	if ( _recording )
	{
		write( cmd_type::draw, primitive, first, count, instanceCount, instanceBase );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::draw_indexed( gl::enum_t primitive, size_t first, size_t count, size_t instanceCount, size_t instanceBase )
{
	if ( _recording )
	{
		write( cmd_type::draw_indexed, primitive, first, count, instanceCount, instanceBase );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::execute( ptr cmdQueue )
{
	assert( cmdQueue && cmdQueue.get() != this );

	if ( _recording )
	{
		write( cmd_type::execute );
		_resources.push_back( cmdQueue );
	}
	else
	{
		cmdQueue->execute();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::execute()
{
	_recording = false;
	_position = 0;
	size_t resIndex = 0;

	while ( _position < _recordedData.size() )
	{
		auto cmd = read<cmd_type>();
		switch ( cmd )
		{
			default:
			{ assert( 0 ); break; }

			case cmd_type::clear_color:
			{
				auto color = read<vec4>();
				clear_color( color );
			}
			break;

			case cmd_type::clear_depth:
			{
				auto depth = read<float>();
				clear_depth( depth );
			}
			break;

			case cmd_type::bind_blend_state:
				bind_state( read<blend_state>() );
				break;

			case cmd_type::bind_depth_stencil_state:
				bind_state( read<depth_stencil_state>() );
				break;

			case cmd_type::bind_rasterizer_state:
				bind_state( read<rasterizer_state>() );
				break;

			case cmd_type::bind_vertex_buffer:
			{
				auto vb = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				const auto &layout = read<detail::layout>();
				auto offset = read<size_t>();
				bind_vertex_buffer( vb, layout, offset );
			}
			break;

			case cmd_type::bind_index_buffer:
			{
				auto ib = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				auto use16bits = read<bool>();
				auto offset = read<size_t>();
				bind_index_buffer( ib, use16bits, offset );
			}
			break;

			case cmd_type::draw:
			case cmd_type::draw_indexed:
			{
				auto primitive = read<gl::enum_t>();
				auto first = read<size_t>();
				auto count = read<size_t>();
				auto instanceCount = read<size_t>();
				auto instanceBase = read<size_t>();

				if ( cmd == cmd_type::draw )
					draw( primitive, first, count, instanceCount, instanceBase );
				else
					draw_indexed( primitive, first, count, instanceCount, instanceBase );
			}
			break;

			case cmd_type::execute:
			{
				auto cmdQueue = std::static_pointer_cast<cmd_queue>( _resources[resIndex++] );
				execute( cmdQueue );
			}
			break;
		}
	}

	_recording = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

#if defined(WIN32)
unsigned g_contextAttribs[] =
{
	gl::CONTEXT_MAJOR_VERSION_ARB, 4,
	gl::CONTEXT_MINOR_VERSION_ARB, 5,
	gl::CONTEXT_PROFILE_MASK_ARB, gl::CONTEXT_CORE_PROFILE_BIT_ARB,
	0
};

//---------------------------------------------------------------------------------------------------------------------
context::context( void *windowNativeHandle )
	: cmd_queue( false )
	, _window_native_handle( windowNativeHandle )
{
	static HMODULE s_renderDoc = LoadLibraryA( "renderdoc.dll" );

	auto hdc = GetDC( HWND( _window_native_handle ) );
	auto tempContext = wglCreateContext( hdc );
	wglMakeCurrent( hdc, tempContext );

	detail::init_gl_api();

	_native_handle = gl::CreateContextAttribsARB(
	                     hdc,
	                     nullptr,
	                     reinterpret_cast<const int *>( g_contextAttribs ) );

	wglMakeCurrent( nullptr, nullptr );
	wglDeleteContext( tempContext );
}

//---------------------------------------------------------------------------------------------------------------------
context::context( ptr sharedContext )
	: cmd_queue( false )
	, _window_native_handle( sharedContext->_window_native_handle )
{
	sharedContext->make_current();

	_window_native_handle = sharedContext->_window_native_handle;
	auto hdc = GetDC( HWND( _window_native_handle ) );

	_native_handle = gl::CreateContextAttribsARB(
	                     hdc,
	                     HGLRC( sharedContext->_native_handle ),
	                     reinterpret_cast<const int *>( g_contextAttribs ) );

	wglMakeCurrent( nullptr, nullptr );
}

//---------------------------------------------------------------------------------------------------------------------
context::~context()
{
	wglDeleteContext( HGLRC( _native_handle ) );
}

//---------------------------------------------------------------------------------------------------------------------
void context::make_current()
{
	if ( wglGetCurrentContext() != HGLRC( _native_handle ) )
		wglMakeCurrent( GetDC( HWND( _window_native_handle ) ), HGLRC( _native_handle ) );
}
#else
#error Not implemented!
#endif

} // namespace gl3d::detail

} // namespace gl3d
