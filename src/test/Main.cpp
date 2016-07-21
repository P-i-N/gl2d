#define GL2D_IMPLEMENTATION
#include <gl2d/gl2d.h>
#include <gl2d/glu2d.h>

using namespace gl2d;

application app;

int main()
{
  auto id = app.window_open("Example", 800, 600);

  app.set_tick_handler([&](float delta)
  {
    text(4, 4, "Hello, world!");
  });

  app.run();
  return 0;
}
