include "scripts/premake-cmake.lua"

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

    filter "system:windows"
        defines { "NOMINMAX" }
    filter "action:vs2022"
        toolset "msc-v145"
    filter {}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

local monoRuntimeCommands = {
    '{COPY} "%{wks.location}/PrehistoricEngine/Dependencies/lib/Mono/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
    'robocopy "%{wks.location}/PrehistoricEngine/Dependencies/lib/Mono/runtime" "%{cfg.targetdir}/Mono" /E /R:2 /W:1 /NFL /NDL /NJH /NJS /NP',
    'if errorlevel 8 exit /b 1',
    'exit /b 0'
}

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/PrehistoricEngine/vendor/GLFW/include"
IncludeDir["GLAD"] = "%{wks.location}/PrehistoricEngine/vendor/GLAD/include"
IncludeDir["ImGUI"] = "%{wks.location}/PrehistoricEngine/vendor/ImGUI"
IncludeDir["OpenAL"] = "%{wks.location}/PrehistoricEngine/vendor/OpenAL-Soft/include"
IncludeDir["Vulkan"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/Vulkan"
IncludeDir["STB"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/stb"
IncludeDir["tinyobj"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/tinyobjloader"
IncludeDir["tinygltf"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/tinygltfloader"
IncludeDir["nlohmann_json"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/nlohmann_json"
IncludeDir["Mono"] = "%{wks.location}/PrehistoricEngine/Dependencies/include/Mono"

group "Dependencies"
    include "PrehistoricEngine/vendor/GLFW"
    include "PrehistoricEngine/vendor/GLAD"
    include "PrehistoricEngine/vendor/ImGUI"
    include "PrehistoricEngine/vendor/OpenAL-Soft"
group ""

project "PrehistoricEngine"
    location "PrehistoricEngine"
    kind "StaticLib"
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
        "%{IncludeDir.ImGUI}",
        "%{IncludeDir.OpenAL}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.nlohmann_json}",
        "%{IncludeDir.Mono}",
        "%{prj.location}/src/engine"
    }

    links
    {
        "GLFW",
        "GLAD",
        "ImGUI",
        "OpenAL"
    }

    filter "system:windows"
        links
        {
            "%{wks.location}/PrehistoricEngine/Dependencies/lib/Mono/mono-2.0-sgen.lib",
            "%{wks.location}/PrehistoricEngine/Dependencies/lib/Vulkan/vulkan-1.lib",
            "%{wks.location}/PrehistoricEngine/Dependencies/lib/AL/OpenAL32.lib",
            "opengl32.lib"
        }
    filter {}

    filter "system:macosx"
        files { "PrehistoricEngine/src/engine/platform/macos/**.mm" }
        links { "-framework Cocoa", "-framework UniformTypeIdentifiers", "-framework OpenGL", "vulkan" }
        if os.getenv("VULKAN_SDK") then
            libdirs { path.join(os.getenv("VULKAN_SDK"), "lib") }
        end

    filter "system:not macosx"
        removefiles { "PrehistoricEngine/src/engine/platform/macos/**" }

    filter "system:not windows"
        -- Window and input are shared GLFW implementations despite their names.
        removefiles
        {
            "PrehistoricEngine/src/engine/platform/windows/WindowsPlatformUtils.h",
            "PrehistoricEngine/src/engine/prehistoric/core/engines/ScriptEngine.*",
            "PrehistoricEngine/src/engine/prehistoric/core/node/component/scripting/**"
        }

    filter "files:**.mm"
        flags { "NoPCH" }
    filter {}

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
        
group "Sandbox projects"

project "PrehistoricEditor"
    location "PrehistoricEditor"
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
        "%{IncludeDir.ImGUI}",
        "%{IncludeDir.OpenAL}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.AudioFile}",
        "%{IncludeDir.nlohmann_json}",
        "%{IncludeDir.Mono}",
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
        postbuildcommands(monoRuntimeCommands)

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
        "%{IncludeDir.ImGUI}",
        "%{IncludeDir.OpenAL}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.nlohmann_json}",
        "%{IncludeDir.Mono}",
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
        postbuildcommands(monoRuntimeCommands)

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

group ""
-- Compile GLFW's native Cocoa backend when generating on macOS.
project "GLFW"
    filter "system:macosx"
        files {
            "PrehistoricEngine/vendor/GLFW/src/cocoa_*.m",
            "PrehistoricEngine/vendor/GLFW/src/cocoa_time.c",
            "PrehistoricEngine/vendor/GLFW/src/posix_thread.c",
            "PrehistoricEngine/vendor/GLFW/src/nsgl_context.m",
            "PrehistoricEngine/vendor/GLFW/src/egl_context.c",
            "PrehistoricEngine/vendor/GLFW/src/osmesa_context.c"
        }
        defines { "_GLFW_COCOA" }
        links { "-framework Cocoa", "-framework IOKit", "-framework CoreFoundation" }
    filter {}
