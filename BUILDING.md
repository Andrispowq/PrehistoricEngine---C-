# Using the engine

If you wish to use the engine, follow these steps:
- Clone the repository into a folder on your computer with the following command: `git clone --recursive https://github.com/Andrispowq/PrehistoricEngine---C-`
- Create the solution files: the build system currently supports Visual Studio (version 2017 and 2019)
  - For Visual Studio Insiders with the v145 C++ toolset, run `build_windows_vs2022.bat`. The bundled Premake uses the `vs2022` generator, and the workspace sets its toolset to v145.
  - For older Visual Studio versions, use `build_windows_vs2017.bat` or `build_windows_vs2019.bat`.
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
channel count for STB-decoded RGBA images. Vulkan supports the scattering atmosphere background. Terrain world objects use OpenGL. Spotify remains
removed. Generated Visual Studio projects are no longer tracked: regenerate them
with Premake so they match the source and dependency graph.

Mono scripting is Windows-only. The Windows build copies `mono-2.0-sgen.dll`
and the runtime from `PrehistoricEngine/Dependencies/lib/Mono` beside both
executables. Keep the DLL and the `Mono` directory beside the executable when
moving or packaging a build.

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

The Vulkan scene writes linear radiance into an RGBA16F attachment. A fullscreen
subpass reads the resolved colour and applies the shared OpenGL exposure curve. For sRGB attachments,
the shader leaves tone-mapped output linear and the attachment encodes it once.
For non-sRGB attachments, the shader retains the configured gamma encoding. Direct lights use the shared
Schlick-GGX geometry term with `k = (roughness + 1)^2 / 8`.
Rebuild the app normally to regenerate the SPIR-V and update the descriptor layout
alongside the C++ code.

`python3 tests/vulkan_pbr_contract.py` compiles and validates the fragment shader,
checks its scalar emission layout and texture bindings, and tests production
sRGB/UNORM format selection numerically. It requires `glslc`, `spirv-val`,
`spirv-dis`, and a C++17 compiler (`CXX`, default `c++`). It does not perform GPU
frame rendering or visual comparison.

## HDR environment maps in OpenGL and Vulkan

Both HDR paths convert the configured panorama into a floating-point cubemap at
`environmentMapResolution`, generate its mip chain, and bake irradiance and GGX
reflection cubemaps. They draw the background from the reflection map at LOD zero
by default. The shared GLSL in `shared/environmentMap/` defines face
orientation, panorama texels, angular sampling, source LOD, and the split-sum
BRDF integral. A constant white environment produces diffuse irradiance of PI.
The BRDF LUT depends only on view angle and roughness.

Vulkan draws the background into the HDR scene using a fullscreen triangle. OpenGL draws a cube and applies its HDR post
process. Both applications configure the original Cape Hill HDR. An empty
Vulkan environment location keeps the window clear colour when the scene has no atmosphere. Vulkan uses a
single source cube level on hardware lacking float linear blits.

Both backends use 512 integration samples and a 256-square BRDF LUT. The sample and editor default to 64 texels per irradiance face edge and 256 per reflection face edge.
Vulkan retains the baked maps across resizes.

Run `python3 tests/vulkan_background_contract.py` for the background shader
interface and `python3 tests/vulkan_ibl_smoke.py` for headless GPU readback
using black, constant, directional, Cape Hill, and procedural sky environments. The latter
requires a Vulkan SDK, a C++17 compiler and a Vulkan device with validation
layers. Use `python3 tests/resource_pipeline.py` to check staging and install.

## Renderer frame order

After scene registration and successful frame preparation, both renderers update
environment lighting, draw the scene, run HDR processing, and finish the output.
OpenGL runs its depth, shadow, light-culling, and water capture passes before
main colour, then runs optional bloom and volumetric processing before tone mapping.
Vulkan renders scene colour and material emission into separate floating-point attachments.
With bloom enabled (K toggles it), fullscreen passes blur emission at up to eight resolutions
and add the levels together. The presentation pass combines bloom with scene colour before tone mapping.
The renderer owns the passes and baked maps for its backend. Outstanding rendering
repairs are listed in the [rendering backlog](docs/RENDERING_BACKLOG.md).

## Volumetric lighting test

