#include <gl2d/gl2d.h>
#include <gl2d/glu2d.h>

gl2d::application app;

int main()
{
  auto id = app.window_open("Test platform_window", 800, 600);

  app.set_event_handler([&](const gl2d::event &e)
  {
    if (e.type == gl2d::event_type::tick)
    {
      gl2d::text(4, 4, "Hello, world!");
    }
  });

  app.run();
  return 0;
}
