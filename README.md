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

### gl3d_input.h
- Enums for key, mouse and gamepad devices
- Callback registration for tick & window events
- Simple access to main input device states:
  - Keyboard
  - Mouse
  - Gamepads
- Depends on gl3d_math.h

### gl3d_window.h
- Windowing library
- Creating windows with initialized OpenGL contexts quickly
- Capturing input events:
  - mouse events
  - mouse wheel events
  - keyboard events (key down, key up, key press)
  - window events (open, close, resize...)
  - joystick/gamepad events
- Frame timing functions
- Multiple windows support
- Depends on gl3d.h, gl3d_input.h

### gl3d_cmd_list.h
- **TODO**
- Records draw calls into a deferred command list

### gl3d_imgui.h
- **TODO**
- [dear imgui](https://github.com/ocornut/imgui) integration

### gl3d_scene.h
- **TODO**
- [ ] Simple scene tree management
- [ ] Forward rendering pipeline
- [ ] One global light
- [ ] RAW model loading
- [ ] OBJ model loading
- [ ] Texture loading (using [stb_image](https://github.com/nothings/stb))
