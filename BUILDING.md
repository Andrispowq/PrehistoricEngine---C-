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
[macOS port assessment](docs/MACOS_PORT.md) records the original port investigation and may
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
at configure time. The full engine still needs the other changes in [the macOS port assessment](docs/MACOS_PORT.md).

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

## Shared fonts and shaders

Fonts and shader sources now live only in `PrehistoricEngine/res/fonts` and
`PrehistoricEngine/res/shaders`. The shared Vulkan PBR fragment shader retains the
sample application's edited geometry term, `(roughness + 1)^2 / 2`.

Install CMake 3.21+ and `glslc` from the Vulkan SDK. The resource build finds `glslc`
on `PATH` or under `VULKAN_SDK/bin` (`Bin` on Windows). You can select it explicitly
with `-DPREHISTORIC_GLSLC=/absolute/path/to/glslc` when configuring. A missing compiler
fails configuration instead of silently using old checked-in SPIR-V.

Regenerate and reload CMake after pulling this change:

```sh
premake5 cmake
cmake --preset debug
cmake --build --preset debug --target Prehistoric PrehistoricEditor --parallel 4
```

Both application targets depend on `EngineResources`. For resource-only work:

```sh
cmake --build --preset debug --target EngineResources
# Or compile only the active Vulkan shaders:
cmake --build --preset debug --target Shaders
```

The CMake build stages fonts, OpenGL GLSL and generated Vulkan SPIR-V under
`<build>/bin/engine-resources/`. Vulkan basic/PBR vertex and fragment shaders compile
incrementally for Vulkan 1.1. Entrypoint edits rebuild that shader; changes to
other files in the Vulkan shader tree conservatively rebuild all active shaders
so includes remain tracked even with SDK depfile bugs affecting paths with spaces.
OpenGL continues to compile GLSL at runtime. Shader compilation failures fail the
build. Do not check generated `.spv` files into the source resource tree.

Native Premake Visual Studio builds use an `EngineResources` Utility project that
invokes the same standalone CMake resource build. Both apps depend on this project,
so shader-only edits are processed without requiring a C++ relink. CMake and `glslc`
must also be available to Visual Studio. Resources are shared at
`bin/<configuration>-<system>-<architecture>/engine-resources`, alongside the
application output folders. Regenerate the solution with the existing batch script.
The resource CMake generator follows the installed CMake default; its build tree is
under `build/resources-<configuration>-<system>-<architecture>`.

The loader searches `engine-resources` next to the executable, then its parent
(the Premake layout). Set `PREHISTORIC_ENGINE_RESOURCES` to an explicit resource
package directory to override discovery. An explicit override is authoritative;
missing files produce an error rather than falling back to old application copies.
Both fonts and shaders use this resolver, including OpenGL shader includes.
Application working directories still need to be `Prehistoric/` or
`PrehistoricEditor/` for the remaining project assets.

A standalone resource build does not configure the engine or download OpenAL:

```sh
cmake -S scripts/resources -B build/resources -G Ninja
cmake --build build/resources --target EngineResources
cmake --install build/resources --prefix build/resource-package
```

Copy the installed `engine-resources` directory next to the deployed executable.
Both Windows installers now include the generated resource directory; they default
to `bin/Distribution-windows-x86_64/engine-resources` and accept an Inno Setup
`/DEngineResourceDir=...` override. Build resources before packaging. Other app
resources remain in their existing `res/` directories.

The editor-only experimental Vulkan environment-map sources are preserved in the
shared tree. `-DPREHISTORIC_EXPERIMENTAL_SHADERS=ON` additionally compiles
`environmentMap.comp`; it does not enable or complete that rendering pipeline.
Other unfinished environment shader variants remain source-only.

Run `python3 tests/resource_pipeline.py` for isolated build/lookup checks. It needs
CMake, Ninja, `glslc`, and a C++17 compiler, but no GPU. It checks no-op builds,
shader/include edits, compiler errors, staged installation, both executable layouts,
explicit overrides and missing resources. Full rendering still needs runtime checks.

## Shared engine default models and textures

`PrehistoricEngine/res/models/` now owns cube, quad, and sphere OBJ files plus their
existing PMF variants under `models/prehistoric/`. The shared texture set contains
`default.png`, `logo.png`, all eight FFT noise images (256/512), water `dudv1.jpg`,
and their existing PRT variants. Noise filenames use lowercase consistently.
`EngineResources` stages these files in the same package as fonts and shaders;
both CMake and Premake application builds and the installers use that package.

