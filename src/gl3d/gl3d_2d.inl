#ifndef __GL3D_2D_H_IMPL__
	#define __GL3D_2D_H_IMPL__
#endif

#include "gl3d_2d.h"

namespace gl3d {

namespace detail {

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
font::font( const void *data, size_t length )
{
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

	/*
	font_texture->set_params( texWidth, texHeight, GL_RGBA, 1, 1 );
	font_texture->alloc_pixels( image );
	font_texture->set_wrap( gl_api.CLAMP_TO_EDGE );
	*/

	data = cursorInput;

	_height = GL3D_DATA_EXTRACT( uint8_t );
	_base = GL3D_DATA_EXTRACT( uint8_t );

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

		_charInfos[chi.id] = chi;
	}

	int numKernings = GL3D_DATA_EXTRACT( int );
	for ( int i = 0; i < numKernings; ++i )
	{
		std::pair<int, int> p;
		p.first = GL3D_DATA_EXTRACT( int );
		p.second = GL3D_DATA_EXTRACT( int );
		_kernings[p] = GL3D_DATA_EXTRACT( int8_t );
	}

#undef GL3D_DATA_EXTRACT
}

//---------------------------------------------------------------------------------------------------------------------
font::font( const char *base64Data, size_t length )
	: font( detail::base64_decode( base64Data, length ) )
{

}

} // namespace gl3d
