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
context::ptr context::create( void *windowNativeHandle )
{
	ptr result = std::make_shared<context>();

	auto hdc = GetDC( HWND( windowNativeHandle ) );
	auto tempContext = wglCreateContext( hdc );
	wglMakeCurrent( hdc, tempContext );

	detail::init_gl_api();

	unsigned contextAttribs[] =
	{
		gl::CONTEXT_MAJOR_VERSION_ARB, 4,
		gl::CONTEXT_MINOR_VERSION_ARB, 6,
		gl::CONTEXT_PROFILE_MASK_ARB, gl::CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	result->_window_native_handle = windowNativeHandle;
	result->_native_handle = gl::CreateContextAttribsARB( hdc, nullptr, reinterpret_cast<const int *>( contextAttribs ) );
	if ( !result->_native_handle )
		return nullptr;

	wglMakeCurrent( nullptr, nullptr );
	wglDeleteContext( tempContext );
	return result;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
cmd_list::cmd_list()
{

}

//---------------------------------------------------------------------------------------------------------------------
cmd_list::~cmd_list()
{

}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::begin()
{
	_position = 0;
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::end()
{
	if ( _recording )
		write( cmd_type::end );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::bind_shader( shader::ptr sh )
{
	if ( _recording )
	{
		write( cmd_type::bind_shader );
		_renderObjects.push_back( sh );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::bind_vertex_buffer( buffer::ptr vb )
{
	if ( _recording )
	{
		write( cmd_type::bind_vertex_buffer );
		_renderObjects.push_back( vb );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::bind_index_buffer( buffer::ptr ib )
{
	if ( _recording )
	{
		write( cmd_type::bind_index_buffer );
		_renderObjects.push_back( ib );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_list::draw( gl::enum_t primitive, unsigned first, unsigned count, unsigned instanceCount, unsigned instanceBase )
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
void cmd_list::draw_indexed( gl::enum_t primitive, unsigned first, unsigned count, unsigned instanceCount, unsigned instanceBase )
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
void cmd_list::execute()
{
	_recording = false;
	_position = 0;
	size_t objIndex = 0;

	while ( _position < _recordedData.size() )
	{
		auto cmd = read<cmd_type>();
		switch ( cmd )
		{
			default:
			{ assert( 0 ); break; }

			case cmd_type::end:
				break;

			case cmd_type::bind_vertex_buffer:
			{
				auto vb = std::static_pointer_cast<buffer>( _renderObjects[objIndex++] );
				bind_vertex_buffer( vb );
			}
			break;

			case cmd_type::bind_index_buffer:
			{
				auto ib = std::static_pointer_cast<buffer>( _renderObjects[objIndex++] );
				bind_index_buffer( ib );
			}
			break;

			case cmd_type::draw:
			case cmd_type::draw_indexed:
			{
				auto primitive = read<gl::enum_t>();
				auto first = read<unsigned>();
				auto count = read<unsigned>();
				auto instanceCount = read<unsigned>();
				auto instanceBase = read<unsigned>();

				if ( cmd == cmd_type::draw )
					draw( primitive, first, count, instanceCount, instanceBase );
				else
					draw_indexed( primitive, first, count, instanceCount, instanceBase );
			}
			break;
		}
	}

	_recording = true;
}

} // namespace gl3d
