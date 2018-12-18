Projects =
{
  -- gl3d.dll
  {
    dir = "src/gl3d",
    type = "dynamic",
    defines = { "GL3D_IMPLEMENTATION", "GL3D_DYNAMIC" }
  },

  -- testbench
  {
    dir = "tests/testbench",
    includes = { "src" },
    type = "console",
    links = { "gl3d" }
  },

  -- fontconv
  {
    dir = "tools/fontconv",
    language = "C#",
    type = "console",
    links = { "System.Drawing" }
  }
}