For a backlit arcade view, open [Sponza Sun Shafts](PrehistoricEditor/res/world/sponza_sun_shafts.json)
from the editor's `res/world` folder. The world sets the starting camera,
exposure, directional sun, and dust volumes. Both renderers preview the shafts. For a clear-air comparison, set both
volumes' density to zero and
compare the light on the floor.

Use **Sun Shadow Radius** in the editor lighting controls to compare
hard shadows at zero with contact-hardening soft shadows. The optional
`engine.renderer.shadowSunAngularRadius` setting supplies the startup radius
in radians (default `0.02`). This controls shadow filtering independently
of the visible atmospheric sun disk.

Open [the Sponza world](Prehistoric/res/world/sponza_volumetric.json) in the
editor using File → Open. To launch the sample directly, set
`PREHISTORIC_WORLD=res/world/sponza_volumetric.json` and run it from
`Prehistoric/`. Both backends can preview, edit and save the volumes.

Right-click empty space in the Scene Hierarchy and choose **Create Volumetric
Sphere**, or add **Volumetric Sphere** to an existing object's components.
The object's position is the center. Edit radius, density, tint, and anisotropy
in Properties, then use File → Save As to write them to the world JSON. Radius
is in world units and is independent of the object's scale. Density is the
center extinction per world unit. Set it to zero for a clear-air comparison.
Positive anisotropy favors looking toward the light.

Each backend renders up to eight spheres with positive radius and density. The
first shadow-casting light supplies directional lighting. Density fades
smoothly to zero at the edge, and opaque surfaces clip the haze.

## Vulkan procedural sky

A JSON world with a `scattering_atmosphere` object and scattering enabled draws
a fullscreen sky into the linear HDR scene. The sun field can name a light in
the scene root. An empty sun field creates a sun that rises from the +X horizon and completes
a full orbit in 120 seconds. The visible sky follows the orbit each frame, and
the default lighting refresh interval is 10 seconds.
Both backends use the Rayleigh/Mie integration and disk profile in
`shaders/shared/atmosphere/scattering.glsl`. Vulkan applies exposure in its
final HDR pass. In worlds without an active scattering atmosphere, the HDR
environment background is used when configured. Vulkan bakes the registered scattering atmosphere into a linear
HDR cubemap
before the first PBR draw, including the bright solar core. OpenGL leaves the
solar disk out of its lighting bake when the same sun supplies shadowed direct
lighting. The disk remains visible in the sky. The irradiance and GGX prefilter
stages consume the resulting cubemap.

### Atmosphere capture timing

Both renderers choose the lighting source after the scene has submitted its sky,
lights, and draw batches. The first rendered frame and a switch between sources
trigger a complete bake before scene draws. OpenGL water reflections and
refractions also run after this decision. Each bake uses one snapshot for all
six faces and publishes the environment, irradiance, and reflection maps together.
The source-independent BRDF LUT survives refreshes.

The visible sky follows the sun every frame. For lighting, both backends compare
the current sun direction and scattering parameters with the last completed
capture. OpenGL also compares exposure because its capture shader applies it.
Changes accumulate until at least `environmentMap.atmosphereRefreshSeconds` have
elapsed since completion (default 10 seconds). The next eligible frame bakes
the latest state. A stationary sky, camera movement, or window resize alone
leaves the maps intact. Setting the interval to zero refreshes on each changed
frame. Baking is synchronous and can make that frame longer.

In the OpenGL editor, loading an HDR file selects it as the lighting source.
Regenerate returns to the scene source. Both actions request a bake on the next
rendered frame and bypass the refresh interval.

## OpenGL voxel rendering

Set `framework.api.active` to `OpenGL` in the application's
`res/config/settings.json`. The voxel path requires OpenGL 4.5 or later.
In the sample launcher, set `PREHISTORIC_WORLD=res/world/sponza_volumetric.json`
and run from `Prehistoric/`. Press **V** to display occupied voxels, **G** to toggle
voxel GI, and **B** to cycle through the available grids from
64³ to 1024³.
The debug view takes precedence while both modes are enabled.

For an emissive GI comparison, launch
`PREHISTORIC_WORLD=res/world/sponza_gi.json`. Two emissive cubes sit at floor
level behind the red and green curtains in opposite side aisles. A third,
green emitter sits in the top-floor side aisle above the amber cube. Toggle
**G** to compare the light on nearby stone. Toggle
**K** to disable bloom for the comparison.

