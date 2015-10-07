#include <gl2d/gl2d.h>
#include <gl2d/glu2d.h>

gl2d::application app;

int main()
{
  app.open_window("Test platform_window", 800, 600);
  app.run();
  return 0;
}
