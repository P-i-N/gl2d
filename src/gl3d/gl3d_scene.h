#ifndef __GL3D_SCENE_H__
#define __GL3D_SCENE_H__

#include "gl3d.h"

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
class node;
class camera;
class root;

namespace detail {
  
//---------------------------------------------------------------------------------------------------------------------
struct transform
{
  vec3 pos;
  mat3 rot;

  transform() { }

  void set_rotation(float yawDeg, float pitchDeg, float rollDeg)
  { rot = mat3::rotate(yawDeg, vec3::unit_z()) * mat3::rotate(pitchDeg, vec3::unit_y()) * mat3::rotate(rollDeg, vec3::unit_x()); }

  vec3 rotation_euler() const
  { return vec3(degrees(atan2(rot[1], rot[0])), degrees(-asin(rot[2])), degrees(atan2(rot[5], rot[8]))); }
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
class node : public detail::ref_counted
{
public:
  typedef detail::ptr<node> ptr;

  node();

  virtual bool set_parent(node *parent);
  node *parent() const { return _parent; }

  root *scene_root() const { return _root; }

  const detail::transform &transform() const { return _trans; }

  void set_position(float x, float y, float z) { _trans.pos = vec3(x, y, z); }
  vec3 position() const { return _trans.pos; }

  void set_position_world(float x, float y, float z);
  vec3 position_world() const;

  void set_rotation(float yawDeg, float pitchDeg, float rollDeg) { _trans.set_rotation(yawDeg, pitchDeg, rollDeg); }
  vec3 rotation_euler() const { return _trans.rotation_euler(); }

protected:
  virtual ~node();

  node *_parent = nullptr;
  root *_root = nullptr;
  detail::transform _trans;
  box3 _aabb;
};

//---------------------------------------------------------------------------------------------------------------------
class camera : public node
{
public:
  typedef detail::ptr<camera> ptr;

  camera();

protected:
  virtual ~camera();

  mat4 _projection;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
class light : public node
{
public:
  typedef detail::ptr<light> ptr;

protected:
  light() { }
  virtual ~light() { }
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
class point_light : public detail::light
{
public:
  typedef detail::ptr<point_light> ptr;
};

//---------------------------------------------------------------------------------------------------------------------
class spot_light : public detail::light
{
public:
  typedef detail::ptr<spot_light> ptr;
};

//---------------------------------------------------------------------------------------------------------------------
class directional_light : public detail::light
{
public:
  typedef detail::ptr<directional_light> ptr;
};

//---------------------------------------------------------------------------------------------------------------------
class root : public node
{
public:
  typedef detail::ptr<root> ptr;

  static ptr create_default();

  root();

  bool set_parent(node *parent) override { return false; }

  bool set_main_camera(camera *node);
  camera *main_camera() const { return _main_camera; }
  
protected:
  virtual ~root();

  camera *_main_camera = nullptr;
};

}

#endif // __GL3D_SCENE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
#ifndef __GL3D_SCENE_H_IMPL__
#define __GL3D_SCENE_H_IMPL__

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
node::node()
{
  
}

//---------------------------------------------------------------------------------------------------------------------
node::~node()
{
  
}

//---------------------------------------------------------------------------------------------------------------------
bool node::set_parent(node *parent)
{
  if (parent != _parent)
  {
    
  }

  return true;
}

//---------------------------------------------------------------------------------------------------------------------
void node::set_position_world(float x, float y, float z)
{

}

//---------------------------------------------------------------------------------------------------------------------
vec3 node::position_world() const
{
  return vec3();
}

//---------------------------------------------------------------------------------------------------------------------
camera::camera()
  : node()
{

}

//---------------------------------------------------------------------------------------------------------------------
camera::~camera()
{

}

//---------------------------------------------------------------------------------------------------------------------
root::ptr root::create_default()
{
  root::ptr result = new root();

  camera::ptr cam = new camera();
  cam->set_parent(result);
  result->set_main_camera(cam);

  return result;
}

//---------------------------------------------------------------------------------------------------------------------
root::root()
  : node()
{
  
}

//---------------------------------------------------------------------------------------------------------------------
root::~root()
{
  
}

//---------------------------------------------------------------------------------------------------------------------
bool root::set_main_camera(camera *node)
{
  return true;
}

}

#endif // __GL3D_SCENE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
