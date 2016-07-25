-- Define list of projects in this solution. Supported parameters and default values:
--
--     dir - required parameter, directory containing source files
--
--     type ("lib") - console application ("console")
--                  - windowed application ("windowed")
--                  - console/windowed in debug/release ("app")
--                  - library or DLL, depends on selected configuration ("lib")
--                  - static library ("static")
--                  - dynamic library ("dynamic")
--
--     language ("C++") - project language
--
--     name ("") - optional project name
--
--     shared_macro ("BUILDING") - macro defined when building shared library
--
--     links ({ }) - linked libraries, projects
--
--     windows_links ({ }) - linked libraries on Windows platform
--
--     linux_links ({ }) - linked libraries on Linux platform
--
--     include ({ }) - include directories
--
--     defines ({ }) - project specific defines
--
--     configure_callback (nil) - function called when this project is being configured
--
-----------------------------------------------------------------------------------------------------------------------
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
  }
}

-- If this is empty, name of current directory will be used instead
SolutionName = ""

-- If this is nil, current directory and sources directory ("src" by default) will be used
IncludeDirs = { ".", "src" }

-----------------------------------------------------------------------------------------------------------------------

if SolutionName == "" then
  SolutionName = path.getname(os.getcwd())
end

-----------------------------------------------------------------------------------------------------------------------

-- Generate named project using defined params
function generateProject(params)

  local name = group().current.filename
  local projectGroup = group().group
  
  print("Generating project: " .. projectGroup .. "/" .. name)
  
  function getParam(key, default)
    local result = params[key]
    if result == nil then return default end
    return result
  end
  
  local _type = getParam("type", "lib")
  local _language = getParam("language", "C++")
  local _include_current = getParam("include_current", false)
  local _name = getParam("name", "")
  local _shared_macro = getParam("shared_macro", "BUILDING")
  local _links = getParam("links", { })
  local _windows_links = getParam("windows_links", { })
  local _linux_links = getParam("linux_links", { })
  local _include = getParam("include", { })
  local _defines = getParam("defines", { })
  local _configure_callback = getParam("configure_callback", nil)

  language(_language)
  
  if _type == "lib" then
    filter { "configurations:Static*" }
      kind "StaticLib"
    filter { "configurations:DLL*" }
      kind "SharedLib"
      defines { _shared_macro }
    filter { }
  elseif _type == "console" then
    kind "ConsoleApp"
  elseif _type == "windowed" then
    kind "WindowedApp"
  elseif _type == "static" then
    kind "StaticLib"
  elseif _type == "dynamic" then
    kind "SharedLib"
    defines { _shared_macro }
  elseif _type == "app" then
    filter { "configurations:*Debug" }
      kind "ConsoleApp"
    filter { "configurations:*Release" }
      kind "WindowedApp"
    filter { }
  end
  
  files { "**.c", "**.cc", "**.cpp", "**.h", "**.hpp", "**.inl" }
  
  if _name ~= "" then
    targetname(_name)
  end
  
  links(_links)
  includedirs(_include)
  defines(_defines)
  
  filter { "system:windows" }
    links(_windows_links)
    
  filter { "system:linux" }
    links(_linux_links)
    
  filter { }
  
  if _configure_callback ~= nil then
    _configure_callback(params)
  end
  
end

-----------------------------------------------------------------------------------------------------------------------

solution(SolutionName)
  configurations { "Debug", "Release" }
  platforms { "64-bit" }
  location(path.join(".build", _ACTION))

filter { "platforms:64*" }
  architecture "x86_64"
  
filter { "configurations:*Debug" }
  defines { "_DEBUG", "DEBUG" }
  flags { "Symbols" }
  targetsuffix "_debug"
  
filter { "configurations:*Release" }
  defines { "_NDEBUG", "NDEBUG" }
  flags { "Symbols" }
  optimize "On"

filter { "system:windows", "platforms:64*" }
  defines { "_WIN64", "WIN64" }
  
filter { "system:windows" }
  defines { "_WIN32", "WIN32", "_CRT_SECURE_NO_WARNINGS", "_WIN32_WINNT=0x0601", "WINVER=0x0601", "NTDDI_VERSION=0x06010000" }
  flags { "NoMinimalRebuild", "MultiProcessorCompile" }
  buildoptions { '/wd"4503"' }
  
  if _ACTION == "vs2013" then
    defines { "_MSC_VER=1800" }
  elseif _ACTION == "vs2015" then
    defines { "_MSC_VER=1900" }

    filter { "system:windows", "configurations:DLL Debug" }
      links { "ucrtd.lib", "vcruntimed.lib", "msvcrtd.lib" }

    filter { "system:windows", "configurations:DLL Release" }
      links { "ucrt.lib", "vcruntime.lib", "msvcrt.lib" }
      
    filter { }
  end
  
filter { }
  includedirs(IncludeDirs)
  targetdir ".bin/%{cfg.buildcfg}"
  
-- Get current directory
cwd = os.getcwd()

-- Set default empty group
group ""
  
-- Enumerate all projects in global Projects table
for k, Project in pairs(Projects) do
  if Project.dir ~= nil then
    ok, err = os.chdir(Project.dir)
    
    if ok == true then
    
      -- Set project name
      local name = k
      
      if type(name) == "number" then
        name = Project.name
      end
      
      if name == nil then
        -- If name was not specified, create it from project directory name
        name = path.getname(Project.dir)
      end
      
      -- Project group based on parent project directory name
      group(path.getdirectory(Project.dir))
      
      -- Set premake project context
      project(name)
      
      -- Generate project information
      generateProject(Project)
      
      -- Change directory back
      os.chdir(cwd)
      
    else
      print("Could not change directory to: " .. Project.dir)
    end
  else
    print("Project does not have a directory specified!")
  end
end
