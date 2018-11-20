#ifndef __GL3D_2D_H__
#define __GL3D_2D_H__

#include <map>

#include "gl3d.h"

namespace gl3d {

struct font
{
	using ptr = std::shared_ptr<font>;

	int base = 0;
	int line_height = 0;

	struct char_info
	{
		int id = 0;
		uvec2 size;
		ibox2 box;
		ivec2 offset;
		vec2 uv[4];
		int x_advance;
	};

	std::map<int, char_info> char_infos;
	std::map<std::pair<int, int>, int> kernings;
	texture::ptr char_atlas;

	static ptr create( const char *base64Data );
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
