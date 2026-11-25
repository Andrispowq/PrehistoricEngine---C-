# Rendering backlog

Step 4 established the [shared renderer frame order](../BUILDING.md#renderer-frame-order).

Step 5 added emissive bloom to Vulkan before HDR tone mapping.

Both renderers composite spherical volumetric media before tone mapping. Volumetric
spheres are editable scene components, and the
[Sponza world](../BUILDING.md#volumetric-lighting-test) provides two examples.

OpenGL and Vulkan water are enabled in the sample. Vulkan selects its geometry or tessellation-only pipeline from device capabilities. Native OpenGL GPU captures exercised the surface, reflection and refraction above and below the waterline. Set `water.reflectionParameters.underwaterPostProcessing` to `false` to disable the underwater tint and blur.

## Remaining work

7. Compare the existing Vulkan terrain against the corrected OpenGL terrain at matched camera positions, including LOD
   transitions and wireframe.
9. Verify editor parity on native Vulkan and OpenGL desktops, including camera input, native file dialogs and moving
   panels between displays with different scale factors.

10. Bake IBL in the background while rendering with the last completed maps. Capture one sky snapshot per bake, coalesce changes received during a bake, publish the complete map set after GPU completion, and retain old images until in-flight draws finish. Verify frame times and resource lifetime on both backends.

11. Add procedural terrain rendering.
12. Reactivate terrain collisions.
13. Improve physics.
14. Improve scripting.

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
   at MSAA edges. Voxel-based sun visibility depends on the voxel rendering port.
3. **Voxel rendering.** Vulkan has a cached 128³ occupied-voxel debug view for triangle meshes. Next port the occluder
   and radiance volumes, emissive and sun
   injection, diffuse bounce propagation, isotropic and six-direction mip chains, full- and half-resolution cone tracing
   and surface-aware composite. Include voxel sun and point-light visibility, grid and memory limits, invalidation,
   editor controls, shortcuts and stage timings. Use Midnight Sponza with bloom disabled.
   See [the VXGI plan](VXGI_PLAN.md) for the OpenGL baseline.
4. **Forward+ and surface buffers.** OpenGL shades materials in its main forward pass. Vulkan needs the depth prepass
   and tiled point-light lists, plus the position/metallic, albedo/roughness,
   and normal outputs consumed by screen-space effects. Introduce each attachment with its first consumer. Bloom, tone
   mapping, exposure, gamma, and the editor scene target already exist on Vulkan.

### Engine maintenance

- Preserve named sub-objects from OBJ and native model imports, then expose their transforms and materials in the editor. Verify the scene round-trip on Sponza curtains. The current native loader uses PMF. Include PRM support if it refers to a separate format.

- Review the world file format and loader/editor round-trip. Compare existing worlds for schema consistency, asset references, defaults, validation and versioning. Propose improvements and a migration path that preserves existing worlds.

- Migrate native vector, quaternion, and matrix operations to GLM incrementally behind the existing math API. Start with matrix multiplication and inversion. Preserve transform order, angle and clip-space conventions, and GPU and scripting layouts. Run the [math suite](../tests/unit/math/MathTests.cpp) and smoke-test both renderers before changing public types.

### Vulkan editor

The [editor](../PrehistoricEditor/src/EditorApp.cpp) defaults to OpenGL. Select
`Vulkan` in the [editor settings](../PrehistoricEditor/res/config/settings.json)
to use the Vulkan backend. Both paths open Midnight Sponza. The OpenGL editor enables voxel GI on supported hardware.

The [editor panels](../PrehistoricEditor/src/EditorLayer.cpp) share the scene
hierarchy, inspector, File menu, statistics, texture browser and environment
controls. A first-run layout docks the panels around the scene. Saved ImGui
layouts take precedence on subsequent launches.

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
