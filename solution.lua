Projects =
{
  -- gl3d
  {
    dir = "src/gl3d",
    type = "static",
    defines = { "GL3D_IMPLEMENTATION" }
  },

  -- gl3d_parts
  {
    dir = "src/gl3d_parts",
    type = "headeronly",
    defines = { "GL3D_IMPLEMENTATION" }
  },

  -- test
  {
    dir = "src/test",
    type = "console",
  },
  
  -- arc
  {
    dir = "src/arc",
    type = "console",
  },

  -- test_imgui
  {
    dir = "src/test_imgui",
    type = "console",
  },

  -- test_immediate
  {
    dir = "src/test_immediate",
    type = "console",
  },

  -- fontconv
  {
    dir = "src/fontconv",
    language = "C#",
    type = "console",
    links = { "System.Drawing" }
  }
}
