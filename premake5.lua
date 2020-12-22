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
IncludeDir["GLAD"] = "%{wks.location}/PrehistoricEngine/vendor/GLAD/include"
IncludeDir["Vulkan"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/Vulkan"
IncludeDir["AL"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/AL"
IncludeDir["STB"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/stb"

include "PrehistoricEngine/vendor/GLFW"
include "PrehistoricEngine/vendor/GLAD"

project "PrehistoricEngine"
    location "PrehistoricEngine"
    kind "StaticLib" -- "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
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
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.AL}",
        "%{IncludeDir.STB}",
        "%{prj.location}/src/engine"
    }

    links
    {
        "%{wks.location}/PrehistoricEngine/Dependencies/lib/Vulkan/vulkan-1.lib",
        "%{wks.location}/PrehistoricEngine/Dependencies/lib/AL/OpenAL32.lib",
        "opengl32.lib",
        "GLFW",
        "GLAD"
    }
    
    filter "kind:SharedLib"
       defines { "PR_BUILD_DLL" }
       postbuildcommands { "{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Prehistoric" }

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
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        runtime "Release"
        optimize "on"

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
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.Vulkan}",
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
        
        -- defines
        -- {
        --     "PR_FAST_MATH",
        --     "PR_LOAD_DLL"
        -- }
    
    filter "configurations:Debug"
        defines "PR_DEBUG"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines "PR_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Distribution"
        defines "PR_DIST"
        runtime "Release"
        optimize "on"
    
