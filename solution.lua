Projects =
{
  -- gl3d
  {
    dir = "src/gl3d",
    type = "static",
    defines = { "GL3D_IMPLEMENTATION" }
  },

  -- test
  {
    dir = "src/test",
    type = "console",
  },
  
  -- test_imgui
  {
    dir = "src/test_imgui",
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
