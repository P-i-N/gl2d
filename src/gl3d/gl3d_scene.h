#ifndef __GL3D_SCENE_H__
#define __GL3D_SCENE_H__

#include "gl3d.h"

namespace gl3d {

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

  void set_parent(node *parent);
  node *parent() const { return _parent; }

  const detail::transform &transform() const { return _trans; }

  void set_position(float x, float y, float z) { _trans.pos = vec3(x, y, z); }
  vec3 position() const { return _trans.pos; }

  void set_rotation(float yawDeg, float pitchDeg, float rollDeg) { _trans.set_rotation(yawDeg, pitchDeg, rollDeg); }
  vec3 rotation_euler() const { return _trans.rotation_euler(); }
    
protected:
  virtual ~node();

private:
  node *_parent = nullptr;
  detail::transform _trans;
  box3 _aabb;
};

//---------------------------------------------------------------------------------------------------------------------
class root : public node
{
public:
  typedef detail::ptr<root> ptr;

  root();

protected:
  virtual ~root();
};

//---------------------------------------------------------------------------------------------------------------------
class camera : public node
{
public:
  typedef detail::ptr<camera> ptr;
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
void node::set_parent(node *parent)
{
  if (parent != _parent)
  {
    
  }
}

//---------------------------------------------------------------------------------------------------------------------
root::root()
{
  
}

//---------------------------------------------------------------------------------------------------------------------
root::~root()
{
  
}

}

#endif // __GL3D_SCENE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
