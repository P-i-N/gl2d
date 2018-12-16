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

namespace gl3d {

detail::gl_api gl;

namespace detail {

thread_local context *tl_currentContext = nullptr;

//---------------------------------------------------------------------------------------------------------------------
int find_uniform_id( const detail::location_variant &location )
{
	if ( !location.holds_name() )
		return location.id();

	int programID;
	glGetIntegerv( +gl_enum::CURRENT_PROGRAM, &programID );
	return gl.GetUniformLocation( programID, location.data );
}

//---------------------------------------------------------------------------------------------------------------------
struct internal_format
{
	gl_enum components = gl_enum::NONE;
	gl_enum type = gl_enum::NONE;
	unsigned pixel_size = 0;
};

//---------------------------------------------------------------------------------------------------------------------
internal_format get_internal_format( gl_format format )
{
	static std::unordered_map<gl_format, internal_format> s_internalFormatMap =
	{
		{ gl_format::R8, { gl_enum::RED, gl_enum::UNSIGNED_BYTE, 1 } },
		{ gl_format::RGB8, { gl_enum::RGB, gl_enum::UNSIGNED_BYTE, 3 } },
		{ gl_format::RGBA8, { gl_enum::RGBA, gl_enum::UNSIGNED_BYTE, 4 } }
	};

	if ( auto iter = s_internalFormatMap.find( format ); iter != s_internalFormatMap.end() )
		return iter->second;

	assert( 0 );
	return internal_format();
}

//---------------------------------------------------------------------------------------------------------------------
unsigned mip_level_count( const uvec3 &size )
{
	return 1 + static_cast<unsigned>( floor( log2( static_cast<float>( maximum( size.x, size.y, size.z ) ) ) ) );
}

} // namespace gl3d::detail

//---------------------------------------------------------------------------------------------------------------------
void check_gl_error()
{
#if defined(_DEBUG)
	GLenum err = glGetError();
	if ( err != GL_NO_ERROR )
	{
		const char *errName = "(unknown error)";

		switch ( err )
		{
			case GL_INVALID_ENUM:
				errName = "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				errName = "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				errName = "GL_INVALID_OPERATION";
				break;
		}

		log::error( "OpenGL error (%d): %s", err, errName );
		assert( 0 );
	}
#endif
}

//---------------------------------------------------------------------------------------------------------------------
buffer::buffer( buffer_usage usage, const void *data, size_t size, bool makeCopy )
	: _usage( usage )
	, _size( size )
	, _owner( ( data != nullptr ) && makeCopy )
{
	if ( _owner )
	{
		_data = new uint8_t[_size];
		memcpy( _data, data, _size );
	}
	else
		_data = static_cast<uint8_t *>( const_cast<void *>( data ) );
}

//---------------------------------------------------------------------------------------------------------------------
buffer::~buffer()
{
	if ( _owner )
		delete[] _data;
}

//---------------------------------------------------------------------------------------------------------------------
void buffer::synchronize()
{
	if ( !_id )
	{
		gl.CreateBuffers( 1, &_id );
		if ( !_size ) return;

		auto flags = +gl_enum::DYNAMIC_STORAGE_BIT | +gl_enum::MAP_WRITE_BIT;

		switch ( _usage )
		{
			case buffer_usage::immutable:
			case buffer_usage::dynamic:
			case buffer_usage::persistent:
			case buffer_usage::persistent_coherent:
			{
				if ( _usage == buffer_usage::immutable )
					flags &= ~( +gl_enum::DYNAMIC_STORAGE_BIT );
				else if ( _usage == buffer_usage::persistent )
					flags |= +gl_enum::MAP_PERSISTENT_BIT;
				else if ( _usage == buffer_usage::persistent_coherent )
					flags |= +gl_enum::MAP_PERSISTENT_BIT | +gl_enum::MAP_COHERENT_BIT;

				gl.NamedBufferStorage( _id, static_cast<int>( _size ), _data, flags );
			}
			break;

			case buffer_usage::dynamic_resizable:
				gl.NamedBufferData( _id, static_cast<int>( _size ), _data, gl_enum::DYNAMIC_DRAW );
				break;
		}

		if ( _owner )
		{
			delete[] _data;
			_data = nullptr;
			_owner = false;
		}

		if ( _usage == buffer_usage::persistent || _usage == buffer_usage::persistent_coherent )
			_data = reinterpret_cast<uint8_t *>( gl.MapNamedBufferRange( _id, 0, static_cast<unsigned>( _size ), flags ) );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void *buffer::map( unsigned accessFlags ) const
{
	if ( _usage == buffer_usage::persistent || _usage == buffer_usage::persistent_coherent )
		return _data;

	return gl.MapNamedBuffer( _id, accessFlags );
}

//---------------------------------------------------------------------------------------------------------------------
void *buffer::map( size_t offset, size_t length, unsigned accessFlags ) const
{
	assert( _id && offset + length <= size() );

	if ( _usage == buffer_usage::persistent || _usage == buffer_usage::persistent_coherent )
		return _data + offset;

	return gl.MapNamedBufferRange( _id, static_cast<unsigned>( offset ), static_cast<unsigned>( length ), accessFlags );
}

//---------------------------------------------------------------------------------------------------------------------
void buffer::unmap() const
{
	assert( _id );

	if ( _usage == buffer_usage::persistent || _usage == buffer_usage::persistent_coherent )
		return;

	gl.UnmapNamedBuffer( _id );
}

//---------------------------------------------------------------------------------------------------------------------
void buffer::resize( const void *data, size_t length )
{
	assert( _id &&  _usage == buffer_usage::dynamic_resizable );

	gl.NamedBufferData( _id, static_cast<int>( length ), data, gl_enum::DYNAMIC_DRAW );
	_size = length;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
shader::shader( shader_code::ptr code, std::string_view defines )
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
		gl.DeleteShader( stageID );
		stageID = 0;
	}

	gl.DeleteProgram( _id );
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
				gl.DeleteShader( _stageIDs[i] );
				_stageIDs[i] = 0;
			}

			continue;
		}

