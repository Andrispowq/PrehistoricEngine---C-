workspace "PrehistoricEngine"
    architecture "x64"

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

project "PrehistoricEngine"
    location "PrehistoricEngine"
    kind "SharedLib"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "engine/prehistoric/core/util/Includes.hpp"
    pchsource "engine/prehistoric/core/util/Includes.cpp"

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
        "%{wks.location}/Dependencies/include/GLFW",
        "%{wks.location}/Dependencies/include/Vulkan",
        "%{wks.location}/Dependencies/include/GL",
        "%{wks.location}/Dependencies/include/AL",
        "%{wks.location}/Dependencies/include/stb",
        "%{prj.location}/src"
    }

    links
	{
        "%{wks.location}/Dependencies/lib/GLFW/glfw3.lib",
        "%{wks.location}/Dependencies/lib/Vulkan/vulkan-1.lib",
        "%{wks.location}/Dependencies/lib/GL/glew32s.lib",
        "%{wks.location}/Dependencies/lib/AL/OpenAL32.lib",
        "opengl32.lib"
	}

    filter "system:windows"
        cppdialect "C++17"
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
        "%{wks.location}/Dependencies/include/AL",
        "%{wks.location}/Dependencies/include/GL",
        "%{wks.location}/Dependencies/include/stb",
        "%{wks.location}/Dependencies/include/Vulkan",
        "%{wks.location}/Dependencies/include/GLFW",
        "%{prj.location}/src",
        "PrehistoricEngine/src"
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
            "PR_LOAD_DLL",
            "PR_FAST_MATH",
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
    
