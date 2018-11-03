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

decltype( gl::GenBuffers ) gl::GenBuffers;
decltype( gl::DeleteBuffers ) gl::DeleteBuffers;
decltype( gl::BindBuffer ) gl::BindBuffer;
decltype( gl::BufferData ) gl::BufferData;
decltype( gl::GenVertexArrays ) gl::GenVertexArrays;
decltype( gl::BindVertexArray ) gl::BindVertexArray;
decltype( gl::EnableVertexAttribArray ) gl::EnableVertexAttribArray;
decltype( gl::VertexAttribPointer ) gl::VertexAttribPointer;
decltype( gl::BindAttribLocation ) gl::BindAttribLocation;
decltype( gl::DeleteVertexArrays ) gl::DeleteVertexArrays;
decltype( gl::CreateShader ) gl::CreateShader;
decltype( gl::DeleteShader ) gl::DeleteShader;
decltype( gl::ShaderSource ) gl::ShaderSource;
decltype( gl::CompileShader ) gl::CompileShader;
decltype( gl::GetShaderiv ) gl::GetShaderiv;
decltype( gl::GetShaderInfoLog ) gl::GetShaderInfoLog;
decltype( gl::CreateProgram ) gl::CreateProgram;
decltype( gl::DeleteProgram ) gl::DeleteProgram;
decltype( gl::AttachShader ) gl::AttachShader;
decltype( gl::DetachShader ) gl::DetachShader;
decltype( gl::LinkProgram ) gl::LinkProgram;
decltype( gl::UseProgram ) gl::UseProgram;
decltype( gl::GetProgramiv ) gl::GetProgramiv;
decltype( gl::GetUniformLocation ) gl::GetUniformLocation;
decltype( gl::Uniform1i ) gl::Uniform1i;
decltype( gl::Uniform2fv ) gl::Uniform2fv;
decltype( gl::UniformMatrix4fv ) gl::UniformMatrix4fv;
decltype( gl::ActiveTexture ) gl::ActiveTexture;
decltype( gl::Enablei ) gl::Enablei;
decltype( gl::Disablei ) gl::Disablei;
decltype( gl::BlendFunci ) gl::BlendFunci;
decltype( gl::BlendEquationi ) gl::BlendEquationi;

#if defined(WIN32)
	decltype( gl::CreateContextAttribsARB ) gl::CreateContextAttribsARB;
