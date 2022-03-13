-- Include conan gennerate script
include("conanbuildinfo.premake.lua")

-- Main Workspace
workspace "RealTimeRendering"
    -- Import conan gennerate config
    conan_basic_setup()

    -- Project
    project "RealTimeRendering"
        kind "ConsoleApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"
        objdir "bin/%{cfg.buildcfg}/obj/"
        location "RealTimeRendering"
        includedirs { "RealTimeRendering" }
        debugdir "app"

        linkoptions { conan_exelinkflags }

        files { "**.h", "**.cpp" }

        filter "configurations:Debug"
        defines { "DEBUG", "RTR_DEBUG" }
        symbols "On"

        filter "configurations:Release"
        defines { "NDEBUG", "RTR_RELEASE" }
        optimize "On"