For the same emitters under a night sky, launch with
`PREHISTORIC_WORLD=res/world/sponza_night_gi.json` from `Prehistoric/`, or open
that world in the OpenGL editor. The world's optional `environment_map` field
selects the shared Milky Way HDR panorama. This scene has no atmosphere or sun.
With bloom off, toggle **G** at a fixed camera position to compare emissive
bounce on nearby surfaces. The panorama supplies the background and environment
lighting. The voxel bake in this scene gets its radiance from the emitters.

The editor opens [Sponza Midnight Atmosphere](PrehistoricEditor/res/world/sponza_midnight_gi.json)
by default. The OpenGL editor starts with voxel GI enabled on supported GPUs,
four diffuse bounces, directional voxel mips, and a 256³ grid subject to the
renderer’s memory budget.
The sun stays directly below the scene and supplies no direct light or voxel
radiance. The planet occludes the solar disk and single-scattered sunlight at
this angle, so the atmosphere and its baked environment are black.
To launch the same world from the sample in a repository checkout, set
`PREHISTORIC_WORLD=../PrehistoricEditor/res/world/sponza_midnight_gi.json`.

In the OpenGL editor, use **Render Settings** to set those options, the voxel
volume center and width, GI strength, and **GI Trace Depth** from one to five
diffuse bounces. The additional bounces run during the radiance bake. Each
propagation traces the preceding bounce and adds its output to the volume
sampled by the fullscreen pass. Voxel size is the volume width divided by grid
resolution. The default 400-unit cube is centered at the origin and
covers the Sponza test world. The 512³ and 1024³ options depend on available
GPU memory and the selected bounce depth. Switching to a more expensive mode
can select a smaller grid that fits the memory budget. Use a tighter volume for smaller scenes. Voxel
settings are local to the renderer session. The menu also exposes the existing
wireframe and bloom toggles.

Use **Half-Resolution GI** in **Render Settings** to compare one trace per
2×2 block with the full-resolution default. The composite uses surface
position and normal to reject samples from neighboring surfaces. Expand
**Voxel GPU Timings** for the latest available GPU measurements. Bounce
propagation includes its intermediate radiance mips. Final radiance mips and
the per-frame tracing and composite passes have separate timings.

Toggle **Directional Voxel Mips** off to use isotropic box averages. The
directional mode uses six half-resolution mip chains that composite the near
layer over the far layer. Switching modes rebakes radiance, including all
diffuse bounces. Both modes use the same first-hit occlusion test. At 256³
the directional chains add about 110 MiB to the voxel allocation. See the
[VXGI improvement plan](docs/VXGI_PLAN.md) for references and A/B results.

OpenGL caches the debug and GI volumes independently. Grid bounds, resolution,
and changes to opaque mesh submissions, transforms, or sampled materials can
invalidate a bake. GI uses a shadow map fitted to the voxel volume, so camera
movement reuses both volumes. Changes to the same sun's direction, colour, or
intensity refresh the radiance volume at most once per
`environmentMap.atmosphereRefreshSeconds` interval (10 seconds by default) while
GI is active. A geometry change or a change of sun source can trigger an
immediate bake. The last completed GI volume remains visible between refreshes.

Radiance mipmaps are generated on the GPU with a compute downsample. Changing
trace depth rebakes radiance. Changing GI strength affects the fullscreen
composite only. The debug ray march and GI cone tracing run every displayed
frame.

**Voxel Sun Shadows** in the editor switches direct sunlight between the
default shadow cascades and rays through the opaque voxel grid. The voxel
mode skips the cascades and the volume-fitted sun shadow map. Compare the
modes in Sponza with the same camera and grid settings. Voxel shadows can miss
thin geometry and cover only receivers and casters within the volume.

GI adds bounced light from sun and emissive materials. With GI enabled,
OpenGL PBR point lights also test voxel visibility to reduce direct light
through walls. Cone tracing stops at the first occupied cell along the cone
axis to reduce indirect leaks. The hit radiance is filtered across that voxel
face, and a screen-space filter smooths indirect light using receiver position
and normal before the result is added to the scene. Local point lights still
use the direct PBR lighting path.

