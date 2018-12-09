# **WORK IN PROGRESS**, use at your own risk or rather don't...

- [x] multiple windows support
- [ ] fullscreen support
- [x] toggle fullscreen with `Alt+Enter`
- [x] frame limiter
- [ ] shader hot reload
- [ ] make `gl3d::shader_code` API better (constructors, `::valid()` method, etc.)
- [x] timestamped log messages
- [ ] support different texture types
  - [ ] TEXTURE_1D
  - [ ] TEXTURE_2D
  - [ ] TEXTURE_3D
  - [ ] TEXTURE_2D_ARRAY
  - [ ] TEXTURE_CUBE_MAP
  - [ ] TEXTURE_CUBE_MAP_ARRAY
  - [ ] multisampling
- [ ] bindless textures
- [ ] blend state: `gl3d::blend_state`
- [x] depth stencil state: `gl3d::depth_stencil_state`
- [x] rasterizer state: `gl3d::rasterizer_state`
- [ ] command queue: `gl3d::cmd_queue`
  - [x] immediate mode
  - [x] deferred mode
  - [x] serialized buffer updates
  - [ ] serialized texture updates
  - [ ] serialized uniform block updates
  - [x] correct VAO handling
  - [ ] erase unused VAOs after while (300 frames/5 seconds?)
  - [ ] using custom vertex attributes
  - [ ] support (multiple) render targets
- [ ] asynchronous upload context: `gl3d::detail::async_upload_context`
  - [ ] buffer updates
  - [ ] texture updates
- [ ] space navigator support
- [x] gamepads with raw input
- [ ] load BMF fonts from files
- [ ] simple renderer with emulated immediate mode: `gl3d::quick_draw`
  - [ ] emulate good old `glBegin` / `glEnd` as efficiently as possible
  - [ ] OMG...
- [ ] simple scene API
  - [ ] node system/hierarchy
  - [ ] fast BVH partitioning
  - [ ] frustum culling
  - [ ] shadow mapping
  - [ ] hybrid clustered forward pipeline?
  - [ ] PBR?
- [ ] ImGui support with multiple viewports

---

# **G L** 3 D
Collection of small header-only libraries for writing simple OpenGL applications, tools or demos. Currently compiles and runs on Windows and Visual Studio only.

+ [Example 1: Open empty window](#example1)
+ [Example 2: Clear window with a color every frame](#example2)
+ [Example 3: Capture window and input device events](#example3)

# Library parts:

### gl3d_base.h
- utility functions shared between other parts
- simple logger - `gl3d::log`
- simple virtual filesystem - `gl3d::vfs`
- button, axis & key enums for input devices:
  - `gl3d::key`
  - `gl3d::mouse_button`
  - `gl3d::gamepad_button`
  - `gl3d::gamepad_axis`
  - `gl3d::space_navigator_button`
- global input device states:
  - `gl3d::keyboard`
  - `gl3d::mouse`
  - `gl3d::gamepad[]`
  - `gl3d::space_navigator` **(WIP)**

### gl3d_math.h
- vector and matrix classes (vec2, vec3, mat4, ...)
- math utility functions (dot, cross, normalize, ...)
- **not SSE optimized**

### gl3d.h
- TODO

### gl3d_window.h
- TODO

### gl3d_2d.h
- TODO

### gl3d_scene.h
- TODO

---

<a id="example1"></a>
## Example 1: Open empty window

```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

using namespace gl3d;

int main()
{
  window::create( "Main Window", { 1280, 800 } );

  run();
  return 0;
}
```

---

<a id="example2"></a>
## Example 2: Clear window with a color every frame

```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

using namespace gl3d;

int main()
{
  window::create( "Main Window", { 1280, 800 } );

  on_tick += [&]()
  {
    auto w = window::from_id( 0 );
    w->context()->clear_color( { 0.125f, 0.25f, 0.5f, 1.0f } );
  };

  run();
  return 0;
}
```

---

<a id="example3"></a>
## Example 3: Capture window and input device events

```cpp
#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_window.h>

using namespace gl3d;

int main()
{
  window::create( "Main Window", { 1280, 800 } );

  on_window_event += [&](window_event &e) -> bool
  {
    // Get concrete window instance from window_id
    auto w = window::from_id(e.window_id);

    switch (e.event_type)
    {
      case window_event::type::resize:
        // ...
        break;

      case window_event::type::close:
        // ...
        break;

      /// etc...
    }

    // Returning false means that the current event was NOT consumed by this
    // event handler and can be passed to next handler in the callback chain
    return false;
  };

  on_input_event += [&](input_event &e) -> bool
  {
    // Get concrete window instance from window_id. Be careful there - some input
    // events are "global" and does NOT have a source window:
    // - gamepad or space navigator events
    auto w = window::from_id(e.window_id);

    // Returning true/false indicates event consumption (same as with on_window_event)
    return false;
  };

  on_tick += [&]()
  {
    auto w = window::from_id( 0 );
    w->context()->clear_color( { 0.125f, 0.25f, 0.5f, 1.0f } );
  };

  run();
  return 0;
}
```
