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

} // namespace gl3d
