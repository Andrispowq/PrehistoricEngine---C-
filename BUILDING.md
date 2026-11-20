# Using the engine

If you wish to use the engine, follow these steps:
- Clone the repository into a folder on your computer with the following command: `git clone --recursive https://github.com/Andrispowq/PrehistoricEngine---C-`
- Create the solution files: the build system currently supports Visual Studio (version 2017 and 2019)
  - Building the Visual Studio solution and project files: execute the `build_windows_vs2017.bat`, or `build_windows_vs2019.bat` file
  - Building for another Visual Studio version: execute the following command: `call vendor\bin\premake\premake5.exe vs<version>` from the repository's root folder (PrehistoricEngine---C-)
  - Building for another development environment: while the Prehistoric Engine is officially untested on other environments, Premake 5.0 supports environments other than Visual Studio. For the whole list, use the command `call vendor\bin\premake\premake5.exe --help` from the repository's root folder (PrehistoricEngine---C-)
- If you've followed the steps, built the project with Visual Studio, but the app won't launch (it will most likely say "The application was unable to start correctly (0xc00007b). Click OK to close the application"), you must follow these steps:
  - Navigate to the repository's root folder (PrehistoricEngine---C-), and go under `PrehistoricEngine/Dependencies/redist` and run the file called `oalinst.exe`, which will install the OpenAL 1.1 SDK, as the engine depends on the SDK being installed

## CLion / CMake (macOS build investigation)

Install a native Premake 5, CMake (3.21+ for the v3 presets), Ninja, and the Apple
command-line developer tools. The checked-in `premake5.exe` is Windows-only.
The generator was verified with Premake 5.0.0-beta8 and AppleClang 17 on arm64.

From the repository root:

```sh
premake5 cmake
cmake --preset debug
cmake --build --preset debug --parallel 4
```

`premake5 cmake` generates the root `CMakeLists.txt` from the existing Premake
projects. It exports all seven main targets, includes, configuration-specific defines,
static-library link dependencies, and the engine precompiled header. GLFW also
gets its Cocoa sources/frameworks on macOS. Debug, Release, and Distribution
presets are supplied. CMake uses the compiler default architecture unless explicitly selected; use the
`macos-arm64` preset below on Apple Silicon, especially with an Intel CMake running
under Rosetta. The old workspace's x64 setting is not forced by the exporter.

In CLion, open the root `CMakeLists.txt` **as a project**, then select the Debug
preset/profile and allow CMake to reload. If this folder is already open as a
plain source directory, reopen it through that CMake file. The run working
directory should be `$PROJECT_DIR$/Prehistoric` or
`$PROJECT_DIR$/PrehistoricEditor`, respectively, because resource paths are relative.

The exporter is deliberately repository-specific: source membership and links
use the Debug configuration (currently identical across configurations), and
platform filters are baked for the generating host. Regenerate after changing
Premake files, adding source files, or changing host OS. Generated CMake output,
build directories, and local IDE settings are ignored by Git. The existing
Visual Studio actions remain available. The exporter does not translate arbitrary
Premake post-build commands or deployment rules; deployment must be configured separately.

**The macOS application targets build; full renderer support is still in progress.**
[MACOS_PORT.md](MACOS_PORT.md) records the original port investigation and may
include issues already resolved. To build just the core dependencies:

```sh
cmake --build --preset debug --target GLFW GLAD ImGUI --parallel 4
```

## Native OpenAL on Apple Silicon

The macOS CMake path now fetches the complete upstream OpenAL Soft 1.24.2 source
release with a pinned SHA-256. Its own CMake checks generate the correct config
and build a static library with CoreAudio support. The Windows vendor snapshot
and its Premake build are retained for Windows. The first configure of each build
directory requires network access; subsequent builds reuse the downloaded source.
No global installation is needed.

```sh
premake5 cmake
cmake --preset macos-arm64
cmake --build --preset macos-arm64 --target OpenALSmoke --parallel 4
ALSOFT_DRIVERS=null ./build/macos-arm64/bin/OpenALSmoke
```

In CLion, select the `macos-arm64` CMake preset/profile and reload CMake. Build
`OpenAL` for the library alone, or `OpenALSmoke` for the standalone API/link check.
The smoke check uses the silent Null backend when run with the environment
variable above; it does not verify output through physical speakers.