#endif

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

	gl::GenBuffers = decltype( gl::GenBuffers )( get_gl_proc_address( "glGenBuffers" ) );
	gl::DeleteBuffers = decltype( gl::DeleteBuffers )( get_gl_proc_address( "glDeleteBuffers" ) );
	gl::BindBuffer = decltype( gl::BindBuffer )( get_gl_proc_address( "glBindBuffer" ) );
	gl::BufferData = decltype( gl::BufferData )( get_gl_proc_address( "glBufferData" ) );
	gl::GenVertexArrays = decltype( gl::GenVertexArrays )( get_gl_proc_address( "glGenVertexArrays" ) );
	gl::BindVertexArray = decltype( gl::BindVertexArray )( get_gl_proc_address( "glBindVertexArray" ) );
	gl::EnableVertexAttribArray = decltype( gl::EnableVertexAttribArray )( get_gl_proc_address( "glEnableVertexAttribArray" ) );
	gl::VertexAttribPointer = decltype( gl::VertexAttribPointer )( get_gl_proc_address( "glVertexAttribPointer" ) );
	gl::BindAttribLocation = decltype( gl::BindAttribLocation )( get_gl_proc_address( "glBindAttribLocation" ) );
	gl::DeleteVertexArrays = decltype( gl::DeleteVertexArrays )( get_gl_proc_address( "glDeleteVertexArrays" ) );
	gl::CreateShader = decltype( gl::CreateShader )( get_gl_proc_address( "glCreateShader" ) );
	gl::DeleteShader = decltype( gl::DeleteShader )( get_gl_proc_address( "glDeleteShader" ) );
	gl::ShaderSource = decltype( gl::ShaderSource )( get_gl_proc_address( "glShaderSource" ) );
	gl::CompileShader = decltype( gl::CompileShader )( get_gl_proc_address( "glCompileShader" ) );
	gl::GetShaderiv = decltype( gl::GetShaderiv )( get_gl_proc_address( "glGetShaderiv" ) );
	gl::GetShaderInfoLog = decltype( gl::GetShaderInfoLog )( get_gl_proc_address( "glGetShaderInfoLog" ) );
	gl::CreateProgram = decltype( gl::CreateProgram )( get_gl_proc_address( "glCreateProgram" ) );
	gl::DeleteProgram = decltype( gl::DeleteProgram )( get_gl_proc_address( "glDeleteProgram" ) );
	gl::AttachShader = decltype( gl::AttachShader )( get_gl_proc_address( "glAttachShader" ) );
	gl::DetachShader = decltype( gl::DetachShader )( get_gl_proc_address( "glDetachShader" ) );
	gl::LinkProgram = decltype( gl::LinkProgram )( get_gl_proc_address( "glLinkProgram" ) );
	gl::UseProgram = decltype( gl::UseProgram )( get_gl_proc_address( "glUseProgram" ) );
	gl::GetProgramiv = decltype( gl::GetProgramiv )( get_gl_proc_address( "glGetProgramiv" ) );
	gl::GetUniformLocation = decltype( gl::GetUniformLocation )( get_gl_proc_address( "glGetUniformLocation" ) );
	gl::Uniform1i = decltype( gl::Uniform1i )( get_gl_proc_address( "glUniform1i" ) );
	gl::Uniform2fv = decltype( gl::Uniform2fv )( get_gl_proc_address( "glUniform2fv" ) );
	gl::UniformMatrix4fv = decltype( gl::UniformMatrix4fv )( get_gl_proc_address( "glUniformMatrix4fv" ) );
	gl::ActiveTexture = decltype( gl::ActiveTexture )( get_gl_proc_address( "glActiveTexture" ) );
	gl::Enablei = decltype( gl::Enablei )( get_gl_proc_address( "glEnablei" ) );
	gl::Disablei = decltype( gl::Disablei )( get_gl_proc_address( "glDisablei" ) );
	gl::BlendFunci = decltype( gl::BlendFunci )( get_gl_proc_address( "glBlendFunci" ) );
	gl::BlendEquationi = decltype( gl::BlendEquationi )( get_gl_proc_address( "glBlendEquationi" ) );


#if defined(WIN32)
	gl::CreateContextAttribsARB = decltype( gl::CreateContextAttribsARB )( get_gl_proc_address( "wglCreateContextAttribsARB" ) );
#endif

	s_initialized = true;
}

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
			dir = trim( line.substr( 1 ) ); // Cut away '#' & trim
			if ( starts_with_nocase( dir, "include" ) )
			{
				addLine = false;
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

				bytes_t bytes;
				if ( !on_data_request.call( path, bytes ) )
				{
					log::error( "Could not open file stream: %s", path.c_str() );
					result = false;
					return;
				}

				if ( !unroll_includes( ss, detail::to_string_view( bytes ), path.parent_path() ) )
					return;
			}
		}

		if ( addLine )
			ss << line << std::endl;
	} );

	return result;
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
shader::shader( std::shared_ptr<shader_code> code, std::string_view defines )
	: _shaderCode( code )
	, _defines( defines )
{

}

//---------------------------------------------------------------------------------------------------------------------
shader::~shader()
{
	clear();
}

//---------------------------------------------------------------------------------------------------------------------
void shader::clear()
{
	for ( auto &stageID : _stageIDs )
	{
		gl::DeleteShader( stageID );
		stageID = 0;
	}

	gl::DeleteProgram( _id );
	_id = 0;
}

