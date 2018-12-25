#ifndef __GL3D_QUICK_DRAW_H_IMPL__
	#define __GL3D_QUICK_DRAW_H_IMPL__
#endif

#include "gl3d_quick_draw.h"

namespace gl3d {

namespace detail {

constexpr size_t k_vertexBatchAllocation = 256;
constexpr size_t k_renderBatchSize = 64;

//---------------------------------------------------------------------------------------------------------------------
bool is_base64( uint8_t c ) { return ( isalnum( c ) || ( c == '+' ) || ( c == '/' ) ); }

//---------------------------------------------------------------------------------------------------------------------
bytes_t base64_decode( const char *encoded_string, size_t decompressedLength = 0 )
{
	static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const char *data = encoded_string;
	size_t len = strlen( encoded_string );

	if ( len < decompressedLength )
	{
		auto *unrolled = new char[decompressedLength + 1];
		unrolled[decompressedLength] = 0;

		for ( size_t i = 0, j = 0; i < len; ++i )
		{
			if ( encoded_string[i] != '~' )
				unrolled[j++] = encoded_string[i];
			else
				for ( size_t k = 0, S = 3 + base64_chars.find( encoded_string[++i] ); k < S; ++k )
					unrolled[j++] = encoded_string[i - 2];
		}

		data = unrolled;
		len = decompressedLength;
	}

	int i = 0, cursor = 0;
	uint8_t char_array_4[4], char_array_3[3];
	std::vector<uint8_t> result;

	while ( len-- && ( data[cursor] != '=' ) && is_base64( data[cursor] ) )
	{
		char_array_4[i++] = data[cursor++];

		if ( i == 4 )
		{
			for ( i = 0; i < 4; ++i ) char_array_4[i] = static_cast<uint8_t>( base64_chars.find( char_array_4[i] ) );

			char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
			char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );
			char_array_3[2] = ( ( char_array_4[2] & 0x3 ) << 6 ) + char_array_4[3];

			for ( i = 0; ( i < 3 ); ++i ) result.push_back( char_array_3[i] );
			i = 0;
		}
	}

	if ( i )
	{
		for ( int j = i; j < 4; j++ ) char_array_4[j] = 0;
		for ( auto &j : char_array_4 ) j = static_cast<uint8_t>( base64_chars.find( j ) );

		char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
		char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );
		char_array_3[2] = ( ( char_array_4[2] & 0x3 ) << 6 ) + char_array_4[3];

		for ( int j = 0; ( j < i - 1 ); j++ ) result.push_back( char_array_3[j] );
	}

