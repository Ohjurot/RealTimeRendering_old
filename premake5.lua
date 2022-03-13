-- Include conan gennerate script
include("conanbuildinfo.premake.lua")

-- Main Workspace
workspace "RealTimeRendering"
    -- Import conan gennerate config
    conan_basic_setup()

    -- Project
    project "RealTimeRendering"
        kind "WindowedApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"
        objdir "bin/%{cfg.buildcfg}/obj/"
        location "RealTimeRendering"
        debugdir "app"

        -- Custom includes and libs
        includedirs { "RealTimeRendering", "vendor/dxc" }
        libdirs { "vendor/bin" }
        links { "dxcompiler.lib", "d3d12.lib", "dxgi.lib", "Pathcch.lib" }

        linkoptions { conan_exelinkflags }

        files { "**.h", "**.cpp" }

        filter "configurations:Debug"
        defines { "DEBUG", "RTR_DEBUG" }
        symbols "On"

        filter "configurations:Release"
        defines { "NDEBUG", "RTR_RELEASE" }
        optimize "On"
