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
IncludeDir["GLFW"] = "%{wks.location}/Dependencies/include/GLFW"
IncludeDir["Vulkan"] = "%{wks.location}/Dependencies/include/Vulkan"
IncludeDir["GLEW"] = "%{wks.location}/Dependencies/include/GLEW"
IncludeDir["AL"] = "%{wks.location}/Dependencies/include/AL"
IncludeDir["STB"] = "%{wks.location}/Dependencies/include/stb"

project "PrehistoricEngine"
    location "PrehistoricEngine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    
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
        "%{wks.location}/Dependencies/lib/GLFW/glfw3.lib",
        "%{wks.location}/Dependencies/lib/Vulkan/vulkan-1.lib",
        "%{wks.location}/Dependencies/lib/GLEW/glew32s.lib",
        "%{wks.location}/Dependencies/lib/AL/OpenAL32.lib",
        "opengl32.lib"
	}

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "PR_BUILD_DLL",
            "PR_FAST_MATH",
            "GLEW_STATIC",
            "_DEBUG",
            "_CONSOLE",
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Prehistoric")
        }
    
    filter "configurations:Debug"
        defines "PR_DEBUG"
        symbols "On"
        buildoptions { "/MDd" }
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        optimize "On"
        buildoptions { "/MD" }
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        optimize "On"
        buildoptions { "/MD" }

project "Prehistoric"
    location "Prehistoric"
    kind "ConsoleApp"
    language "C++"
    
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
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "PR_FAST_MATH"
        }
    
    filter "configurations:Debug"
        defines "PR_DEBUG"
        symbols "On"
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        optimize "On"
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        optimize "On"
    