//---------------------------------------------------------------------------------------------------------------------
bool shader::compile()
{
	for ( size_t i = 0; i < +shader_stage::__count; ++i )
	{
		auto &src = _shaderCode->stage_source( static_cast<shader_stage>( i ) );
		if ( src.empty() )
		{
			if ( _stageIDs[i] )
			{
				gl::DeleteShader( _stageIDs[i] );
				_stageIDs[i] = 0;
			}

			continue;
		}

		if ( !_stageIDs[i] )
		{
			static constexpr gl::enum_t glTypes[] =
			{ gl::VERTEX_SHADER, gl::GEOMETRY_SHADER, gl::FRAGMENT_SHADER, gl::COMPUTE_SHADER };

			_stageIDs[i] = gl::CreateShader( glTypes[i] );
		}

		auto srcData = src.c_str();
		gl::ShaderSource( _stageIDs[i], 1, &srcData, nullptr );
		gl::CompileShader( _stageIDs[i] );

		int status;
		gl::GetShaderiv( _stageIDs[i], gl::COMPILE_STATUS, &status );
		if ( !status )
		{
			int logLength;
			gl::GetShaderiv( _stageIDs[i], gl::INFO_LOG_LENGTH, &logLength );

			std::unique_ptr<char[]> text = std::make_unique<char[]>( logLength + 1 );
			gl::GetShaderInfoLog( _stageIDs[i], logLength, nullptr, text.get() );
			text[logLength] = 0;

			log::error( "%s", text.get() );
			return false;
		}
	}

	if ( !_id )
		_id = gl::CreateProgram();

	for ( auto stageID : _stageIDs ) if ( stageID ) gl::AttachShader( _id, stageID );
	gl::LinkProgram( _id );
	for ( auto stageID : _stageIDs ) if ( stageID ) gl::DetachShader( _id, stageID );

	int status;
	gl::GetProgramiv( _id, gl::LINK_STATUS, &status );
	if ( !status )
	{
		clear();
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
bool shader_code::source( std::string_view sourceCode, const std::filesystem::path &cwd )
{
	std::stringstream ss;
	if ( !detail::unroll_includes( ss, sourceCode, cwd ) )
		return false;

	for ( auto &stageSource : _stageSources )
		stageSource.clear();

	std::string sharedSource = "#version 420\n";
	std::string *currentStage = &sharedSource;

	_unrolledSource = ss.str();
	detail::for_each_line( _unrolledSource, [&]( std::string_view line, unsigned lineNum )
	{
		bool addLine = true;

		if ( auto dir = detail::trim( line ); !dir.empty() && dir[0] == '#' )
		{
			dir = detail::trim( dir.substr( 1 ) ); // Cut away '#' & trim
			if ( detail::starts_with_nocase( dir, "vertex", "vert", "vs" ) )
			{
				addLine = false;
				currentStage = &_stageSources[+shader_stage::vertex];
			}
			else if ( detail::starts_with_nocase( dir, "fragment", "frag", "fs", "pixel" ) )
			{
				addLine = false;
				currentStage = &_stageSources[+shader_stage::fragment];
			}
		}

		if ( addLine )
			*currentStage += line;
	} );

	for ( auto &stageSource : _stageSources )
		if ( !stageSource.empty() )
			stageSource = ( sharedSource + stageSource );

	_path.clear();
	_source = sourceCode;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool shader_code::load( std::istream &is, const std::filesystem::path &cwd )
{
	detail::bytes_t bytes;
	if ( !detail::read_all_bytes( is, bytes, true ) )
		return false;

	return source( detail::to_string_view( bytes ), cwd );
}

//---------------------------------------------------------------------------------------------------------------------
bool shader_code::load( const std::filesystem::path &path )
{
	detail::bytes_t bytes;
	if ( !on_data_request.call( path, bytes ) )
		return false;

	if ( !source( detail::to_string_view( bytes ), path.parent_path() ) )
		return false;

	_path = path;
	return true;
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
void cmd_queue::bind_shader( shader::ptr sh )
{
	if ( _recording )
	{
		write( cmd_type::bind_shader );
		_resources.push_back( sh );
	}
	else
	{
		if ( sh )
		{
			if ( !sh->id() )
				sh->compile();
		}
		else
		{

		}
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

			case cmd_type::bind_shader:
			{
				auto sh = std::static_pointer_cast<shader>( _resources[resIndex++] );
				bind_shader( sh );
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
	gl::CONTEXT_MINOR_VERSION_ARB, 6,
	gl::CONTEXT_PROFILE_MASK_ARB, gl::CONTEXT_CORE_PROFILE_BIT_ARB,
	0
};

//---------------------------------------------------------------------------------------------------------------------
context::context( void *windowNativeHandle )
	: cmd_queue( false )
	, _window_native_handle( windowNativeHandle )
{
	//static HMODULE s_renderDoc = LoadLibraryA( "renderdoc.dll" );

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
