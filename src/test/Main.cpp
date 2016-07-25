#define GL3D_IMPLEMENTATION
#include <gl3d/glu2d.h>

int main()
{
  using namespace gl3d;

  // Triangle geometry
  geometry::ptr geom = new geometry();
  auto vertices = geom->alloc_vertices(3);

  vertices->pos = vec3( 0,  1, -50);
  vertices->color = vec4::red();
  ++vertices;

  vertices->pos = vec3(-1, -1, -50);
  vertices->color = vec4::green();
  ++vertices;

  vertices->pos = vec3( 1, -1, -50);
  vertices->color = vec4::blue();
  ++vertices;

  auto id = app.window_open("Example", 400, 300);

  app.set_event_handler([&](const event &e)
  {
    if (e.type == event_type::paint)
    {
      auto size = app.get_window_size(e.window_id);
      float aspectRatio = static_cast<float>(size.x) / size.y;

      auto ctx = current_context3d;
      ctx->bind(geom);
      ctx->set_uniform(GL3D_UNIFORM_PROJECTION_MATRIX, mat4::perspective(60.0f, aspectRatio, 0.01f, 1000.0f));
      ctx->set_uniform(GL3D_UNIFORM_MODELVIEW_MATRIX, mat4());
      ctx->draw();
    }
  });
      
  app.run();
  return 0;
}
