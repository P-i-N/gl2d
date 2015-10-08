#include <gl2d/gl2d.h>
#include <gl2d/glu2d.h>

gl2d::application app;

void on_event(const gl2d::event &e)
{
  switch (e.type)
  {
    case gl2d::event_type::tick:
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    break;

    case gl2d::event_type::mouse_move:
    {
      printf("mouse_move: %d %d\n", e.mouse.x, e.mouse.y);
    }
    break;

    case gl2d::event_type::resize:
    {
      printf("resize: %d %d\n", e.resize.width, e.resize.height);
    }
    break;

    default:
      break;
  }
}

int main()
{
  app.set_event_handler(on_event);
  auto id = app.window_open("Test platform_window", 800, 600);
  app.set_window_size(id, 400, 300);

  app.run();
  return 0;
}