		if ( !_stageIDs[i] )
		{
			static constexpr gl_enum glTypes[] =
			{ gl_enum::VERTEX_SHADER, gl_enum::GEOMETRY_SHADER, gl_enum::FRAGMENT_SHADER, gl_enum::COMPUTE_SHADER };

			_stageIDs[i] = gl.CreateShader( glTypes[i] );
		}

		auto srcData = src.c_str();
		gl.ShaderSource( _stageIDs[i], 1, &srcData, nullptr );
		gl.CompileShader( _stageIDs[i] );

		int compileStatus;
		gl.GetShaderiv( _stageIDs[i], gl_enum::COMPILE_STATUS, &compileStatus );
		if ( !compileStatus )
		{
			int logLength;
			gl.GetShaderiv( _stageIDs[i], gl_enum::INFO_LOG_LENGTH, &logLength );

			std::unique_ptr<char[]> text = std::make_unique<char[]>( logLength + 1 );
			gl.GetShaderInfoLog( _stageIDs[i], logLength, nullptr, text.get() );
			text[logLength] = 0;

			log::error( "%s", text.get() );
			return false;
		}
	}

	if ( !_id )
		_id = gl.CreateProgram();

	for ( auto stageID : _stageIDs ) if ( stageID ) gl.AttachShader( _id, stageID );
	gl.LinkProgram( _id );
	for ( auto stageID : _stageIDs ) if ( stageID ) gl.DetachShader( _id, stageID );

	int linkStatus;
	gl.GetProgramiv( _id, gl_enum::LINK_STATUS, &linkStatus );
	if ( !linkStatus )
	{
		int logLength;
		gl.GetProgramiv( _id, gl_enum::INFO_LOG_LENGTH, &logLength );

		std::unique_ptr<char[]> text = std::make_unique<char[]>( logLength + 1 );
		gl.GetProgramInfoLog( _id, logLength, nullptr, text.get() );
		text[logLength] = 0;

		log::error( "%s", text.get() );
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

	std::string sharedSource =
	    "#version 460 core\n"
	    "#extension GL_ARB_gpu_shader_int64 : enable\n"
	    "#extension GL_ARB_shader_draw_parameters : enable\n";

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
		{
			*currentStage += line;
			*currentStage += detail::s_lineSeparator;
		}
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
	detail::read_all_bytes( is, bytes, true );

	return source( detail::to_string_view( bytes ), cwd );
}

