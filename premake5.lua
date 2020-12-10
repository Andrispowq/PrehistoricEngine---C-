workspace "PrehistoricEngine"
    architecture "x64"
    startproject "Prehistoric"

    configurations
    {
        "Debug",
        "Release",
        "Distribution"
    }

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/PrehistoricEngine/vendor/GLFW/include"
IncludeDir["Vulkan"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/Vulkan"
IncludeDir["GLEW"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/GLEW"
IncludeDir["AL"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/AL"
IncludeDir["STB"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/stb"

include "PrehistoricEngine/vendor/GLFW"

project "PrehistoricEngine"
    location "PrehistoricEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Includes.hpp"
    pchsource "%{prj.location}/src/engine/Includes.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

    includedirs
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.GLEW}",
        "%{IncludeDir.AL}",
        "%{IncludeDir.STB}",
        "%{prj.location}/src/engine"
    }

    links
    {
        "%{wks.location}/PrehistoricEngine/Dependencies/lib/Vulkan/vulkan-1.lib",
        "%{wks.location}/PrehistoricEngine/Dependencies/lib/GLEW/glew32s.lib",
        "%{wks.location}/PrehistoricEngine/Dependencies/lib/AL/OpenAL32.lib",
        "opengl32.lib",
        "GLFW"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "PR_FAST_MATH",
            "GLEW_STATIC"
        }
    
    filter "configurations:Debug"
        defines "PR_DEBUG"
        runtime "Debug"
        symbols "on"
        buildoptions { "/MTd" }
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions { "/MT" }
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        runtime "Release"
        optimize "on"
        buildoptions { "/MT" }

project "Prehistoric"
    location "Prehistoric"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.GLEW}",
        "%{IncludeDir.AL}",
        "%{IncludeDir.STB}",
        "%{prj.location}/src",
        "PrehistoricEngine/src/engine"
    }

    defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

    links
    {
        "PrehistoricEngine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "PR_FAST_MATH"
        }
    
    filter "configurations:Debug"
        defines "PR_DEBUG"
        runtime "Debug"
        symbols "on"
        buildoptions { "/MTd" }
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions { "/MT" }
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        runtime "Release"
        optimize "on"
        buildoptions { "/MT" }
    
