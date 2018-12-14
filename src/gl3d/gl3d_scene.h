#ifndef __GL3D_SCENE_H__
#define __GL3D_SCENE_H__

#include "gl3d.h"

namespace gl3d {

struct centered_box
{
	vec3 center;
	vec3 extent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class bvh
{
public:
	struct node
	{
		unsigned children[2];
		unsigned first_leaf;
		unsigned leaf_count;
	};

	struct leaf
	{

	};

protected:
};

} // namespace gl3d

#endif // __GL3D_SCENE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
	#ifndef __GL3D_SCENE_H_IMPL__
		#define __GL3D_SCENE_H_IMPL__
		#include "gl3d_scene.inl"
	#endif // __GL3D_SCENE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