The generated configuration is
`build/macos-arm64/_deps/openal_soft-build/config.h`. On Apple Silicon,
`HAVE_INTRIN_H` is undefined and the configure summary lists CoreAudio and Neon.
Do not edit this generated file or the old vendor configuration to fix macOS.
The integration lives in `scripts/native-openal.cmake` and is included by the
Premake CMake exporter. The engine and applications inherit matching upstream
OpenAL headers and static-link requirements from its target.

For an offline source checkout, pass
`-DFETCHCONTENT_SOURCE_DIR_OPENAL_SOFT=/absolute/path/to/openal-soft-1.24.2`
at configure time. The full engine still needs the other changes in MACOS_PORT.md.

## Vulkan compute on macOS

The compute pipeline and `ResetBindings` were brought over from `beta-features`
(`8571251`) and adapted to the current macOS branch. The asset factory now creates
`VKComputePipeline` for Vulkan compute requests. Compute descriptors bind at the
compute bind point; pools count each descriptor type, including storage buffers
and storage images. Existing Apple GPU discovery and portability extensions are
retained.

To run the GPU check with the CLion build directory:

```sh
premake5 cmake
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target Prehistoric VulkanComputeSmoke --parallel 6
./cmake-build-debug/bin/VulkanComputeSmoke ./cmake-build-debug/vulkan_compute.spv
```

`VulkanComputeSmoke` is optional and appears when `glslangValidator` is available.
It requires the Vulkan SDK validation layer and GPU access. It uses the engine's
actual `VKComputePipeline`, writes 256 integers to a storage buffer, verifies CPU
readback, recreates the pipeline and repeats. Both Apple M1 devices exposed by the
installed Vulkan drivers passed with zero validation errors. This is a headless
compute check, not validation of the full renderer or its swapchain.

Use the window/asset-manager constructor through `createPipeline(Compute, ...)`
for engine shaders. The native-handle constructor supports compute jobs without
a window; it borrows the device, shader module and pipeline layout. Keep those
objects alive until after the pipeline is destroyed. Native-handle pipelines
aren't asset-manager resources.

`setInvocationSize` specifies workgroup counts, not individual invocation counts.
Bind resources through the Vulkan shader's descriptor sets; the inherited OpenGL
texture/SSBO binding tables are rejected rather than silently ignored. Storage
images must have storage usage and be transitioned to `GENERAL` before use;
`SetTexture` only updates descriptors. Dispatch outside a render pass on a
compute-capable queue. The caller owns submission, memory barriers, queue-family
transfers when needed, and waiting before pipeline recreation or destruction.
See the Vulkan [dispatch requirements](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdDispatch.html)
and [synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html).

The beta environment-map shader is present after the branch merge, but is not
enabled: its image descriptors, shader path and renderer integration are unfinished.
The merge also retains the experimental ocean/FFT, physics, model/texture formats
and scripting code; their presence does not mean they have been validated at runtime.


## Branch workflow after the beta merge

`master` is the shared integration branch. Create a short-lived feature branch from
an up-to-date `master` for each change, review and validate it, merge it back, then
delete the merged feature branch. Codex-created branches use `codex/` as a prefix.
The merge commit retains the history of `beta-features`; the old branch is retired.

The merge keeps the macOS platform, dialogs, Vulkan portability/device selection,
compute pipeline, resource configuration and native OpenAL build. Vulkan texture
loading expands beta RGB/grayscale `.prt` data to RGBA, and records the actual
channel count for STB-decoded RGBA images. OpenGL-only atmosphere and terrain
world objects are skipped by the Vulkan loader. Spotify remains
removed. Generated Visual Studio projects are no longer tracked: regenerate them
with Premake so they match the source and dependency graph.

Beta's Mono scripting integration is Windows-only: its sources, engine hooks,
import library and DLL copy step are excluded on macOS. The Windows implementation
still expects its existing Mono installation/runtime layout. A macOS Mono port is
separate work. Windows execution has not yet been verified on this host.

Next work is ordered as follows:

1. Diagnose the Vulkan viewport occupying the top-left quarter of the window.
2. Diagnose camera translation while rotation works.
3. Add tests for existing behaviour, CI/CD, and actual Windows build verification.
4. Extend Vulkan rendering support.

Application changes stay with the project author. Codex handles merge integration,
tests, CI/CD and supporting maintenance, with feature development kept separate.
