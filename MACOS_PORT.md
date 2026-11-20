# macOS port assessment

Assessed on 2026-11-20, Apple Silicon, AppleClang 17, Premake 5.0.0-beta8.
This change enables CMake generation and CLion project loading; it does not port
the engine or claim a runnable macOS executable.

## Verified results

- Premake generates all seven main CMake targets; CMake configuration/generation succeeds.
- GLFW (Cocoa), GLAD, and ImGUI compile into native static libraries.
- The original full build failed compiling OpenAL because its vendored config
  enabled Windows/x86 features. This blocker is now addressed for macOS CMake:
  the pinned upstream build generates a native configuration with CoreAudio and
  NEON on arm64, and leaves `HAVE_INTRIN_H` undefined. The arm64 `OpenALSmoke`
  executable builds and passes context creation/audio-buffer upload with the
  silent Null backend; physical speaker output has not been tested.
- Platform header fix implemented: the engine precompiled header now compiles on
  arm64. Static/debug and shared producer/consumer macro configurations also pass
  standalone compile and dynamic-library loading checks.
- No game/editor executable was linked or run. Further compiler errors can emerge
  after these first blockers; the source audit below is not a claim that every
  translation unit has successfully compiled.
- CLion UI import could not be verified: Computer Use permissions were unavailable.
  Open the root generated `CMakeLists.txt` as a project to perform the import.

Paths below are relative to `PrehistoricEngine/src/engine/` unless stated otherwise.

## Changes required, in implementation order

### 1. Native dependency/build configuration

- Root `premake5.lua`: keep `vulkan-1.lib` and `opengl32.lib` in Windows-only filters. On macOS, discover and link
  native Vulkan/MoltenVK libraries. OpenAL is now supplied by its
  upstream CMake target, with `OpenAL32.lib` restricted to Windows. Keep all libraries
  on the same architecture. Avoid linking both bundled OpenAL and a second
  OpenAL implementation. CMake currently retains these original dependencies
  so the incomplete port remains visible rather than pretending to link.
- OpenAL fix implemented: `scripts/native-openal.cmake` downloads the pinned,
  hash-verified upstream 1.24.2 release and uses its CMake platform checks.
  The old `vendor/OpenAL-Soft/include/config.h` remains Windows-specific and is
  no longer used by the macOS build. Use the `macos-arm64` preset to select arm64
  explicitly even if the terminal CMake runs under Rosetta.
- `.gitmodules` names an OpenAL directory different from the actual
  `vendor/OpenAL-Soft` directory. Reconcile this before relying on recursive clones
  for reproducible dependency setup.
- GLFW's missing Cocoa build sources are addressed by this change. The remaining
  GLFW warnings are deprecated macOS APIs, not current compilation failures.

### 2. Platform layer and application startup

- `platform/Platform.h` is now implemented for macOS: `TARGET_OS_OSX` detection,
  empty `PR_API` for static builds/default visibility for shared libraries,
  `dlopen`/`dlsym`/`dlclose`, and `raise(SIGTRAP)` for debugger breaks.
  Library handles are `void*`; `FREE_LIBRARY` returns zero on successful macOS
  close (unlike Windows `FreeLibrary`, which returns nonzero on success).
- `prehistoric/application/EntryPoint.h`: the engine entry point now supports
  both Windows and macOS.
- `platform/windows/WindowsWindow.cpp`: extract/reuse its largely GLFW-based
  window implementation as a shared desktop backend and remove the unconditional
  Windows header from that backend. Use a forward-compatible core context for
  an OpenGL 4.1 fallback. Handle framebuffer pixel size separately from logical
  window size on Retina displays.
- `platform/windows/WindowsInput.*`: reuse its GLFW input handling behind the
  shared desktop backend. Update `prehistoric/common/framework/Input.cpp` and
  `prehistoric/core/engines/RenderingEngine.cpp` includes/factories: they currently
  construct implementations only for Windows, leaving macOS without input/window.
- `platform/windows/WindowsPlatformUtils.h`: keep the Win32 dialogs only on
  Windows. Add an Objective-C++ macOS implementation using `NSOpenPanel` and
  `NSSavePanel` (or a portable dialog library), and select it in
  `prehistoric/common/util/PlatformUtils.cpp`. That file currently includes the
  Windows implementation unconditionally and otherwise leaves its singleton null.
