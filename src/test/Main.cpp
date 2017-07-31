#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

#include <chrono>

int main()
{
  using namespace gl3d;
  
  // Triangle geometry
  geometry::ptr geom = new geometry();
  auto vertices = geom->alloc_vertices(3);

  vertices->pos = vec3( 0,  1, 0);
  vertices->color = vec4::red();
  ++vertices;

  vertices->pos = vec3(-1, -1, 0);
  vertices->color = vec4::green();
  ++vertices;

  vertices->pos = vec3( 1, -1, 0);
  vertices->color = vec4::blue();
  ++vertices;

  window_open("Example", 400, 300);
  
  on_event += [&](event &e)
  {
    if (e.canceled)
      return;

    if (e.type == event_type::paint)
    {
      auto size = get_window_size(e.window_id);
      float aspectRatio = static_cast<float>(size.x) / size.y;

      auto ctx = state.ctx3d;
      ctx->bind(geom);
      ctx->set_uniform(GL3D_UNIFORM_PROJECTION_MATRIX, mat4::perspective(60.0f, aspectRatio, 0.01f, 1000.0f));
      ctx->set_uniform(GL3D_UNIFORM_MODELVIEW_MATRIX, mat4::look_at(5.0f * sin(state.time), 2.0f, 5.0f * cos(state.time), 0.0f, 0.0f, 0.0f).invert());
      ctx->draw();
    }
  };

  on_tick += [&]()
  {
    auto ctx = state.ctx2d;
    int y = 0;
    
    if (gamepad[0][gamepad_button::a]) ctx->texti(16, y += 16, "^AButton A");
    if (gamepad[0][gamepad_button::b]) ctx->texti(16, y += 16, "^CButton B");
    if (gamepad[0][gamepad_button::x]) ctx->texti(16, y += 16, "^9Button X");
    if (gamepad[0][gamepad_button::y]) ctx->texti(16, y += 16, "^EButton Y");
    if (gamepad[0][gamepad_button::thumb_left]) ctx->texti(16, y += 16, "Thumb Left");
    if (gamepad[0][gamepad_button::thumb_right]) ctx->texti(16, y += 16, "Thumb Right");
    if (gamepad[0][gamepad_button::shoulder_left]) ctx->texti(16, y += 16, "Shoulder Left");
    if (gamepad[0][gamepad_button::shoulder_right]) ctx->texti(16, y += 16, "Shoulder Right");
    if (gamepad[0][gamepad_button::up]) ctx->texti(16, y += 16, "UP");
    if (gamepad[0][gamepad_button::down]) ctx->texti(16, y += 16, "DOWN");
    if (gamepad[0][gamepad_button::left]) ctx->texti(16, y += 16, "LEFT");
    if (gamepad[0][gamepad_button::right]) ctx->texti(16, y += 16, "RIGHT");

    auto size = get_window_size();
    float cx = size.x / 2.0f - size.x / 4.0f;
    float r = size.x / 8.0f;
    float cy = size.y / 2.0f + r * gamepad[0].axis_x[3];;

    ctx->line(cx, cy, cx + r * gamepad[0].axis_x[1], cy - r * gamepad[0].axis_y[1]);

    cx = size.x / 2.0f + size.x / 4.0f;
    cy = size.y / 2.0f + r * gamepad[0].axis_x[4];;
    ctx->line(cx, cy, cx + r * gamepad[0].axis_x[2], cy - r * gamepad[0].axis_y[2]);

    ctx->texti(8, 8, "Hello, world!");
  };

  run();
  return 0;
}
