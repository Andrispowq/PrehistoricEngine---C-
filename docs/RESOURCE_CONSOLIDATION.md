# Resource consolidation assessment

**Implementation update:** Fonts and shaders have since been consolidated under
`PrehistoricEngine/res/`, with generated runtime packages and portable compilation.
See [BUILDING.md](../BUILDING.md#shared-fonts-and-shaders). The inventory and findings
below describe the pre-migration snapshot.

Assessed 20 November 2026 against the current working tree, including the local Vulkan PBR shader edit. Scope: every
immediate folder in `Prehistoric/res/` and `PrehistoricEditor/res/`, recursively compared by relative path and SHA-256
content hash, with loader and writer call sites inspected.

**A shared resource root is viable, but moving every resource into engine ownership is not.** Fonts and renderer shaders
are strong engine-resource candidates. Models and textures mix engine defaults with scene content. Audio can be
deduplicated as shared sample content. Settings, worlds, and scripts retain application or project ownership; profiling
is writable runtime output.

No populated folder is safe to relocate by filesystem operations alone with the current code. Paths depend on the
application's working directory, and several bypass the asset manager. “Share” below means safe after the listed path,
ownership, and packaging changes. This is a static assessment, not a runtime migration test. No assets were moved.

## Inventory and classification

Counts include tracked files and the two new `.gitkeep` placeholders; ignored logs and other untracked runtime files are
excluded. “Identical” means matching relative path and identical bytes. Savings remove one copy of those files, not Git
history or files with different names/formats.

| Folder    | Sample files | Editor files | Identical pairs | Conflicting paths | Classification                                       |
|-----------|-------------:|-------------:|----------------:|------------------:|------------------------------------------------------|
| config    |            3 |            3 |               2 |                 1 | Split defaults from application settings             |
| fonts     |           10 |           10 |              10 |                 0 | Share as engine resources                            |
| icon      |            1 |            1 |               1 |                 0 | Share current default; preserve application override |
| models    |           32 |            8 |               8 |                 0 | Split engine primitives from project content         |
| profiling |            1 |            1 |               1 |                 0 | Keep outside read-only resources                     |
| scripts   |            5 |            3 |               0 |                 0 | Keep application/project-specific                    |
| shaders   |           71 |           79 |              70 |                 1 | Share after resolving shader conflict                |
| sounds    |            5 |            5 |               5 |                 0 | Share sample content, not engine defaults            |
| textures  |          172 |           90 |              90 |                 0 | Split engine defaults from project content           |
| world     |            2 |            7 |               0 |                 2 | Keep application/project-specific                    |

There are **186 identical asset pairs plus one empty placeholder pair**. Removing one copy saves **544,789,320 bytes,
about 519.6 MiB** of working-tree content. Textures account for about 308.3 MiB, sounds 186.4 MiB, and models 23.7 MiB.
This measures duplication, not permission to delete every duplicate or equivalent runtime behavior after relocation.

## Folder assessments

### config

**Do not merge the entire folder.** Both terrain configurations are identical, but `settings.json` differs in window
title, OpenGL version, active backend, environment-map asset, and water height. The sample selects Vulkan and a `.prt`
environment map; the editor selects OpenGL and an HDR source. Picking either file would change the other application's
behavior.

Engine defaults could live centrally, with explicit per-application settings layered over them. The current loader reads
one JSON document; layered defaults would require implementation. Identical terrain presets can belong to a shared
sample package, but remain scene content rather than universal engine configuration. Their texture paths must migrate
with their dependencies.

Evidence: [CoreEngine.h](../PrehistoricEngine/src/engine/prehistoric/core/CoreEngine.h), [EngineSettings.cpp](../PrehistoricEngine/src/engine/prehistoric/core/config/EngineSettings.cpp),
and both applications' `res/config/` trees.

### fonts

**Yes, share all ten current files as engine UI resources.** The Open Sans files are byte-identical. The shared ImGui
layer directly loads Bold and Regular at `res/fonts/opensans/...`; route these calls through the engine resource root.
Preserve the other variants unless their removal is separately justified. Future projects should be able to select their
own fonts without editing engine resources.

Evidence: [ImGUILayer.cpp](../PrehistoricEngine/src/engine/prehistoric/core/ImGUI/ImGUILayer.cpp).

### icon

**Yes for the identical current default, with packaging changes.** Both folders contain the same `logo.ico`. Both
Windows installer scripts use `{app}\\res\\icon\\logo.ico` for shortcuts, so the deployed path must be updated or
intentionally preserved. An application should retain control over its branding.

The GLFW window icon is a separate asset: it loads `res/textures/logo.png`, not this ICO. Migrating only `icon/` does
not update the window icon.

Evidence: [engine installer](../engine_installer_script.iss), [editor installer](../editor_installer_script.iss), [WindowsWindow.cpp](../PrehistoricEngine/src/engine/platform/windows/WindowsWindow.cpp).

### models

**Share engine primitives; keep scenes and imported content in project or sample resources.** All eight editor files
have identical sample counterparts: cube, quad, sphere, dome, `myscene` OBJ/MTL, and Sponza OBJ/MTL. Cube and sphere are
directly used by engine environment-map/atmosphere code; the editor also loads cube. These are good central defaults.
Sponza, `myscene`, and the sample-only grass asset pack are scene content even when shared by two applications.

The sample additionally contains six `.pmf` files and the grass pack, including images, a Blender source, and a PDF.
These must not be discarded merely because an OBJ version also exists. Model loading can write converted PMF assets, so
move those writes to a cache or explicit project export destination before treating the shared root as read-only.

Preserve OBJ/MTL relationships. The OBJ loader uses a material search directory and constructs material texture paths
under `res/textures/`; moving meshes without their referenced textures is unsafe.

Evidence: [OBJLoader.cpp](../PrehistoricEngine/src/engine/prehistoric/core/model/obj/OBJLoader.cpp), [VertexBufferLoader.cpp](../PrehistoricEngine/src/engine/prehistoric/core/resources/loaders/VertexBufferLoader.cpp), [PrehistoricModelFormat.cpp](../PrehistoricEngine/src/engine/prehistoric/core/model/loader/PrehistoricModelFormat.cpp), [EnvironmentMapRenderer.cpp](../PrehistoricEngine/src/engine/prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.cpp).

### profiling

**No. This is output, not an input asset.** Only the empty placeholders belong to the inventory; `engine.log` is
ignored. The logger opens `./res/profiling/engine.log` for writing. A shared filename would let two application
instances overwrite each other's logs.

Use a writable application/session-specific log directory and create it at startup. Keep logs out of engine resource
packages and installation directories.

Evidence: [Log.cpp](../PrehistoricEngine/src/engine/prehistoric/core/util/log/Log.cpp).

### scripts

**No wholesale consolidation.** The sample contains `CameraController.cs`, `ExampleComponent.cs`, `Logger.cs`,
`PlayerController.cs`, and `Scripts.dll`. The editor contains `Script.cs`, `Script.dll`, and `compile.bat`. There are no
matching relative paths to deduplicate.

Keep gameplay/editor scripts with their project. The engine's reusable scripting support already lives under
`PrehistoricEngine/scripting/Support/`. Script compilation and assembly paths depend on the working directory, and the
editor batch file uses a relative Mono compiler path. Compiled assemblies should eventually be build outputs, but
removing them now requires a reliable replacement build path. The current Mono integration is Windows-only.

Evidence: [ScriptEngine.cpp](../PrehistoricEngine/src/engine/prehistoric/core/engines/ScriptEngine.cpp), [editor script build](../PrehistoricEditor/res/scripts/compile.bat), [BUILDING.md](../BUILDING.md).

### shaders

**Yes, this is the highest-priority consolidation candidate, after one explicit conflict decision.** Seventy files match
exactly. The editor has eight additional Vulkan environment-map files; preserve them as experimental engine assets
rather than silently dropping them or treating them as a validated pipeline.

The current `vulkan/pbr/pbr.frag` differs: the sample's local edit uses `(r * r) / 2.0`, while the editor uses `/ 8.0`
in `GeometrySchlickGGX`. This is a shading behavior change, not formatting. Both PBR fragment SPIR-V files are identical
despite the GLSL difference; identical binaries therefore do not establish that either source has been rebuilt. Choose
the intended formula and regenerate binaries as part of the later shader build work.

Both shader loaders prepend `res/shaders/`. OpenGL include expansion recursively uses that same root. Centralize source
paths and include resolution, and give Vulkan generated binaries a separate build/deployment location. Preserve existing
SPIR-V until the replacement compilation and loading path works for both applications. Backend subdirectories must
remain distinct.

Evidence: [GLShader.cpp](../PrehistoricEngine/src/engine/platform/opengl/rendering/shaders/GLShader.cpp), [VkShader.cpp](../PrehistoricEngine/src/engine/platform/vulkan/rendering/shaders/VkShader.cpp), [current consolidated fragment source](../PrehistoricEngine/res/shaders/vulkan/pbr/pbr.frag).

### sounds

**Technically shareable in full; classify as shared sample content.** All five WAV files match byte-for-byte. The sample
application directly loads `_Closer.wav` and `_Eternal.wav`; these are application soundtrack choices, not engine
requirements. A shared sample asset package eliminates the duplicate audio without forcing it into every engine
deployment.

Update caller paths and packaging. Identical bytes alone do not establish that a track is unused or that its
distribution terms allow a broader engine package.

Evidence: [PrehistoricApp.cpp](../Prehistoric/src/PrehistoricApp.cpp), both `res/sounds/` directories.

### textures

**Share a selected engine subset and deduplicate remaining sample textures separately.** All 90 editor
textures/supporting files have identical sample counterparts. The sample has another 82 files, including FFT noise
images, water textures, and converted `.prt` assets.

`default.png` is an engine material fallback; the logo is a default window icon. FFT noise and water distortion textures
are dependencies of engine modules. These are good central defaults. Terrain surfaces, floor materials, Sponza textures,
heightmaps, and environment HDR images are scene/project choices even where engine code consumes them.

Moving this folder requires more than changing `TEXTURE_PATH`: material constructors, window setup, FFT/water modules,
OBJ material loading, configuration files, and serialized worlds contain paths. Conversion currently writes to
`res/textures/prehistoric/` using only the basename, so centralizing all projects would also increase filename-collision
risk. Preserve directory identity in a writable conversion cache. Do not equate `.prt` assets with their source images:
current sample settings explicitly select a converted asset.

There is also a case-sensitive portability issue: FFT code asks for `noise256_0.jpg` through `noise256_3.jpg`, while
source filenames begin `Noise256_`. Resolve this during migration and validate on a case-sensitive filesystem.

Evidence: [Material.cpp](../PrehistoricEngine/src/engine/prehistoric/core/model/material/Material.cpp), [H0k.cpp](../PrehistoricEngine/src/engine/prehistoric/core/modules/fft/H0k.cpp), [Water.cpp](../PrehistoricEngine/src/engine/prehistoric/core/modules/water/Water.cpp), [TextureLoader.cpp](../PrehistoricEngine/src/engine/prehistoric/core/resources/loaders/TextureLoader.cpp).

### world

**No, preserve project identity and writable destinations.** Both overlapping filenames, `world_1.json` and
`testLevel.wrld`, have different contents. The editor also has five unique worlds. Merging by filename would overwrite
distinct scenes.

World files contain resource directories and references; the serializer writes those references back. The editor saves
new worlds under `res/world/`. Keep project worlds separate, migrate their asset references deliberately, and direct
saves to the project rather than shared engine resources. A reusable demonstration world can be an explicitly named
sample/template; opening it for editing should not overwrite the shared template.

Evidence: [WorldLoader.cpp](../PrehistoricEngine/src/engine/prehistoric/core/scene/world/WorldLoader.cpp), [WorldSerialiser.cpp](../PrehistoricEngine/src/engine/prehistoric/core/scene/world/WorldSerialiser.cpp), [EditorLayer.cpp](../PrehistoricEditor/src/EditorLayer.cpp).

## Proposed resource ownership and migration

Use one engine-defined resource resolver with separate roots for engine defaults, shared sample content, project
content, generated shader binaries, and writable caches/logs. A single API does not require every asset to belong to the
engine.

A suitable source layout would be `PrehistoricEngine/res/` for built-in fonts, shaders, primitive models and default
textures; `samples/res/` for shared demonstration assets; and application/project resource roots for settings, worlds
and scripts. Deployment should resolve these roots relative to an explicit launch configuration or installed bundle,
rather than assuming a repository-relative working directory. Names such as `engine:`, `sample:` and `project:` could
express ownership in serialized references; this is a proposed interface, not an existing feature.

1. Introduce the resolver and explicit root configuration. Cover direct font, shader, icon, model and texture reads,
   plus references read from configurations and worlds. Merely replacing the asset manager's path macros is
   insufficient.
2. Separate all writes from shared resources: logs, world saves, PMF/PRT conversion, script outputs and generated
   SPIR-V. Preserve original asset-relative paths in cache keys to avoid basename collisions.
3. Move fonts and selected defaults first. Resolve the PBR formula conflict, then centralize shader sources and add
   portable compilation. Keep optional experimental shaders explicitly classified.
4. Deduplicate shared sample models, textures and sounds as a complete dependency set. Preserve project settings/worlds
   and migrate their references without changing scene selection or backend settings.
5. Update both application launch configurations and Windows installer scripts. The current installers copy each
   application's `res/` tree and use fixed source-machine paths; a new engine root will not be packaged automatically.
6. Validate both apps from a clean checkout and an installed/staged layout outside the repository: shader loading,
   fonts/icons, default materials, model/MTL textures, configured environments, audio, world load/save and conversion
   output. Check concurrent logs, a read-only engine root, case-sensitive paths, and a second no-op shader build.
   Exercise optional engine modules separately rather than assuming startup covers FFT, water, or terrain.

The safest first consolidation is **fonts and shaders**, followed by **engine default models/textures**. The largest
disk saving comes from a **shared sample texture/audio package**. Settings, scripts, worlds, and profiling should not be
flattened into that package.
