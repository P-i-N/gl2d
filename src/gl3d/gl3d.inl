#ifndef __GL3D_H_IMPL__
	#define __GL3D_H_IMPL__
#endif

#include "gl3d.h"

#if defined(_MSC_VER)
	#pragma comment(lib, "opengl32.lib")
#endif

#if defined(WIN32)
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

static constexpr char *s_lineSeparator = "\n";

//---------------------------------------------------------------------------------------------------------------------
std::string_view trim( std::string_view text )
{
	if ( text.empty() ) return text;

	size_t start = 0, end = text.length() - 1;
	while ( start < text.length() && isspace( text[start] ) ) ++start;
	while ( end > start && isspace( text[end] ) ) --end;

	return text.substr( start, end - start + 1 );
}

//---------------------------------------------------------------------------------------------------------------------
bool starts_with_nocase( std::string_view text, std::string_view start )
{
	if ( start.empty() ) return true;
	if ( start.length() > text.length() ) return false;

	for ( size_t i = 0, S = start.length(); i < S; ++i )
		if ( tolower( start[i] ) != tolower( text[i] ) )
			return false;

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool unroll_includes( std::stringstream &ss, std::string_view sourceCode, const std::filesystem::path &cwd )
{
	size_t cursor = 0;
	while ( cursor <= sourceCode.length() )
	{
		bool addLine = true;

		auto sepPos = sourceCode.find( detail::s_lineSeparator, cursor );
		if ( sepPos == std::string::npos )
			sepPos = sourceCode.length();

		auto line = trim( sourceCode.substr( cursor, sepPos - cursor ) );
		if ( !line.empty() && line[0] == '#' )
		{
			addLine = false;
			line = trim( line.substr( 1 ) ); // Cut away '#' & trim
			if ( starts_with_nocase( line, "include" ) )
			{
				line = trim( line.substr( 7 ) ); // Cut away "include" & trim

				bool isRelative = false;

				if ( line[0] == '"' && line.back() == '"' )
					isRelative = true;
				else if ( line[0] == '<' && line.back() == '>' )
					isRelative = false;
				else
				{

				}

				std::filesystem::path path = trim( line.substr( 1, line.length() - 2 ) );
				if ( isRelative )
					path = cwd / path;

				std::ifstream ifs( path.c_str(), std::ios_base::in | std::ios_base::binary );
				if ( !ifs.is_open() )
					return false;

				ifs.seekg( 0, std::ios_base::end );
				size_t size = ifs.tellg();
				ifs.seekg( 0, std::ios_base::beg );

				std::unique_ptr<char[]> bytes( new char[size + 1] );
				ifs.read( bytes.get(), size );
				bytes[size] = 0;

				if ( !unroll_includes( ss, std::string_view( bytes.get(), size ), path.parent_path() ) )
					return false;
			}
		}

		cursor = sepPos + strlen( detail::s_lineSeparator );

		if ( addLine )
			ss << line << std::endl;
	}

	return true;
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

	std::string unrolledSource = ss.str();
	std::string_view unrolledView = unrolledSource;

	size_t cursor = 0;
	while ( cursor <= unrolledView.length() )
	{
		bool addLine = true;

		auto sepPos = unrolledView.find( detail::s_lineSeparator, cursor );
		if ( sepPos == std::string::npos )
			sepPos = unrolledView.length();

		auto line = detail::trim( unrolledView.substr( cursor, sepPos - cursor ) );
		if ( !line.empty() && line[0] == '#' )
		{
			addLine = false;
			line = detail::trim( line.substr( 1 ) ); // Cut away '#' & trim
			if ( detail::starts_with_nocase( line, "vertex" ) || detail::starts_with_nocase( line, "vert" ) )
			{

			}
			else if ( detail::starts_with_nocase( line, "fragment" ) || detail::starts_with_nocase( line, "frag" ) )
			{

			}
		}

		cursor = sepPos + strlen( detail::s_lineSeparator );

		if ( addLine )
			printf( "%s\n", std::string( line ).c_str() );
	}

	_path.clear();
	_source = sourceCode;
	_unrolledSource = unrolledSource;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool shader::load( std::istream &is, const std::filesystem::path &cwd )
{
	is.seekg( 0, std::ios_base::end );
	size_t size = is.tellg();
	is.seekg( 0, std::ios_base::beg );

	std::unique_ptr<char[]> bytes( new char[size + 1] );
	is.read( bytes.get(), size );
	bytes[size] = 0;

	return source( std::string_view( bytes.get(), size ), cwd );
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
	{
		write( cmd_type::clear_depth, depth );
		return;
	}
	else
	{
		glClearDepth( depth );
		glClear( GL_DEPTH_BUFFER_BIT );
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
