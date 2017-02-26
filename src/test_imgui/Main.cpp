#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

#include "imgui.h"
#include <gl3d/gl3d_imgui.h>

int main()
{
  using namespace gl3d;

  window_open("ImGui example", 800, 600);

  on_tick += [&]()
  {
    
  
  };

  run();
  return 0;
}
