# Rendering backlog

Step 4 established the [shared renderer frame order](../BUILDING.md#renderer-frame-order).

Step 5 added emissive bloom to Vulkan before HDR tone mapping.

Both renderers composite spherical volumetric media before tone mapping. Volumetric
spheres are editable scene components, and the
[Sponza world](../BUILDING.md#volumetric-lighting-test) provides two examples.

OpenGL and Vulkan water are enabled in the sample. Vulkan selects its geometry or tessellation-only pipeline from device capabilities. Native OpenGL GPU captures exercised the surface, reflection and refraction above and below the waterline. Set `water.reflectionParameters.underwaterPostProcessing` to `false` to disable the underwater tint and blur.

The [render quality review](RENDER_QUALITY_REVIEW.md) contains five editor scenes, measured edge cases, and comparison
frames.

## Remaining work

Specialised duplication now covers terrain, water, orbiting suns and GUI elements/buttons/sliders, including
instance settings, generated-resource ownership, authored attachments and world round-trips. See
[duplication contracts and limitations](../BUILDING.md#editor-render-settings). Native Vulkan multi-instance and
world-load checks are covered; verify native OpenGL on a supported PC. macOS OpenGL compatibility is deferred below.

**Camera-following voxel GI: stage one implemented.** A single snapped volume retains mesh coverage while
scrolling and voxelises newly exposed regions. Lighting still refreshes across the volume; scene edits and
teleports rebuild it. See [scope and checks](../BUILDING.md#camera-following-voxel-volume). Next: multiple
resolution levels, transition blending, partial lighting updates and native OpenGL GPU validation.

**Terrain voxelisation and GI reception.** Optional terrain coverage and terrain/water GI reception are implemented,
with native Vulkan smoke checks; see [scope and checks](../BUILDING.md#terrain-voxel-contribution-and-water-reception).
Terrain contribution and reception are separate opt-in settings, both off by default.
Water is a lighting receiver only. Defer animated water voxelisation because rebuilding ocean coverage each frame
would add substantial cost. Remaining quality checks include complex heightfields, LOD seams, native OpenGL parity
and representative update timings.

**Voxel-based refraction.** Research transmission through the voxel representation for water and refractive
materials, including refractive index, thickness, absorption, entry/exit surfaces and off-screen radiance. Define
how it combines with existing refraction captures and what happens outside the voxel volume. Compare visual quality,
leakage and performance against the current refraction path; caustics are a separate decision.

**Volumetric-effects toggle.** Add a live Render Settings control to disable volumetric effects without deleting
scene objects. Define its interaction with the existing sun-shaft control, persist the chosen setting, skip disabled
GPU passes and invalidate affected temporal history. Verify spheres, shafts and underwater composition on both backends.

Environment Map GameObjects and exclusive atmosphere/environment ownership are implemented. See
[editor settings and world migration](../BUILDING.md#editor-render-settings). Local probes and spatially varying
lighting remain separate future work; environment position and scale do not affect distant lighting.

**Live rendering-backend switching (planned).** Allow switching between OpenGL and Vulkan only when both are
supported on the host. This requires more than changing the API setting: preserve the scene, unsaved edits, camera,
selection and editor layout; drain GPU work; recreate the window/context or surface, renderer, backend-owned assets,
pipelines and ImGui resources; restore portable asset references; and invalidate temporal history and baked caches.
Probe capabilities before teardown and define rollback if target-backend initialization fails. Test repeated switches,
detached editor windows, resource lifetime and recovery on a machine supporting both APIs. Keep this as backlog work;
the current API setting still takes effect at application startup.

Engine mesh raycasting and editor viewport picking are implemented. Set **Pick distance** in the editor's Render
Settings. Click once to select an instance and click it again to select a submesh. Clicking empty space clears the
selection.

Selected meshes have an amber outline in the lit viewport. Object selection outlines the instance as a whole,
and submesh selection outlines only the selected part.

Sunlight scatters through volumetric spheres using the directional shadow visibility. **Render Settings** exposes
shaft strength and the intensity and size of the seven-sprite sun lens flare. Both effects default to enabled.

**Depth of field** is available in Render Settings. Focus distance and aperture diameter use world units.
The editor saves them with the scene. Existing scenes start with the effect disabled.

Render Settings includes bloom intensity and optional screen-space ambient occlusion for PBR meshes.
The scene stores the bloom multiplier, AO radius in world units, and AO strength.

The **Screen-space reflections** control enables current-frame glossy reflections. The Sponza midnight
mirror scene enables it by default. Reliable screen hits replace the environment or voxel specular
term. Hits fade near screen edges and on rough surfaces. With a shadow-only voxel grid, AO samples missing
screen directions within its configured radius. With VXGI, diffuse cones already integrate off-screen
visibility, so SSAO contributes local contact detail.

**Motion blur and temporal antialiasing** are opt-in on both backends. Camera and rigid-mesh motion vectors,
depth-aware blur, jittered HDR accumulation and history rejection share shader math. See the
[implementation and validation guide](MOTION_TEMPORAL.md) for supported geometry, diagnostics, resource costs,
and remaining native OpenGL verification.

1. **Render Settings organisation completed.** Seven focused submenus now contain rendering, environment baking,
   diagnostics and interaction controls. HDR selection lives in Environment Map object Properties.
2. **Research variable-sized voxels.** Compare sparse voxel octrees, sparse bricks, and per-mesh
   resolution selected from geometric detail against the current uniform base grid and occupancy mips.
   Measure memory, traversal cost, rebuild cost for moving meshes, and light leakage across resolution
   boundaries at equal budgets in the diagnostic scenes. Evaluate filtering radiance between levels
   separately from skipping empty space during traversal. Compare the cascaded sparse distance fields in
   [AMD’s Brixelizer GI sample](https://gpuopen.com/manuals/fidelityfx_sdk/samples/brixelizer-gi/) with
   [Laine and Karras](https://research.nvidia.com/sites/default/files/pubs/2010-02_Efficient-Sparse-Voxel/laine2010tr1_paper.pdf)
   and the filtered hierarchy
   of [Crassin et al.](https://research.nvidia.com/labs/rtr/publication/crassin2011givoxels/).

Imported OBJ and PMF submeshes appear under each model instance in the editor
hierarchy. Selecting a part opens its material controls. Edits and resets apply
to that instance, and world JSON stores edited parts with the model renderer.
Parts use the parent model transform. Individual submesh transforms remain open.

7. Compare the existing Vulkan terrain against the corrected OpenGL terrain at matched camera positions, including LOD
   transitions and wireframe.
9. Verify editor parity on native Vulkan and OpenGL desktops, including camera input, native file dialogs and moving
   panels between displays with different scale factors.

10. Bake IBL in the background while rendering with the last completed maps. Capture one sky snapshot per bake, coalesce changes received during a bake, publish the complete map set after GPU completion, and retain old images until in-flight draws finish. Verify frame times and resource lifetime on both backends.

11. Add procedural terrain rendering.
12. Reactivate terrain collisions.
13. Improve physics.
14. Improve scripting.
15. Add animation support.

### Vulkan rendering parity

1. **Ocean.** Both backends run the FFT, generate normals, tessellate quadtree patches, capture reflection and
   refraction, and apply underwater HDR effects. Compare at the waterline with bloom on and off, after resize, and with
   MSAA on and off. The renderer uses the same 1024² capture size as OpenGL. The shaders use nearest depth lookup,
   include bloom throughout the underwater blur, and share the blue wireframe color. Vulkan wireframe requires non-solid
   fill support. Vulkan still stores depth in refraction color alpha and resolves it with color under MSAA. OpenGL
   samples a depth texture.
2. **Shadows and volumetrics.** Vulkan PBR meshes now use the directional sun and seven stabilized shadow layers.
   Both backends share cascade placement, blending and contact-hardening filtering. The matching atmosphere bake
   excludes the directly lit solar disk. Vulkan draws each layer separately on devices including Apple Silicon.
   Vulkan composites the editable spheres after bloom and before underwater effects and tone mapping. Both backends
   share the volume integration shader. Vulkan clips rays against sampled scene depth, using the nearest depth sample
   at MSAA edges. Voxel sun visibility is available for meshes and media.
3. **Voxel rendering.** Vulkan voxelizes opaque mesh triangles, bakes emissive and directional light, builds the
   occlusion and radiance hierarchies and propagates diffuse bounces. The editor exposes the occupied grid,
   directional mips, voxel sun shadows, full- and half-resolution tracing, surface filtering and GPU timings.
   See [Vulkan VXGI](../BUILDING.md#vulkan-voxel-global-illumination) and the [OpenGL baseline](VXGI_PLAN.md).
4. **Forward+ and surface buffers.** OpenGL shades materials in its main forward pass. Vulkan needs the depth prepass
   and tiled point-light lists, plus the position/metallic, albedo/roughness,
   and normal outputs consumed by screen-space effects. Introduce each attachment with its first consumer. Bloom, tone
   mapping, exposure, gamma, and the editor scene target already exist on Vulkan.

### Engine maintenance

- Review the world file format and loader/editor round-trip. Compare existing worlds for schema consistency, asset references, defaults, validation and versioning. Propose improvements and a migration path that preserves existing worlds.

- Migrate native vector, quaternion, and matrix operations to GLM incrementally behind the existing math API. Start with matrix multiplication and inversion. Preserve transform order, angle and clip-space conventions, and GPU and scripting layouts. Run the [math suite](../tests/unit/math/MathTests.cpp) and smoke-test both renderers before changing public types.

### Vulkan editor

The [editor](../PrehistoricEditor/src/EditorApp.cpp) defaults to OpenGL. Select
`Vulkan` in the [editor settings](../PrehistoricEditor/res/config/settings.json)
to use the Vulkan backend. Both paths open Midnight Sponza. Both editors enable voxel GI on supported hardware.

The [editor panels](../PrehistoricEditor/src/EditorLayer.cpp) share the scene
hierarchy, inspector, File menu, statistics, texture browser and environment
controls. A first-run layout docks Scene Hierarchy on the left, with a visible
Stats panel below Properties on the right of the main window. The object list occupies the left side.
Saved Environment docks migrate to Properties. Stats remains docked and
cannot be covered by another panel's tab.
The editor keeps rendering when focus moves to another window; minimization still pauses it.

The [Vulkan renderer](../PrehistoricEngine/src/engine/platform/vulkan/rendering/VKRenderer.cpp)
tone-maps the editor scene into a sampleable image. The main window draws
ImGui into the acquired swapchain image, then detached windows can sample the
same scene. The sample application still presents its tone-mapped scene directly.
The [Vulkan ImGui integration](../PrehistoricEngine/src/engine/platform/vulkan/rendering/VKImGui.cpp)
handles fonts, texture descriptors and platform-window pipelines.

For native verification, open the same world on both backends. Compare viewport
orientation, material and light edits, texture thumbnails, HDR reload, environment
rebake, preview LOD, exposure and gamma. Save and reopen the edited scene, then
detach the Viewport, resize both windows and dock it again.

A headless SwiftShader editor smoke run with reduced bake sizes reloaded all
15 test-scene objects, rendered 14 scene draws, recreated the scene target on
resize, detached the Viewport and exercised Save As, New and Open using test
dialog paths. Frame captures showed the scene and asset thumbnails. The run
completed with Vulkan validation enabled and no reported validation errors.
Native input, file pickers and a visual OpenGL comparison remain open.

### Deferred — end of backlog

**Native OpenGL on macOS — deferred; lowest priority.** Revisit only if a concrete need for native macOS OpenGL
justifies the substantial compatibility rewrite. OpenGL can be tested on PC, and the existing Vulkan backend
provides the near-equivalent macOS rendering path.

If revisited, add a capability-driven OpenGL 4.1 core path for the sample and editor.
Audit GLSL versions, compute shaders, image load/store, shader-storage buffers, direct-state-access calls and
resource limits; provide compatible raster/CPU alternatives or explicitly unavailable optional effects. Lowering
the requested context version alone is insufficient. Preserve the existing desktop OpenGL path and Vulkan behavior.
Validate first rendered frames, scene loading, IBL, terrain/water, editor docking and supported AA/post-processing on
macOS. Reference [GLFW platform constraints](https://www.glfw.org/docs/latest/compat.html); the
[macOS port assessment](MACOS_PORT.md) is historical and must be checked against current code.
