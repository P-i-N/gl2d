#ifndef __GL3D_SCENE_H__
#define __GL3D_SCENE_H__

#include "gl3d.h"

namespace gl3d {

class node : detail::ref_counted
{
public:
  typedef detail::ptr<node> ptr;

  node();

  void set_parent(node *parent);
  node *parent() const { return _parent; }

protected:
  virtual ~node();

private:
  node *_parent = nullptr;
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

}

#endif // __GL3D_SCENE_H_IMPL__
#endif // GL3D_IMPLEMENTATION
