#ifndef __GL3D_H_IMPL__
	#define __GL3D_H_IMPL__
#endif

#include "gl3d.h"

#ifdef _MSC_VER
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
context::ptr context::from_native_handle( void *nativeHandle )
{
	ptr result = std::make_shared<context>();
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
context::ptr context::from_shared_context( const context &parent )
{
	ptr result = std::make_shared<context>();
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
void context::make_current()
{
	/*
	if ( wglGetCurrentContext() != hglrc )
		wglMakeCurrent( hdc, hglrc );
	*/
}

} // namespace gl3d::detail

} // namespace gl3d