Code and saved asset IDs use `engine://models/cube.obj` or
`engine://textures/default.png`. The asset manager canonicalizes the old paths for
these specific defaults, so existing JSON and text worlds still load without
rewriting scene files. The world loader accepts engine IDs even when a scene sets
`res/models/` or `res/textures/` as its base directory. Physical executable-relative
paths are resolved only when reading files, not stored in serialized asset IDs.

Legacy aliases are an explicit list of migrated defaults, including the old
capitalized noise filenames and converted PMF/PRT paths. Other project paths are
not redirected. These legacy default names are reserved aliases; give custom
project assets distinct names. Missing engine resources fail explicitly rather
than substituting a project asset.

Shared Sponza, dome, `myscene`, terrain textures, floor materials and environment
images now belong to the separate sample package described below. Unique grass,
converted assets and water caustics stay app-specific. Project working-directory
requirements remain unchanged for app settings, worlds and unique assets.

Immediate and queued loads of shared defaults skip the existing automatic PMF/PRT
export. Loading a built-in no longer recreates copies under application `res/`
folders or requires writing to the shared package. Existing project asset import
and export behavior remains unchanged. The checked-in converted defaults are
preserved as inputs, not regenerated during a read.

`python3 tests/resource_pipeline.py` also verifies all default models/textures in
both executable layouts, legacy aliases, engine IDs with world directory prefixes,
and that scene-specific references stay project-local.

## Shared sample resources

Shared sample content now lives in `samples/res/`, separately from engine defaults:
5 model files (dome OBJ, myscene OBJ/MTL, Sponza OBJ/MTL), 88 texture/support files,
and 5 WAV files. Matching paths were compared byte-for-byte before consolidating
both application copies. Associated texture documentation is preserved.

Both applications build `SampleResources` through their existing `EngineResources`
dependency. CMake stages the package at `<build>/bin/sample-resources`; native
Premake stages it beside `engine-resources` in the configuration output directory.
Rebuild just sample content with:

```sh
cmake --build cmake-build-debug --target SampleResources
```

The standalone `scripts/resources` build and install now produce both
`engine-resources` and `sample-resources`. Deploy both directories beside the apps
(or in the common parent directory for Premake's separate application folders).
`PREHISTORIC_SAMPLE_OUTPUT` selects the sample staging destination at configure time;
its default is beside `PREHISTORIC_RESOURCE_OUTPUT`. Windows installers include the
sample package and accept `/DSampleResourceDir=...` for an alternative build layout.

Runtime IDs use `sample://models/sponza.obj`, `sample://textures/oakFloor/oakFloor_DIF.png`
and `sample://sounds/_Closer.wav`. `PREHISTORIC_SAMPLE_RESOURCES` overrides the
sample package location independently of `PREHISTORIC_ENGINE_RESOURCES`. Explicit
missing overrides fail rather than falling back to another package. The manifest
in `SharedSampleAssets.h` maps only the migrated legacy `res/...` paths to sample
IDs, so existing worlds and configurations do not need rewriting. The world loader
accepts sample IDs alongside its per-scene directory prefixes; saved IDs remain
portable. Model material files are located relative to the model's parent directory.

Sample model and texture reads skip automatic PMF/PRT export, just like engine
assets. Unique converted PMF/PRT files, the sample app's grass pack and caustics
remain application-owned. Settings, terrain configurations, worlds, scripts,
fonts/shader ownership and engine defaults are unchanged by this consolidation.
Legacy shared filenames are reserved aliases; use distinct names for new private
project assets. An identical filename alone never triggers automatic sharing.

The resource regression script checks both packages and app layouts, aliases,
missing overrides, and incremental sample updates without shader rebuilds. Build
and path tests do not establish full scene rendering or audio playback correctness.

## Vulkan PBR emission and output encoding

Vulkan PBR uses the same scalar `emission` material property as OpenGL. Missing or
negative emission means zero; positive emission is tinted by albedo with OpenGL's
existing minimum tint and emission factor. There is no Vulkan PBR emission-map
binding, so a missing map cannot pick an unrelated material texture.

PBR output uses the actual colour-attachment format each frame. For sRGB attachments,
the shader leaves tone-mapped output linear and the attachment encodes it once.
For non-sRGB attachments, the shader retains the configured gamma encoding. Exposure
and the BRDF geometry divisor `/ 2.0` are unchanged; background rendering is separate.
Rebuild the app normally to regenerate the SPIR-V and update the descriptor layout
alongside the C++ code.

`python3 tests/vulkan_pbr_contract.py` compiles and validates the fragment shader,
checks its scalar emission layout and texture bindings, and tests production
sRGB/UNORM format selection numerically. It requires `glslc`, `spirv-val`,
`spirv-dis`, and a C++17 compiler (`CXX`, default `c++`). It does not perform GPU
frame rendering or visual comparison.
