#ifndef __GL3D_2D_H__
#define __GL3D_2D_H__

#include <map>

#include "gl3d.h"

namespace gl3d {

class font
{
public:
	using ptr = std::shared_ptr<font>;

	font( const void *data, size_t length );
	font( const detail::bytes_t &bytes ): font( bytes.data(), bytes.size() ) { }
	font( const char *base64Data, size_t length = 0 );

	int base() const { return _base; }
	int height() const { return _height; }

	struct char_info
	{
		int id = 0;
		uvec2 size;
		ibox2 box;
		ivec2 offset;
		vec2 uv[4];
		int x_advance;
	};

	using char_info_map = std::map<int, char_info>;
	const char_info_map &char_infos() const { return _charInfos; }

	using kernings_map = std::map<std::pair<int, int>, int>;
	const kernings_map &kernings() const { return _kernings; }

protected:
	int _base = 0, _height = 0;
	char_info_map _charInfos;
	kernings_map _kernings;
	texture::ptr _texture;
};

} // namespace gl3d

#endif // __GL3D_2D_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_2D_H_IMPL__
		#define __GL3D_2D_H_IMPL__
		#include "gl3d_2d.inl"
	#endif // __GL3D_2D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