//---------------------------------------------------------------------------------------------------------------------
bool shader_code::load( const std::filesystem::path &path )
{
	detail::bytes_t bytes;
	if ( !vfs::load( path, bytes ) )
		return false;

	if ( !source( detail::to_string_view( bytes ), path.parent_path() ) )
		return false;

	_path = path;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
texture::ptr texture::white_pixel()
{
	static uint32_t whiteRGBA = 0xFFFFFFFFu;
	static auto result = texture::create( gl_format::RGBA8, uvec2{ 1, 1 }, &whiteRGBA );
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
texture::ptr texture::checkerboard()
{
#define B 0xFF000000u
#define W 0xFFFFFFFFu

	static uint32_t checkerboardRGBA[] =
	{
		B, W, B, W, B, W, B, W,
		W, B, W, B, W, B, W, B,
		B, W, B, W, B, W, B, W,
		W, B, W, B, W, B, W, B,
		B, W, B, W, B, W, B, W,
		W, B, W, B, W, B, W, B,
		B, W, B, W, B, W, B, W,
		W, B, W, B, W, B, W, B,
	};

#undef W
#undef B

	static auto result = texture::create( gl_format::RGBA8, uvec2{ 8, 8 }, checkerboardRGBA );
	result->filter_mag( gl_enum::NEAREST );
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
texture::texture( gl_enum type, gl_format format, const uvec3 &dimensions, bool hasMips )
	: _type( type )
	, _format( format )
	, _dimensions( dimensions )
	, _owner( false )
{
	assert( _dimensions.x > 0 && _dimensions.y > 0 && _dimensions.z > 0 );
	switch ( _type )
	{
		case gl_enum::TEXTURE_1D:
			_dimensions.y = 1;
			_dimensions.z = 1;
			break;

		case gl_enum::TEXTURE_2D:
			_dimensions.z = 1;
			break;

		case gl_enum::TEXTURE_CUBE_MAP:
			_dimensions.z = 6;
			break;

		case gl_enum::TEXTURE_CUBE_MAP_ARRAY:
			_dimensions.z = align_up( _dimensions.z, 6u );
			break;
	}
}

//---------------------------------------------------------------------------------------------------------------------
texture::texture(
    gl_enum type, gl_format format, const uvec3 &dimensions,
    const detail::type_range<part> &parts,
    bool buildMips, bool makeCopy )
	: texture( type, format, dimensions, buildMips )
{
	_owner = makeCopy;
	auto internalF = detail::get_internal_format( format );

	if ( _numParts = static_cast<unsigned>( parts.size ) )
	{
		_parts = std::make_unique<part[]>( _numParts );

		for ( size_t i = 0; i < parts.size; ++i )
		{
			auto &p = _parts[i];
			p = parts.data[i];

			uvec2 size{ maximum( 1, _dimensions.x >> p.mip_level ), maximum( 1, _dimensions.y >> p.mip_level ) };
			auto rowStride = size.x * internalF.pixel_size;

			if ( makeCopy )
			{
				auto *copy = new uint8_t[rowStride * size.y];
				memcpy( copy, p.data, rowStride * size.y );
				p.data = copy;
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
texture::~texture()
{
	clear();
}

//---------------------------------------------------------------------------------------------------------------------
void texture::clear()
{
	if ( _owner && _parts && _numParts )
	{
		for ( size_t i = 0; i < _numParts; ++i )
			delete[] _parts[i].data;
	}

	_parts.reset();
	_numParts = 0;
	_owner = false;
}

//---------------------------------------------------------------------------------------------------------------------
void texture::wrap( gl_enum u, gl_enum v, gl_enum w )
{
	if ( _wrap[0] == u && _wrap[1] == v && _wrap[2] == w )
		return;

	_wrap[0] = u;
	_wrap[1] = v;
	_wrap[2] = w;
	_dirtySampler = true;
}

//---------------------------------------------------------------------------------------------------------------------
void texture::filter( gl_enum minFilter, gl_enum magFilter )
{
	if ( _filter[0] == minFilter && _filter[1] == magFilter )
		return;

	_filter[0] = minFilter;
	_filter[1] = magFilter;
	_dirtySampler = true;
}

//---------------------------------------------------------------------------------------------------------------------
uint64_t texture::synchronize()
{
	if ( !_id )
	{
		gl.CreateTextures( _type, 1, &_id );

		auto internalF = detail::get_internal_format( _format );
		unsigned mipLevels = _buildMips
		                     ? detail::mip_level_count( { _dimensions.x, _dimensions.y, _dimensions.z } )
		                     : 1;

		switch ( _type )
		{
			case gl_enum::TEXTURE_1D:
			{

			}
			break;

			case gl_enum::TEXTURE_2D:
			{
				gl.TextureStorage2D( _id, mipLevels, _format, _dimensions.x, _dimensions.y );

				for ( size_t i = 0; i < _numParts; ++i )
				{
					auto &p = _parts[i];
					gl.TextureSubImage2D(
					    _id, p.mip_level,
					    0, 0, width( p.mip_level ), height( p.mip_level ),
					    internalF.components, internalF.type, p.data );
				}
			}
			break;

			case gl_enum::TEXTURE_2D_ARRAY:
			{

			}
			break;

			case gl_enum::TEXTURE_3D:
			{

			}
			break;

			case gl_enum::TEXTURE_CUBE_MAP:
			{

			}
			break;

			case gl_enum::TEXTURE_CUBE_MAP_ARRAY:
			{

			}
			break;

			default:
				assert( 0 );
				break;
		}

		clear();
		_dirtySampler = true;
	}

	if ( _dirtySampler && _id )
	{
		if ( _bindlessHandle )
			gl.MakeTextureHandleNonResidentARB( _bindlessHandle );

		gl.TextureParameteri( _id, gl_enum::TEXTURE_WRAP_S, static_cast<int>( _wrap[0] ) );
		gl.TextureParameteri( _id, gl_enum::TEXTURE_WRAP_T, static_cast<int>( _wrap[1] ) );
		gl.TextureParameteri( _id, gl_enum::TEXTURE_WRAP_R, static_cast<int>( _wrap[2] ) );
		gl.TextureParameteri( _id, gl_enum::TEXTURE_MIN_FILTER, static_cast<int>( _filter[0] ) );
		gl.TextureParameteri( _id, gl_enum::TEXTURE_MAG_FILTER, static_cast<int>( _filter[1] ) );

		_bindlessHandle = gl.GetTextureHandleARB( _id );
		gl.MakeTextureHandleResidentARB( _bindlessHandle );

		_dirtySampler = false;
	}

	return _bindlessHandle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::gl_state::reset()
{
	current_vb = nullptr;
	current_vb_layout = nullptr;
	current_ib = nullptr;
	dirty_input_assembly = true;
}

//---------------------------------------------------------------------------------------------------------------------
size_t cmd_queue::gl_state::write_temp_data( const void *data, size_t size )
{
	if ( !temp_buffer )
	{
		temp_buffer = buffer::create( buffer_usage::persistent_coherent, nullptr, 1024 * 1024 );
		temp_buffer->synchronize();
		mapped_temp_buffer = static_cast<uint8_t *>( temp_buffer->map() );

		glGetIntegerv( +gl_enum::UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_block_alignment );
	}

	auto offset = temp_buffer_cursor;
	memcpy( mapped_temp_buffer + offset, data, size );

	temp_buffer_cursor = align_up( temp_buffer_cursor + size, static_cast<size_t>( uniform_block_alignment ) );
	if ( temp_buffer_cursor >= temp_buffer->size() )
		temp_buffer_cursor = 0;

	return offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
cmd_queue::cmd_queue( gl_state *state )
	: _deferred( state == nullptr )
	, _state( state )
{
	if ( _deferred )
	{
		_recordedData.reserve( 65536 );
		_resources.reserve( 64 );
	}
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

	if ( _state )
	{
		_state->reset();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::clear_color( const vec4 &color )
{
	if ( _deferred )
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
	if ( _deferred )
		write( cmd_type::clear_depth, depth );
	else
	{
		glClearDepth( depth );
		glClear( GL_DEPTH_BUFFER_BIT );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::update_texture( texture::ptr tex, const void *data, unsigned layer, unsigned mipLevel, size_t rowStride )
{
	assert( tex );

	if ( _deferred )
	{
		write( cmd_type::update_texture, layer, mipLevel, rowStride );
		_resources.push_back( tex );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::update_buffer( buffer::ptr buff, const void *data, size_t size, size_t offset, bool preserveContent )
{
	assert( buff && buff->usage() != buffer_usage::immutable && ( size + offset ) <= buff->size() );

	if ( _deferred )
	{
		write( cmd_type::update_buffer, offset, preserveContent );
		write_data( data, size );
		_resources.push_back( buff );
	}
	else
	{
		buff->synchronize();

		if ( offset == 0 && ( size == buff->size() ) )
		{
			memcpy( buff->map(), data, size );
			buff->unmap();
		}
		else
		{
			auto accessFlags = +gl_enum::MAP_WRITE_BIT;
			if ( preserveContent ) accessFlags |= +gl_enum::MAP_INVALIDATE_RANGE_BIT;

			memcpy( buff->map( offset, size, accessFlags ), data, size );
			buff->unmap();
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::resize_buffer( buffer::ptr buff, const void *data, size_t size )
{
	assert( buff && buff->usage() == buffer_usage::dynamic_resizable );

	if ( _deferred )
	{
		write( cmd_type::resize_buffer );
		write_data( data, size );
		_resources.push_back( buff );
	}
	else
	{
		buff->synchronize();
		buff->resize( data, size );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_shader( shader::ptr sh )
{
	if ( _deferred )
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

			gl.UseProgram( sh->id() );
		}
		else
			gl.UseProgram( 0 );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_vertex_buffer( buffer::ptr vb, const detail::layout &layout, size_t offset )
{
	if ( _deferred )
	{
		write( cmd_type::bind_vertex_buffer, &layout, offset );
		_resources.push_back( vb );
	}
	else
	{
		if ( vb )
			vb->synchronize();

		if ( _state->current_vb != vb || _state->current_vb_layout != &layout || _state->current_vb_offset != offset )
		{
			_state->current_vb = vb;
			_state->current_vb_layout = &layout;
			_state->current_vb_offset = offset;

			_state->dirty_input_assembly = true;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_vertex_attribute( buffer::ptr attribs, unsigned slot, gl_enum glType, bool perInstance, size_t offset, size_t stride )
{
	if ( _deferred )
	{
		write( cmd_type::bind_vertex_attribute, slot, glType, perInstance, offset, stride );
		_resources.push_back( attribs );
	}
	else
	{
		if ( attribs )
			attribs->synchronize();

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_index_buffer( buffer::ptr ib, bool use16bits, size_t offset )
{
	if ( _deferred )
	{
		write( cmd_type::bind_index_buffer, use16bits, offset );
		_resources.push_back( ib );
	}
	else
	{
		if ( ib )
			ib->synchronize();

		if ( _state->current_ib != ib )
		{
			_state->current_ib = ib;
			_state->dirty_input_assembly = true;
		}

		_state->current_ib_16bits = use16bits;
		_state->current_ib_offset = offset;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_texture( texture::ptr tex, unsigned slot )
{
	if ( _deferred )
	{
		write( cmd_type::bind_texture, slot );
		_resources.push_back( tex );
	}
	else
	{
		if ( tex )
			tex->synchronize();

		gl.BindTextureUnit( slot, tex ? tex->id() : 0 );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_render_target( texture::ptr tex, unsigned slot, unsigned layer, unsigned mipLevel )
{
	if ( _deferred )
	{
		write( cmd_type::bind_render_target, slot, layer, mipLevel );
		_resources.push_back( tex );
	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::bind_storage_buffer( buffer::ptr buff, unsigned slot, size_t offset, size_t length )
{
	if ( _deferred )
	{
		write( cmd_type::bind_storage_buffer, slot, offset, length );
		_resources.push_back( buff );
	}
	else
	{
		if ( buff )
		{
			buff->synchronize();
			if ( length == size_t( -1 ) )
				length = buff->size() - offset;

			assert( length <= buff->size() );

			gl.BindBufferRange(
			    gl_enum::SHADER_STORAGE_BUFFER,
			    slot, buff->id(),
			    static_cast<unsigned>( offset ),
			    static_cast<unsigned>( length ) );
		}
		else
		{

		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform_block( const detail::location_variant &location, const void *data, size_t size )
{
	assert( data && size );

	if ( _deferred )
	{
		assert( size <= 65536 );
		write( cmd_type::set_uniform_block );
		write_location_variant( location );
		write_data( data, size );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
	{
		auto offset = _state->write_temp_data( data, size );
		gl.BindBufferRange( gl_enum::UNIFORM_BUFFER, id, _state->temp_buffer->id(), offset, size );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, bool value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::BOOL, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform1i( id, value ? 1 : 0 );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, int value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::INT, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform1i( id, value );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, float value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform1f( id, value );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const vec2 &value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT_VEC2, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform2fv( id, 1, value.data );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const vec3 &value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT_VEC3, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform3fv( id, 1, value.data );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const vec4 &value )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT_VEC4, value );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform4fv( id, 1, value.data );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const mat3 &value, bool transpose )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT_MAT3, value, transpose );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.UniformMatrix3fv( id, 1, transpose ? 1 : 0, value.data );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const mat4 &value, bool transpose )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::FLOAT_MAT4, value, transpose );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.UniformMatrix4fv( id, 1, transpose ? 1 : 0, value.data );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, texture::ptr tex )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform, gl_enum::TEXTURE );
		_resources.push_back( tex );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
	{
		assert( 0 ); // TODO
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const uint64_t *values, size_t count )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform_array, gl_enum::UNSIGNED_INT64 );
		write_data( values, sizeof( decltype( *values ) ) * count );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.UniformHandleui64vARB( id, static_cast<unsigned>( count ), values );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const uvec3 *values, size_t count )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform_array, gl_enum::UNSIGNED_INT_VEC3 );
		write_data( values, sizeof( decltype( *values ) ) * count );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform3uiv( id, static_cast<unsigned>( count ), reinterpret_cast<const unsigned *>( values ) );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const ivec4 *values, size_t count )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform_array, gl_enum::INT_VEC4 );
		write_data( values, sizeof( decltype( *values ) ) * count );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.Uniform4iv( id, static_cast<unsigned>( count ), reinterpret_cast<const int *>( values ) );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_uniform( const detail::location_variant &location, const mat4 *values, size_t count, bool transpose )
{
	if ( _deferred )
	{
		write( cmd_type::set_uniform_array, gl_enum::FLOAT_MAT4, transpose );
		write_data( values, sizeof( decltype( *values ) ) * count );
		write_location_variant( location );
	}
	else if ( auto id = find_uniform_id( location ); id >= 0 )
		gl.UniformMatrix4fv( id, static_cast<unsigned>( count ), transpose ? 1 : 0, reinterpret_cast<const float *>( values ) );
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_state( const blend_state &bs )
{
	if ( _deferred )
		write( cmd_type::bind_blend_state, bs );
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_state( const depth_stencil_state &ds )
{
	if ( _deferred )
		write( cmd_type::bind_depth_stencil_state, ds );
	else
	{
		if ( ds.depth_test )
			glEnable( GL_DEPTH_TEST );
		else
			glDisable( GL_DEPTH_TEST );

		if ( ds.depth_write )
			glDepthMask( GL_TRUE );
		else
			glDepthMask( GL_FALSE );

		glDepthFunc( +ds.depth_func );

		if ( ds.stencil_test )
			glEnable( GL_STENCIL_TEST );
		else
			glDisable( GL_STENCIL_TEST );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::set_state( const rasterizer_state &rs )
{
	if ( _deferred )
		write( cmd_type::bind_rasterizer_state, rs );
	else
	{
		if ( rs.front_ccw )
			glFrontFace( GL_CCW );
		else
			glFrontFace( GL_CW );

		if ( rs.face_cull_mode == gl_enum::NONE )
			glDisable( GL_CULL_FACE );
		else if ( +rs.face_cull_mode == GL_FRONT )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
		}
		else if ( +rs.face_cull_mode == GL_BACK )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
		}
		else
			assert( 0 );

		if ( rs.wireframe )
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		else
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		if ( rs.depth_clamp )
			glEnable( +gl_enum::DEPTH_CLAMP );
		else
			glDisable( +gl_enum::DEPTH_CLAMP );

		if ( rs.scissor_test )
			glEnable( GL_SCISSOR_TEST );
		else
			glDisable( GL_SCISSOR_TEST );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::draw( gl_enum primitive, size_t first, size_t count, size_t instanceCount, size_t instanceBase )
{
	if ( _deferred )
	{
		write( cmd_type::draw, primitive, first, count, instanceCount, instanceBase );
	}
	else
	{
		if ( _state->dirty_input_assembly )
			synchronize_input_assembly();

		gl.DrawArraysInstancedBaseInstance(
		    primitive,
		    static_cast<int>( first ),
		    static_cast<unsigned>( count ),
		    static_cast<unsigned>( instanceCount ),
		    static_cast<unsigned>( instanceBase ) );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::draw_indexed( gl_enum primitive, size_t first, size_t count, size_t instanceCount, size_t instanceBase )
{
	if ( _deferred )
	{
		write( cmd_type::draw_indexed, primitive, first, count, instanceCount, instanceBase );
	}
	else
	{
		if ( _state->dirty_input_assembly )
			synchronize_input_assembly();

		if ( _state->current_ib_16bits )
		{
			gl.DrawElementsInstancedBaseInstance(
			    primitive,
			    static_cast<int>( count ),
			    gl_enum::UNSIGNED_SHORT,
			    reinterpret_cast<const void *>( _state->current_ib_offset + 2 * first ),
			    static_cast<unsigned>( instanceCount ),
			    static_cast<unsigned>( instanceBase ) );
		}
		else
		{
			gl.DrawElementsInstancedBaseInstance(
			    primitive,
			    static_cast<int>( count ),
			    gl_enum::UNSIGNED_INT,
			    reinterpret_cast<const void *>( _state->current_ib_offset + 4 * first ),
			    static_cast<unsigned>( instanceCount ),
			    static_cast<unsigned>( instanceBase ) );
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::execute( ptr cmdQueue )
{
	assert( cmdQueue && cmdQueue.get() != this );

	if ( _deferred )
	{
		write( cmd_type::execute );
		_resources.push_back( cmdQueue );
	}
	else
	{
		cmdQueue->execute( _state );
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool cmd_queue::synchronize_input_assembly()
{
	unsigned vaoID = _state->current_vb_layout
	                 ? detail::tl_currentContext->get_or_create_layout_vao( _state->current_vb_layout )
	                 : 0;

	if ( vaoID )
	{
		if ( _state->current_vb )
		{
			gl.VertexArrayVertexBuffer( vaoID, 0,
			                            _state->current_vb->id(),
			                            reinterpret_cast<const void *>( _state->current_vb_offset ),
			                            _state->current_vb_layout->stride );
		}
		else
			gl.VertexArrayVertexBuffer( vaoID, 0, 0, nullptr, 0 );

		if ( _state->current_ib )
			gl.VertexArrayElementBuffer( vaoID, _state->current_ib->id() );
		else
			gl.VertexArrayElementBuffer( vaoID, 0 );

		gl.BindVertexArray( vaoID );
	}
	else
	{

	}

	_state->dirty_input_assembly = false;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void cmd_queue::execute( gl_state *state )
{
	_state = state;
	_deferred = false;
	_position = 0;
	size_t resIndex = 0;

	while ( _position < _recordedData.size() )
	{
		auto cmd = read<cmd_type>();
		switch ( cmd )
		{
			default:
				assert( 0 );
				break;

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

			case cmd_type::update_texture:
			{
			}
			break;

			case cmd_type::update_buffer:
			{
				auto offset = read<size_t>();
				auto preserveContent = read<bool>();
				auto data = read_data();
				auto buff = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				update_buffer( buff, data.first, data.second, offset, preserveContent );
			}
			break;

			case cmd_type::resize_buffer:
			{
				auto data = read_data();
				auto buff = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				resize_buffer( buff, data.first, data.second );
			}
			break;

			case cmd_type::bind_blend_state:
				set_state( read<blend_state>() );
				break;

			case cmd_type::bind_depth_stencil_state:
				set_state( read<depth_stencil_state>() );
				break;

			case cmd_type::bind_rasterizer_state:
				set_state( read<rasterizer_state>() );
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
				const auto &layout = *read<const detail::layout *>();
				auto offset = read<size_t>();
				bind_vertex_buffer( vb, layout, offset );
			}
			break;

			case cmd_type::bind_vertex_attribute:
			{
				auto attribs = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				auto slot = read<unsigned>();
				auto glType = read<gl_enum>();
				auto perInstance = read<bool>();
				auto offset = read<size_t>();
				auto stride = read<size_t>();
				bind_vertex_attribute( attribs, slot, glType, perInstance, offset, stride );
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

			case cmd_type::bind_texture:
			{
				auto tex = std::static_pointer_cast<texture>( _resources[resIndex++] );
				bind_texture( tex, read<unsigned>() );
			}
			break;

			case cmd_type::bind_render_target:
			{
				assert( 0 );
			}
			break;

			case cmd_type::bind_storage_buffer:
			{
				auto buff = std::static_pointer_cast<buffer>( _resources[resIndex++] );
				auto slot = read<unsigned>();
				auto offset = read<size_t>();
				auto length = read<size_t>();
				bind_storage_buffer( buff, slot, offset, length );
			}
			break;

			case cmd_type::set_uniform_block:
			{
				auto location = read_location_variant();
				auto data = read_data();
				set_uniform_block( location, data.first, data.second );
			}
			break;

			case cmd_type::set_uniform:
			{
#define CASE_TYPE(_GLEnum, _Type) _GLEnum: { \
						auto value = read<_Type>(); \
						set_uniform(read_location_variant(), value); \
					} break

#define CASE_TYPE_MATRIX(_GLEnum, _Type) _GLEnum: { \
						auto value = read<_Type>(); \
						auto transpose = read<bool>(); \
						set_uniform( read_location_variant(), value, transpose ); \
					} break

				switch ( read<gl_enum>() )
				{
					case CASE_TYPE( gl_enum::BOOL, bool );
					case CASE_TYPE( gl_enum::INT, int );
					case CASE_TYPE( gl_enum::FLOAT, float );
					case CASE_TYPE( gl_enum::FLOAT_VEC2, vec2 );
					case CASE_TYPE( gl_enum::FLOAT_VEC3, vec3 );
					case CASE_TYPE( gl_enum::FLOAT_VEC4, vec4 );
					case CASE_TYPE_MATRIX( gl_enum::FLOAT_MAT3, mat3 );
					case CASE_TYPE_MATRIX( gl_enum::FLOAT_MAT4, mat4 );

					default:
						assert( 0 );
						break;
				}

#undef CASE_TYPE
#undef CASE_TYPE_MATRIX
			}
			break;

			case cmd_type::set_uniform_array:
			{
				auto type = read<gl_enum>();
				auto data = read_data();

				switch ( type )
				{
					case gl_enum::UNSIGNED_INT64:
						set_uniform( read_location_variant(), reinterpret_cast<const uint64_t *>( data.first ), data.second / sizeof( uint64_t ) );
						break;

					case gl_enum::UNSIGNED_INT_VEC3:
						set_uniform( read_location_variant(), reinterpret_cast<const uvec3 *>( data.first ), data.second / sizeof( uvec3 ) );
						break;

					case gl_enum::INT_VEC4:
						set_uniform( read_location_variant(), reinterpret_cast<const ivec4 *>( data.first ), data.second / sizeof( ivec4 ) );
						break;

					case gl_enum::FLOAT_MAT4:
						set_uniform( read_location_variant(), reinterpret_cast<const mat4 *>( data.first ), data.second / sizeof( mat4 ) );
						break;

					default:
						assert( 0 );
						break;
				}
			}
			break;

			case cmd_type::draw:
			case cmd_type::draw_indexed:
			{
				auto primitive = read<gl_enum>();
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

		check_gl_error();
	}

	_state = nullptr;
	_deferred = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

#if defined(WIN32)
unsigned g_contextAttribs[] =
{
	+gl_enum::CONTEXT_MAJOR_VERSION, 4,
	+gl_enum::CONTEXT_MINOR_VERSION, 6,
	+gl_enum::CONTEXT_PROFILE_MASK, +gl_enum::CONTEXT_CORE_PROFILE_BIT,
	0
};

//---------------------------------------------------------------------------------------------------------------------
context::context( void *windowNativeHandle, ptr sharedContext )
	: cmd_queue( &_glState )
	, _window_native_handle( windowNativeHandle )
{
	static HMODULE s_renderDoc = LoadLibraryA( "renderdoc.dll" );

	auto hdc = GetDC( HWND( _window_native_handle ) );
	auto tempContext = wglCreateContext( hdc );
	wglMakeCurrent( hdc, tempContext );

	gl = gl_api();

	_native_handle = gl.CreateContextAttribsARB(
	                     hdc,
	                     sharedContext ? HGLRC( sharedContext->_native_handle ) : nullptr,
	                     reinterpret_cast<const int *>( g_contextAttribs ) );

	reset();
	wglMakeCurrent( nullptr, nullptr );
	wglDeleteContext( tempContext );
}

//---------------------------------------------------------------------------------------------------------------------
context::context( ptr sharedContext )
	: cmd_queue( &_glState )
	, _window_native_handle( sharedContext->_window_native_handle )
{
	sharedContext->make_current();

	_window_native_handle = sharedContext->_window_native_handle;
	auto hdc = GetDC( HWND( _window_native_handle ) );

	_native_handle = gl.CreateContextAttribsARB(
	                     hdc,
	                     HGLRC( sharedContext->_native_handle ),
	                     reinterpret_cast<const int *>( g_contextAttribs ) );

	reset();
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

	tl_currentContext = this;
}
#else
#error Not implemented!
#endif

//---------------------------------------------------------------------------------------------------------------------
unsigned context::get_or_create_layout_vao( const detail::layout *layout )
{
	assert( layout );
	unsigned vaoID = 0;

	auto ptrKey = reinterpret_cast<std::uintptr_t>( layout );
	if ( auto iter = _layoutVAOs.find( ptrKey ); iter == _layoutVAOs.end() )
	{
		gl.CreateVertexArrays( 1, &vaoID );

		for ( unsigned i = 0; i < 8; ++i )
		{
			if ( layout->mask & ( 1u << i ) )
				gl.EnableVertexArrayAttrib( vaoID, i );
			else
				gl.DisableVertexArrayAttrib( vaoID, i );
		}

		for ( auto &a : layout->attribs )
		{
			gl.VertexArrayAttribFormat( vaoID, a.location, a.element_count, a.element_type, 0, a.offset );
			gl.VertexArrayAttribBinding( vaoID, a.location, 0 );
		}

		_layoutVAOs.insert( { ptrKey, { layout, vaoID, 0 } } );
	}
	else
	{
		iter->second.unused_frames = 0;
		vaoID = iter->second.vao_id;
	}

	return vaoID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
async_upload_context::async_upload_context( context::ptr mainContext )
{

}

//---------------------------------------------------------------------------------------------------------------------
async_upload_context::~async_upload_context()
{

}

} // namespace gl3d::detail

} // namespace gl3d
