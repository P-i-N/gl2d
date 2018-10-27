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

#if !defined(GL3D_APIENTRY)
	#if defined(WIN32)
		#define GL3D_APIENTRY __stdcall
	#else
		#define GL3D_APIENTRY
	#endif
#endif

#include <cassert>

namespace gl3d {

decltype( gl::CreateContextAttribsARB ) gl::CreateContextAttribsARB;

namespace detail {

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
void cmd_queue::bind_shader( shader::ptr sh )
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

} // namespace gl3d::detail

} // namespace gl3d
