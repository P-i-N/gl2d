# gl3d
Collection of small header-only libraries for writing simple OpenGL applications, tools or demos. Currently compiles and runs on Windows and Visual Studio only, future support for Linux is possible.

### gl3d_math.h
- Vector and matrix classes (vec2, vec3, mat4, ...)
- Math utility functions (dot, cross, normalize, ...)
- Not SSE optimized, simple implementation

### gl3d.h
- Main OpenGL library layer
- OpenGL 3.0+ support
- Automatically finds extension functions (glCreateShader, glUniform...)
- Wrappers for basic OpenGL objects & concepts:
  - buffers
  - geometries with easy VAO layout definitions
  - shaders and programs (techniques) with preprocessor macros
  - compute shaders
  - simple uniform binding
  - textures, texture arrays, cubemaps
  - render targets (FBO)
- Depends on gl3d_math.h

### gl3d_2d.h
- 2D drawing library
- Text rendering with embedded fonts (no need for external loading)
- Supports colored strings with '^' marks
- Depends on gl3d.h

### gl3d_win32.h
- Windowing library
- Creating windows with initialized OpenGL contexts quickly
- Capturing input events:
  - mouse events
  - mouse wheel events
  - keyboard events (key down, key up, key press)
  - window events (open, close, resize...)
  - joystick/gamepad events (TODO)
- Frame timing functions
- Multiple windows support
- Depends on gl3d_2d.h

### gl3d_ui.h
TODO

### gl3d_scene.h
TODO

-------------------------------------------------------------------------------

### Example 1 - open window
```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

using namespace gl3d;

int main()
{
  // Open main window
  window_open("Example", 400, 300);

  // Start main application loop
  run();

  return 0;
}
```

### Example 2 - Hello, world!
```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

using namespace gl3d;

int main()
{
  window_open("Example", 400, 300);

  // Tick handler called once every frame
  tick_handler = [&]()
  {
    auto ctx = current_context2d;
    ctx->text(10, 10, "Hello, world!");
  };

  run();
  return 0;
}
```

### Example 3 - rotating triangle
```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

using namespace gl3d;

int main()
{
  // Triangle geometry
  geometry::ptr geom = new geometry();

  // Allocate 3 vertices
  auto vertices = geom->alloc_vertices(3);

  // Top vertex
  vertices->pos = vec3(0, 1, 0);
  vertices->color = vec4::red();
  ++vertices;

  // Left vertex
  vertices->pos = vec3(-1, -1, 0);
  vertices->color = vec4::green();
  ++vertices;

  // Right vertex
  vertices->pos = vec3(1, -1, 0);
  vertices->color = vec4::blue();
  ++vertices;

  window_open("Example", 400, 300);

  // Event handler for capturing all application events
  event_handler = [&](const event &e)
  {
    // Paint event called for every window before end of the frame
    if (e.type == event_type::paint)
    {
      auto size = get_window_size(e.window_id);
      float aspectRatio = static_cast<float>(size.x) / size.y;

      auto ctx = current_context3d;
      ctx->bind(geom);
      ctx->set_uniform(GL3D_UNIFORM_PROJECTION_MATRIX, mat4::perspective(120.0f, aspectRatio, 0.01f, 1000.0f));
      ctx->set_uniform(GL3D_UNIFORM_MODELVIEW_MATRIX, mat4::look_at(15.0f * sin(time()), 0.0f, 15.0f * cos(time()), 0.0f, 0.0f, 0.0f).invert());
      ctx->draw();
    }
  };

  tick_handler = [&]()
  {
    auto ctx = current_context2d;
    ctx->text(10, 10, "FPS: ^9%.3f ms", 1.0f / delta());
  };

  run();
  return 0;
}
```
