#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>
#include <gl3d/gl3d_scene.h>

int main()
{
  using namespace gl3d;

  window_open("Simple 3D scene", 800, 600);

  // Create default scene with active camera
  auto scene = root::create_default();

  on_tick += [&]()
  {


  };

  run();
  return 0;
}