- Root source selection must exclude genuinely Win32-only implementations on macOS
  once the shared/macOS implementations exist.

### 3. Apple Silicon and compiler portability

- `prehistoric/core/util/math/Math.h:7`: guard `<xmmintrin.h>` with the same x86/SIMD
  conditions as the SIMD implementation. The existing scalar branch is the initial
  arm64 route; do not enable `PR_FAST_MATH` on arm64 without a NEON implementation.
- Compile all engine/editor units with AppleClang after the platform header is
  fixed. Resolve missing direct standard-library includes and compiler assumptions
  as errors surface; successful PCH compilation does not validate every source file.

### 4. Choose and implement the rendering route

The current full OpenGL renderer cannot run unchanged on macOS. Repository
`Prehistoric/res/config/settings.json` requests OpenGL 4.5. macOS exposes at most
OpenGL 4.1 core. Specific incompatible uses include:

- `platform/opengl/rendering/pipeline/GLComputePipeline.cpp`: compute dispatch,
  image bindings, and memory barriers.
- `platform/opengl/buffer/GLShaderStorageBuffer.cpp`: shader-storage buffers.
- `platform/opengl/texture/GLTexture.cpp`: immutable texture storage calls that
  need replacement or verified extension support for a 4.1 fallback.
- `Prehistoric/res/shaders/opengl/**`: compute shaders for forward-plus light
  culling, terrain generation, environment maps, and post-processing.

A reduced OpenGL 4.1 port must replace compute work with CPU or fragment-render
passes, replace SSBO usage with supported buffers/textures, adapt shader versions
and features, and disable unsupported effects until equivalents exist. Merely
changing the requested context version cannot work.

The existing Vulkan backend is a plausible longer-term route through MoltenVK,
but it is incomplete, so selecting Vulkan in settings is insufficient:

- `platform/vulkan/framework/context/VKInstance.cpp`: enumerate and enable
  `VK_KHR_portability_enumeration` and set
  `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR` when available/required.
  Use matching modern Vulkan headers and loader libraries.
- `platform/vulkan/framework/device/VKDevice.cpp`: enable
  `VK_KHR_portability_subset` when advertised; query and gate every requested
  feature. It currently unconditionally requests geometry shaders and sample-rate
  shading after checking suitability with only anisotropy enabled. Provide
  alternatives for unsupported features rather than failing device creation.
- `prehistoric/core/modules/gpgpu/NormalMapRenderer.cpp` has commented-out Vulkan
  pipeline/texture creation. Implement the Vulkan compute resources and dispatch.
- `prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.cpp`
  directly casts pipelines to `GLComputePipeline`; move the operations behind
  backend-neutral interfaces and implement the Vulkan equivalents.
- Audit `prehistoric/core/resources/AssembledAssetManager.cpp` and the remaining
  terrain/post-processing modules for OpenGL-only asset/pipeline creation. Supply
  Vulkan shaders and descriptors for the features used by the selected scene.
- Replace shader `compile.bat` steps under `Prehistoric/res/shaders/vulkan/` with
  portable build commands using SDK shader tools; package the resulting SPIR-V.

### 5. Resources, packaging, and validation

Set each app's working directory to its own source folder during development.
For distribution, copy resources into an app bundle and resolve paths from the
bundle instead of assuming the process working directory. Package native dynamic
libraries with appropriate install names/rpaths; Windows DLLs/installers cannot
serve that purpose.

Validate incrementally: native dependencies, window/input and clear-screen test,
audio, simple scene, then terrain, compute effects, editor dialogs, and resource
loading. Test resize/Retina/fullscreen behavior and shutdown on actual macOS.
Only after these pass should a complete macOS port be considered runnable.

## External references

- [CLion CMake project import](https://www.jetbrains.com/help/clion/cmakelists-txt-file.html)
- [GLFW macOS context requirements and Retina sizing](https://www.glfw.org/faq.html)
- [MoltenVK runtime and portability requirements](https://github.com/KhronosGroup/MoltenVK/blob/main/Docs/MoltenVK_Runtime_UserGuide.md)