	if ( data != encoded_string )
		delete[] data;

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
static const size_t default_base64_length = 5484;
static const char *default_base64 =
    "gABAAAgHXACxN978GB9gAwMDbn8IDEYAGWbGhoEgYAMDA2NgPgyGgBhmxoZdXmADAwNjMEsMhoAMxsaGWVt+OzuzZzALDIaADMbGhllbY2dnG2MYHrgDQQzG"
    "xoZZW2NjYw9jGDwMBkEMxsaGWVtjY2MHYwxoDAYiDMbGhlk2Y2NjD2MMaQwGIgzGxvyYAHNjYxtjDD4MBiIYZsZgGB9uP2MzYwAIDAYUGGbGwA0A~EwCAccF"
    "LA33gAAMAz74ePjMwA~GP3gMGzM2NjQOwogNDIkHP2N4DBs2NjY0G8GcHQ7JjGFjzAwbNjY2MJvAnD3P34xhY8z8G+YzNjCbwJT1zZ+HYT/MDBs25jcATo73"
    "5IyEs2Eb/g0bNgY2NCCb90Tsz7ZhM4YNGzMGMzQgm/cE7M+cYWOGDfvh4+EDEBtjBEzCnGHDA~GgOYwRMgjc/AAA+Pj5jO34A~GH4+Y2FhM2djPz88PkNjY2"
    "NjQ2NgYBtjY2NjBkNHY2NjY0NjYGAPY2NjYwNDRzZjY2MDYzA8B2NjY2M/A00cNmNjPmMYYA9jc2M/Y3NZHDZjc2BjDGAbP24/Y2NjWRw2Y25hYwZhMwNgA2"
    "NjY3E2HGNgYT5/PmMDYANjY2NxYxxjPj4A~EDPz5+YWMcPgAAYLxtALAdA~GB+n2cYjAGMMzB+nsFv9psxGAP2mQEGgzE4BrHBYAabPRgDZpgB2IAxNAawwW"
    "AGmzEYA2aYAdiAMTI+mMHAg/EwGB9mmA0GgzExYIzBz8PwMBgDZpiNAYwxf2CMwYBhYDAYA2aYbQAwADBggMGAMWAwGAPGvQUA~AwYYzBgDEwMBjDAwAAcAg"
    "DMD6M34/xM/wAAMC3xviIBA~HmDke9taKTwgz+47PGAJn5plNAzPgCScQM5vZ2BiCbDbzDIMx0AIAALaZ2dgYAmw2YzjDMMA~AC2mdnY2J/P8/NgwzAGA~A/"
    "pnZ2BjCzDOYZcM3BgAAAMyY2dgcwowxmjkeA~CDMmJmPG4KP4QEA~aH/bNvv3A~M22YDA~NNsGA~NH8BA~/A~/A~/A~GPDF8A~AgA~AKQAzAAMB/w4EIQAAA"
    "HwACgACCQEDBSIA~ALADMABQMAAgYjA~AJQANAAgJAAMJJA~GHDAACCCUA~D0ADQkAAw4mA~ALgANAAgJAAMJJwAAABEAMwACAwACAygA~AiA~ABAwAAgUpA"
    "~ALA~BQMAAIFKgAAAGQAKQAGBQECCCsA~AmACsABwcBBAosA~ACAAzAAIEAQoELQAAACQAMwAEAQAHBS4A~AYADMAAgIBCgQvA~AHA~BUMAAIHMA~AFAAFQA"
    "HCQADCDEA~A6ACEABgkBAwgyA~AWAAVAAcJAAMIMwAAAGAAFQAHCQADCDQA~CCEABwkAAwg1A~ACAAhAAcJAAMINgAAABAAFwAHCQADCDcAAAB4A~ABwkAAw"
    "g4A~AZAALAAcJAAMIOQAAAGwACwAHCQADCDoAAABhACkAAgcBBQQ7A~AYgAfAAIJAQUEPA~AG4AHwAICAEECj0A~CDMABwQBBgo+A~AZQAfAAgIAQQKPwAAA"
    "BAAIQAGCQADB0A~ABGA~ACQoAAwpBA~ASQALAAgJAAMJQgA~AgAFwAHCQADCEMAAAB0AAsABwkAAwhEA~AWwALAAgJAAMJRQAAAB4AIQAGCQADB0YAAABIAC"
    "AABQkAAwZHA~AGAAXAAcJAAMISA~AFIACwAICQADCUkAAABaAB8ABAkAAwVKA~ATgAgAAUJAAMGSwAAAGgAFQAHCQADCEwA~AXACEABgkAAwdNA~AGgANAAo"
    "JAAMLTgAAACAAFwAHCQADCE8A~A3AA0ACAkAAwlQA~DXAAcJAAMIUQAAADkA~AICwADCVIAAABAAAwACAkAAwlTA~ASAAWAAcJAAMIVA~AEEAIAAGCQADB1U"
    "A~A4ABcABwkAAwhWA~AMAAXAAcJAAMIVwA~A4ADQALCQADDFgA~AoABcABwkAAwhZA~AJQAhAAYJAAMHWgAAACwAIQAGCQADB1sA~AxA~ABAwAAgVcA~AFgA"
    "~AUMAAIHXQAAACcA~AEDAACBV4AAAB1ACgACAQBAwpfA~AGwAzAAgBAA0IYA~ABQAMwADAgICB2EA~AuACsABgcABQdiA~AWA~BcKAAIIYwAAAFAAKgAFBwA"
    "FBmQAAABQA~ABwoAAghlA~APAArAAYHAAUHZgAAAG8A~AFCgACBWcAAABAABYABwkABQhoA~AYA~BcKAAIIaQAAAF8AHwACCQADA2oAAABCA~BwsAAwRrA~A"
    "aA~BYKAAIHbA~AHUA~ACCgACA20A~ALACsACgcABQtuA~AdwAfAAcHAAUIbwAAABYAKwAHBwAFCHA~ABwABUABwkABQhxA~AeAAVAAcJAAUIcgAAAFwAKQAE"
    "BwAFBXMAAABKACoABQcABQZ0A~AVAAfAAUJAAMGdQAAAB4AKwAHBwAFCHYA~A1ACsABgcABQd3A~DrAAoHAAULeA~AEMAKgAGBwAFB3kA~AzACEABgkABQd6"
    "A~AVgApAAUHAAUGewA~A8A~AGDAACB3wA~A2A~BgwCAgd9A~ACA~BYMAAIHfgAAAGsAKAAJBAAGCgA~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A"
    "~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~p=";

//---------------------------------------------------------------------------------------------------------------------
static const size_t monospace_base64_length = 5484;
static const char *monospace_base64 =
    "gABAAONB4A4YeuHxDCAOR34i32czQTAYGGFjmAgwGyUzttlsE/IwGIBhZowZMBs1M7bZbPNaMBjfYGyMERAbFx+U2WybWjAY2GBsjDEYDggDnNls23occNhg"
    "bIwxCD88PwzfZ9viMRjYYGzcIIw5LGMM2ADbojEY2GBseGCMMSpjDNhgu6ExGNhgbGBABH87PgfYYAv4MBiYYWZgwAYA~ETIDAYGWNiwAEAIIgHh+dj8yDgD"
    "g564QGA5GzMhMdmZgMA~DCAh+RsbIxmbgyAAWxwcA8zzOjvTWTMZm8MgwFswBkMY2aAZM1mbOZtbIMBbIAZDGBmQGLNYuzvbGyDj++DGczDY+5vj8MGxmYGs"
    "9ls9jwMg2FMYo7DA8bjA5vZbDYYDIPBTGIOA~CCP2WwGGAyDgQ8AAPh9ZjwPn9lsNhgMgwHA5+dgzGaGGZuZ7zMY3wcAzwyiYMxmgxkzA~EjMEMo2HMZoMZg"
    "M9n5pkxOIzBB6FhfGYDH7MBbBbbOAy/x4wwYQxmAxieAWwGWzQGDN7M8GMMZgYMno/nB3swPgzYTBBjDDw8BwwYbIZ5MGYM2OcXAgA~BcGGzGWDBmjA8AAAC"
    "fmWdmGhhsZtgwZjgA2L+gsdnMY7PHZ/ab/TwADMy2oLHZj0EzA~EnhnGtqSx2UBBgN8n+r5h+4wxw7a7sdlBMx7YLBqwYRsMYMY2Gx+fbwYzzKwasGEbjDHM"
    "NhsA~BM8ys+rBhG4wZGAAA/h4AAB7GYxuwofoMDQDfzgA~B+wmYbsKEaDAFswNwA~BM8NmG7IhGt44bd8A~CDPBbPucLxoAdG0A~EeA~EQOQA~GM/u40Ufg6"
    "EA~GB7YbTZsyI+xAQA~FD32A2bAzjtwEA~Fj9lgNijGjwEA~GLzZYDYoAoMBA~Hw32DmOR8A~JHwA~/A~/A~/A~/A~/A~WPDF8A~AgA~AbgAjAAMB/w4HIQA"
    "AAH0A~ACCQMDByIAAABKACUABQMBAwcjA~ARQALAAcIAAQHJA~BsA~AGDAECByUAAABYA~ABwkAAwcmA~ATwA~AgJAAMHJwAAAFYAJAACAwMDBygA~AmA~AB"
    "AwCAwcpA~AMA~BQMAQMHKgAAAC4AKQAGBQEDBysA~A1ACkABgUBBgcsA~AfAAcAAMFAQkHLQAAAGkAIwAEAQIIBy4AAABeACQAAgICCgcvA~ASQA~AUKAAMH"
    "MA~AG0ACgAHCAAEBzEA~AqABcABggBBAcyA~AHAAXAAYIAQQHMwA~A4AFwAGCAEEBzQAAABlAAoABwgABAc1A~ABwAXAAYIAQQHNgAAADEAFwAGCAEEBzcA~"
    "AHACAABggBBAc4A~DhAAYIAQQHOQAAAHcAEwAGCAAEBzoAAAB9AAoAAgYCBgc7A~APgAfAAMIAgYHPA~AEgAHQAFBwEFBz0AAABQACQABQMBBwc+A~AQgAeA"
    "AUHAQUHPwAAABwADQAFCQIDB0A~BDA~ABwwAAwdBA~AVAAUAAYIAQQHQgAAAEYAFAAGCAEEB0MAAABwABMABggBBAdEA~AdQAKAAcIAQQHRQAAABsAIAAFCA"
    "EEB0YA~AzACAABQgBBAdHA~APQAMAAcIAAQHSA~ABUAFwAGCAEEB0kA~A5AB8ABAgCBAdKA~AIQAgAAUIAQQHSwAAACMAFwAGCAEEB0wA~AnACAABQgBBAdN"
    "A~ALQAgAAUIAQQHTgAAAE0ACwAHCAEEB08AAABdAAoABwgABAdQA~AYgATAAYIAQQHUQAAADoA~AHCwAEB1IA~AOACAABggBBAdTA~APwAVAAYIAQQHVA~AF"
    "sAEwAGCAEEB1UAAABpABMABggBBAdWA~AVQAKAAcIAAQHVwAAABUAIAAFCAEEB1gA~CBgABggBBAdZA~ANAANAAgIAAQHWgAAAE0AFAAGCAEEB1sA~A1A~AB"
    "AwBAwdcA~AQgA~AYKAQMHXQAAACsA~AEDAIDB14A~A8ACgABQQBBAdfA~AYQAjAAcBAA4HYA~AFkAJAAEAgEDB2EA~AHACkABgYBBgdiA~AFQANAAYJAQMHY"
    "wAAABwAKQAFBgEGB2QA~AOAA0ABgkAAwdlA~AbgAcAAYGAQYHZgAAACIADQAFCQEDB2cAAABgA~ABwkABgdoA~ABwANAAYJAQMHaQAAAC4ADQAFCQEDB2oA~"
    "AgA~ABQwBAwdrA~DOAAYJAQMHbA~ACgADQAFCQEDB20AAABOAB0ACAYABgduA~AFQApAAYGAQYHbwAAAF8AHAAHBgAGB3A~AB2A~ABgkBBgdxA~AbwA~AYJA"
    "AYHcgA~A4AKQAGBgEGB3MA~CCoABgYBBgd0A~AOAAWAAYIAAQHdQAAAGcAHAAGBgEGB3YA~AiACkABQYBBgd3A~AVwAdAAcGAAYHeA~AHUAHAAGBgEGB3kAA"
    "ABoA~ABgkABgd6A~AKAApAAUGAQYHewAAABIA~AGDAADB3wA~Gg0DAgd9A~AGQA~AYMAQMHfgAAAEIAJgAHAwAHBwA~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A"
    "~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~p=";

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
font::ptr font::create( const char *base64Data )
{
	auto bytes = detail::base64_decode( base64Data );
	auto data = reinterpret_cast<const void *>( bytes.data() );
	auto size = bytes.size();

#define GL3D_DATA_EXTRACT(_Type) \
	(*reinterpret_cast<const _Type *>(data)); data = reinterpret_cast<const uint8_t *>(data) + sizeof(_Type)

	int texWidth = GL3D_DATA_EXTRACT( short );
	int texHeight = GL3D_DATA_EXTRACT( short );
	auto texWidthf = static_cast<float>( texWidth );
	auto texHeightf = static_cast<float>( texHeight );

	std::unique_ptr<uint32_t[]> image{ new uint32_t[texWidth * texHeight] };
	memset( image.get(), 0, texWidth * texHeight * sizeof( uint32_t ) );

	uint32_t *cursorOutput = image.get();
	auto cursorInput = reinterpret_cast<const uint8_t *>( data );

	for ( int xy = 0; xy < texWidth * texHeight; xy += 8, ++cursorInput )
	{
		auto byte = *cursorInput;
		for ( int i = 0; i < 8; ++i, ++cursorOutput, byte >>= 1 )
			if ( byte & 1 )
			{
				*cursorOutput = 0xFFFFFFFFu;
				*( cursorOutput + texWidth + 1 ) = 0xFF000000u; // Remove this to get rid of black font shadow
			}
	}

	image[texWidth * texHeight - 1] = 0xFFFFFFFFu;

	data = cursorInput;

	font::ptr result = std::make_shared<font>();
	result->char_atlas = texture::create( gl_internal_format::RGBA8, uvec2{ texWidth, texHeight }, image.get() );
	result->line_height = GL3D_DATA_EXTRACT( uint8_t );
	result->base = GL3D_DATA_EXTRACT( uint8_t );

	int numChars = GL3D_DATA_EXTRACT( int );
	for ( int i = 0; i < numChars; ++i )
	{
		char_info chi;
		chi.id = GL3D_DATA_EXTRACT( int );
		chi.box.min.x = GL3D_DATA_EXTRACT( uint16_t );
		chi.box.min.y = GL3D_DATA_EXTRACT( uint16_t );
		chi.size.x = 1 + GL3D_DATA_EXTRACT( uint8_t );
		chi.size.y = 1 + GL3D_DATA_EXTRACT( uint8_t );
		chi.box.max.x = chi.box.min.x + chi.size.x;
		chi.box.max.y = chi.box.min.y + chi.size.y;
		chi.offset.x = GL3D_DATA_EXTRACT( int8_t );
		chi.offset.y = GL3D_DATA_EXTRACT( int8_t );
		chi.x_advance = GL3D_DATA_EXTRACT( int8_t );
		for ( int j = 0; j < 4; ++j )
			chi.uv[j] = vec2( chi.box.corner( j ).x / texWidthf, chi.box.corner( j ).y / texHeightf );

		result->char_infos[chi.id] = chi;
	}

	int numKernings = GL3D_DATA_EXTRACT( int );
	for ( int i = 0; i < numKernings; ++i )
	{
		std::pair<int, int> p;
		p.first = GL3D_DATA_EXTRACT( int );
		p.second = GL3D_DATA_EXTRACT( int );
		result->kernings[p] = GL3D_DATA_EXTRACT( int8_t );
	}
#undef GL3D_DATA_EXTRACT

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
quick_draw::quick_draw()
{
	static const char *s_immediateShader = R"SHADER_SOURCE(
#vertex

	layout (location = 0) in vec4 v_PositionColor;
	layout (location = 1) in uvec4 v_Data;

	in int gl_BaseInstance;

	uniform mat4 u_ProjectionMatrix;
	uniform mat4 u_ViewMatrix;
	uniform mat4 u_Transforms[64];

	out vec4 Color;
	out vec2 UV;
	flat out uint TextureIndex;

	void main()
	{
		mat4 transform = u_Transforms[gl_BaseInstance + gl_InstanceID];
		gl_Position = u_ProjectionMatrix * u_ViewMatrix * transform * vec4(v_PositionColor.xyz, 1);
		
		uint c = floatBitsToUint(v_PositionColor.w);
		Color = vec4(float(c & 0xFF), float((c >> 8) & 0xFF), float((c >> 16) & 0xFF), float((c >> 24) & 0xFF));
		Color /= 255.0;
		UV = vec2(uintBitsToFloat(v_Data.x), uintBitsToFloat(v_Data.y));
		TextureIndex = (v_Data.z >> 16);
	}

#fragment

	uniform uint64_t u_Textures[64];

	layout(origin_upper_left) in vec4 gl_FragCoord;
	in vec4 Color;
	in vec2 UV;
	flat in uint TextureIndex;

	out vec4 out_Color;

	void main()
	{
		out_Color = Color * texture(sampler2D(u_Textures[TextureIndex]), UV);
	}
	)SHADER_SOURCE";

	auto code = shader_code::create();
	code->source( s_immediateShader );
	_shader = shader::create( code );

	reset();
}

//---------------------------------------------------------------------------------------------------------------------
quick_draw::~quick_draw()
{

}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::reset()
{
	_dirtyBuffers = true;

	_states.clear();
	_currentState = _states.emplace_back();

	_drawCalls.clear();
	_indices.clear();

	_vertices.resize( detail::k_vertexBatchAllocation );
	_currentVertex = _vertices.begin();
	_startVertex = UINT_MAX;

	_transforms = { mat4() };

	_currentData = { 0, 0, 0, 0 };
	set_scissors( { 0, 0, 4095, 4095 } );
	uv( { 0, 0 } );

	_textureHandles = { 0 };
	_textureIndexMap = { { texture::white_pixel(), 0 } };

	unsigned white = 0xFFFFFFFFu;
	memcpy( &_currentColor, &white, 4 );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::render( cmd_queue::ptr queue, const mat4 &view, const mat4 &proj )
{
	if ( _drawCalls.empty() )
		return;

	if ( _dirtyBuffers )
	{
		if ( !_vertexBuffer )
			_vertexBuffer = buffer::create( buffer_usage::dynamic_resizable, _vertices );
		else
		{
			auto verticesSize = _vertices.size() * sizeof( compact_gpu_vertex );

			if ( verticesSize > _vertexBuffer->size() )
				queue->resize_buffer( _vertexBuffer, _vertices.data(), verticesSize );
			else
				queue->update_buffer( _vertexBuffer, _vertices.data(), verticesSize );
		}

		if ( !_indexBuffer )
			_indexBuffer = buffer::create( buffer_usage::dynamic_resizable, _indices );
		else
		{
			auto indicesSize = _indices.size() * sizeof( unsigned );

			if ( indicesSize > _indexBuffer->size() )
				queue->resize_buffer( _indexBuffer, _indices.data(), indicesSize );
			else
				queue->update_buffer( _indexBuffer, _indices.data(), indicesSize );
		}

		for ( auto &kvp : _textureIndexMap )
		{
			auto tex = kvp.first;
			_textureHandles[kvp.second] = tex->synchronize();
		}

		_dirtyBuffers = false;
	}

	queue->bind_shader( _shader );
	queue->bind_vertex_buffer( _vertexBuffer, compact_gpu_vertex::layout() );
	queue->bind_index_buffer( _indexBuffer );
	queue->set_uniform( "u_ProjectionMatrix", proj );
	queue->set_uniform( "u_ViewMatrix", view );
	queue->set_uniform( "u_Textures", _textureHandles );

	unsigned currentStateIndex = UINT_MAX;
	mat4 transforms[detail::k_renderBatchSize];

	size_t start = 0;
	while ( start < _drawCalls.size() )
	{
		size_t end = minimum( start + detail::k_renderBatchSize, _drawCalls.size() );
		auto count = end - start;

		for ( size_t i = 0; i < count; ++i )
		{
			auto id = _drawCalls[start + i].transformIndex;
			transforms[i] = _transforms[id];
		}

		queue->set_uniform( "u_Transforms", transforms, count );

		for ( size_t i = start, instanceID = 0; i < end; ++i, ++instanceID )
		{
			const auto &dc = _drawCalls[i];
			if ( dc.stateIndex != currentStateIndex )
			{
				currentStateIndex = dc.stateIndex;
				const auto &state = _states[currentStateIndex];
				queue->set_state( state.ds );
				queue->set_state( state.rs );
			}

			queue->draw_indexed( dc.primitive, dc.firstIndex, dc.indexCount, 1, instanceID );
		}

		start += detail::k_renderBatchSize;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::push_transform()
{
	assert( !building_mesh() );
	_transforms.push_back( _transforms.back() );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::push_transform( const mat4 &mult )
{
	assert( !building_mesh() );
	_transforms.push_back( _transforms.back() * mult );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::pop_transform()
{
	assert( !building_mesh() && _transforms.size() > 1 ); // There should always remain 1 matrix on the stack (identity)
	_transforms.pop_back();
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::bind_texture( texture::ptr tex )
{
	_currentData.z &= 0x0000FFFFu;

	if ( tex )
	{
		auto iter = _textureIndexMap.find( tex );
		if ( iter != _textureIndexMap.end() )
		{
			_currentData.z |= ( iter->second << 16 );
			return;
		}

		auto index = static_cast<unsigned>( _textureHandles.size() );
		_currentData.z |= ( index << 16 );
		_textureIndexMap.insert( { tex, index } );
		_textureHandles.push_back( 0 );
		_dirtyBuffers = true;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::bind_font( font::ptr f )
{
	assert( !building_mesh() );

	if ( f )
	{

	}
	else
	{

	}
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::set_scissors( const uvec4 &box )
{
	assert( box.x < 4096 && box.y < 4096 && box.z < 4096 && box.w < 4096 );

	uint64_t scissorData = box.x | ( box.y << 12 ) | ( box.z << 24 ) | ( static_cast<uint64_t>( box.w ) << 36 );
	_currentData.z &= 0xFFFF0000u;
	_currentData.z |= static_cast<unsigned>( scissorData >> 32u );
	_currentData.w = scissorData & 0xFFFFFFFFu;
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::begin( gl_enum primitiveType )
{
	assert( !building_mesh() );

	_currentDrawCall.primitive = primitiveType;
	_currentDrawCall.firstIndex = static_cast<unsigned>( _indices.size() );
	_currentDrawCall.indexCount = 0;
	_currentDrawCall.stateIndex = static_cast<unsigned>( _states.size() - 1 );

	_buildingMesh = true;
	_startVertex = static_cast<unsigned>( _currentVertex - _vertices.begin() );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::end()
{
	assert( building_mesh() );
	_buildingMesh = false;

	auto numVertices = static_cast<unsigned>( _currentVertex - _vertices.begin() ) - _startVertex;
	if ( !numVertices )
		return;

	switch ( _currentDrawCall.primitive )
	{
		case gl_enum::LINES:
		{
			assert( ( numVertices % 2 ) == 0 );
			_currentDrawCall.indexCount = numVertices;

			_indices.resize( _indices.size() + numVertices );
			for ( unsigned i = 0, *index = _indices.data() + _currentDrawCall.firstIndex, v = _startVertex; i < numVertices; ++i )
				*index++ = v++;
		}
		break;

		case gl_enum::QUADS:
		{
			_currentDrawCall.primitive = gl_enum::TRIANGLES;

			assert( ( numVertices % 4 ) == 0 );
			_currentDrawCall.indexCount = ( numVertices / 4 ) * 6;

			_indices.resize( _indices.size() + _currentDrawCall.indexCount );
			for ( unsigned i = 0, *index = _indices.data() + _currentDrawCall.firstIndex, v = _startVertex; i < numVertices; i += 4 )
			{
				*index++ = v;
				*index++ = v + 1;
				*index++ = v + 2;

				*index++ = v;
				*index++ = v + 2;
				*index++ = v + 3;

				v += 4;
			}
		}
		break;
	}

	bool pushDrawCall = _drawCalls.empty() || ( !_drawCalls.back().try_merging_with( _currentDrawCall ) );
	if ( pushDrawCall )
		_drawCalls.push_back( _currentDrawCall );

	_dirtyBuffers = true;
	_startVertex = UINT_MAX;
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::vertex( const vec3 &pos )
{
	assert( building_mesh() );

	if ( _currentVertex == _vertices.end() )
	{
		_vertices.resize( _vertices.size() + detail::k_vertexBatchAllocation );
		_currentVertex = _vertices.end() - detail::k_vertexBatchAllocation;
	}

	_currentVertex->posColor = vec4( pos.x, pos.y, pos.z, _currentColor );
	_currentVertex->data = _currentData;
	++_currentVertex;
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::color( const vec4 &c )
{
	auto u = static_cast<unsigned>( clamp( c.x, 0.0f, 1.0f ) * 255.0f );
	u |= static_cast<unsigned>( clamp( c.y, 0.0f, 1.0f ) * 255.0f ) << 8u;
	u |= static_cast<unsigned>( clamp( c.z, 0.0f, 1.0f ) * 255.0f ) << 16u;
	u |= static_cast<unsigned>( clamp( c.w, 0.0f, 1.0f ) * 255.0f ) << 24u;

	memcpy( &_currentColor, &u, 4 );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::uv( const vec2 &texCoord )
{
	memcpy( _currentData.data, texCoord.data, 8 );
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::draw_aabb( const box3 &aabb, bool filled )
{
	if ( filled )
	{

	}
	else
	{
		begin( gl_enum::LINES );

		end();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::draw_frustum( const mat4 &worldProj )
{

}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::draw_texture( const vec2 &pos, texture::ptr tex, float scale )
{
	if ( !tex )
		return;

	auto oldData = _currentData;
	bind_texture( tex );
	begin( gl_enum::QUADS );
	{
		uv( { 0, 1 } );
		vertex( pos );
		uv( { 1, 1 } );
		vertex( { pos.x + tex->width() * scale, pos.y } );
		uv( { 1, 0 } );
		vertex( { pos.x + tex->width() * scale, pos.y + tex->height() * scale } );
		uv( { 0, 0 } );
		vertex( { pos.x, pos.y + tex->height() * scale } );
	}
	end();
	_currentData = oldData;
}

//---------------------------------------------------------------------------------------------------------------------
void quick_draw::draw_text( const vec2 &pos, std::string_view text )
{

}

} // namespace gl3d
