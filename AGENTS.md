# Repository guide

## Layout and scope

- `PrehistoricEngine/src/engine/prehistoric/` contains shared engine code;
  `platform/opengl/` and `platform/vulkan/` contain the rendering backends.
- `Prehistoric/` is the sample application; `PrehistoricEditor/` is the editor.
  Each has its own project `res/` tree. Shared fonts, shaders, primitive models, and default textures live only in
  `PrehistoricEngine/res/`; `scripts/resources/CMakeLists.txt` builds/stages them.
- Shared demonstration models, textures and sounds live in `samples/res/`, staged
  as `sample-resources` by `SampleResources` (also a dependency of `EngineResources`).
  Use `sample://...` references; keep settings, worlds, scripts and unique assets
  with their application. `SharedSampleAssets.h` lists explicit legacy aliases.
- Read `BUILDING.md` for current build instructions. `docs/MACOS_PORT.md` is a
  historical port assessment; some listed blockers have already been resolved.
- Keep changes focused on the requested scope. Preserve unrelated local work.
  Do not change application behavior during maintenance unless requested.

## Build and validation

- Premake files and `scripts/premake-cmake.lua` are the source of the build graph.
  The root `CMakeLists.txt` is generated and ignored; do not hand-edit it.
- For the native CMake workflow, run from the repository root:
  `premake5 cmake`, `cmake --preset debug`, then
  `cmake --build --preset debug --parallel 4`.
- Apple Silicon can use the `macos-arm64` configure/build presets. Native OpenAL
  is supplied through `scripts/native-openal.cmake`; do not patch vendor config headers.
- Run applications with `Prehistoric/` or `PrehistoricEditor/` as their working
  directory so relative resource paths resolve.
- Validate changes with the relevant build or smoke check from `BUILDING.md`.
  `tests/openal_smoke.cpp` checks OpenAL; `tests/vulkan_compute_smoke.cpp` checks
  Vulkan compute. Neither validates the full renderer. Report checks actually run
  and distinguish source inspection from runtime verification.
- Documentation and ignore-only changes need link/path and Git diff checks,
  not a full engine rebuild.

## Code and assets

- Follow the surrounding C++ formatting and ownership conventions. Keep shared
  interfaces backend-neutral and platform-specific code behind platform selection.
- Vulkan feature presence does not imply parity with OpenGL. Verify descriptors,
  image layouts, synchronization, shader interfaces, and integration at call sites.
- Vulkan loads generated SPIR-V from `engine-resources` beside the executable
  (or in its parent for Premake). Build `EngineResources` after shared asset changes;
  both application targets depend on it. Never restore duplicated app shader trees.
- Use `engine://models/...` and `engine://textures/...` for built-ins. Keep app-only scene
  assets in project `res/` trees; shared sample assets use `sample://...`. Resolve paths only in loaders so saved IDs
  remain
  portable; do not auto-export shared assets into project conversion folders.
- Preserve vendored dependencies, runtime binaries, model `.obj` files unless their replacement/build path is explicitly
  addressed.
- Do not track profiler output, ImGui layouts, build output, or machine-local IDE
  settings. Keep profiling directory placeholders: the logger expects the directory.

## Workflow

- `master` is the integration branch. Use short-lived `codex/` branches for new
  branches created by Codex; reuse an appropriate existing checkout.
- Do not commit, merge, or publish unless requested. Summarize changes, validation,
  and remaining limitations when handing work back.