At trace depths above one, the bake stores one representative diffuse surface
per voxel. At 256³ this adds a 64 MiB surface grid and up to two 146 MiB
radiance volumes for propagation. Higher depth and resolution increase the
cost of each bake.

Terrain and water patches can receive the screen-space GI contribution through
their surface buffers, while only opaque meshes populate the volume. Thin
geometry can produce gaps at coarse resolutions. At 256³ the debug cache uses
64 MiB, and GI uses about 210 MiB including its packed work texture.

## CPU math tests and CI

The first unit-test milestone uses Catch2 3.8.1 and CTest. Generate the build graph
on the current host from a recursive checkout (Premake reads the vendor project
files), then use the isolated test presets:

```sh
premake5 cmake
cmake --preset tests
cmake --build --preset tests --parallel 4
ctest --preset tests
```

Use `tests-release` in place of `tests` in all three CMake/CTest commands to test
optimized code. These configurations set `PREHISTORIC_TESTS_ONLY=ON` and return
before configuring the engine, Vulkan, shaders, OpenAL, or application resources.
They compile the production `Math.cpp`, `Math.h`, and `MathT.h` directly, without a
window, GPU, audio device, engine PCH, or scene. Existing application presets are
unchanged. CMake 3.21+, Ninja and a C++17 compiler are required; Windows Ninja
builds should run from an x64 Visual Studio developer shell.

The first configure downloads a SHA-256-pinned Catch2 source archive. For offline
use, supply an extracted copy of that same release:

```sh
cmake --preset tests -DFETCHCONTENT_SOURCE_DIR_CATCH2=/absolute/path/to/Catch2-3.8.1
```

`MathTestsScalar` runs on macOS and Windows. MSVC x64 also builds
`MathTestsSIMD`, compiling the same contracts and production source separately
with `PR_FAST_MATH`. This keeps incompatible scalar/SIMD types out of the same
executable. CTest discovers individual cases, gives each a timeout, and the test
presets fail if no tests are found.

Coverage includes vector arithmetic and normalization, cross products,
reflection, interpolation, axis rotation, quaternion products and conjugates,
matrix storage/copy/move construction, transform order, invertible transforms,
perspective mapping and view-basis conventions. Float comparisons combine
absolute and relative tolerances; inputs are deterministic. Zero normalization,
singular inverse policy, invalid projection arguments, and exhaustive template
instantiations are not specified by this initial suite.

Run the correctness contracts with `ctest --preset tests -L contract`.
The four initially reproduced scalar defects are now normal passing contracts:

- Vector4 and quaternion `(Vector2, z, w)` constructors preserve all components,
  in both scalar and SIMD implementations.
- Scalar orthographic projection uses column-major translation and matches SIMD:
  view-space `-near` maps to depth -1 and `-far` to +1, as with perspective.
- All four scalar Vector2 ordering operators compare matching components.

One separate Windows SIMD defect remains: scalar interpolation does not apply its
weight to every component (the reproducer exercises Vector3's `y`). On Windows,
`ctest --preset tests -L known-defect -V` runs this Catch2 `!shouldfail` case.
The expected assertion failure satisfies that tracker; an unexpected pass fails
it so the marker must be removed when fixed. It is not a passing correctness
contract. CI runs this separate report only on Windows; macOS has no remaining
expected-failure cases.

The testing setup also includes compilation/linkage prerequisites: explicit
math/C-string includes, template definitions included by Math.cpp, external
linkage for its out-of-line methods, and the duplicate SIMD texture-coordinate
member name (`p` names the third texture coordinate; `r` remains the first colour
coordinate).

`.github/workflows/math-tests.yml` runs Debug and Release on macOS 14 and Windows
2022 for pull requests and pushes to `master`, plus manual dispatch. It installs
checksum-pinned Premake 5.0.0-beta8, generates CMake on each host, and uploads
separate contract/known-defect JUnit reports and CTest logs even on failure.
Windows uses the Visual Studio 2022 x64 generator to avoid requiring an activated
Ninja toolchain shell. The workflow does not build or validate the full engine,
renderer, camera, events, or frame loop. Branch-protection settings must be applied
in GitHub after the workflow has run successfully; this file alone does not make
checks required.
