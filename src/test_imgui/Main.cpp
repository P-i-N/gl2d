#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>
#include <gl3d/gl3d_imgui.h>

int main()
{
  using namespace gl3d;

  window_open("ImGui example", 400, 300);

  run();
  return 0;
}
