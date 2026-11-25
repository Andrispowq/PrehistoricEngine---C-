# Rendering backlog

Step 4 established the [shared renderer frame order](../BUILDING.md#renderer-frame-order).

Step 5 added emissive bloom to Vulkan before HDR tone mapping.

OpenGL composites spherical volumetric media before tone mapping. Volumetric
spheres are editable scene components, and the
[Sponza world](../BUILDING.md#opengl-volumetric-lighting-test) provides two examples.

OpenGL and Vulkan water are enabled in the sample. Vulkan selects its geometry or tessellation-only pipeline from device capabilities. Native OpenGL GPU captures exercised the surface, reflection and refraction above and below the waterline. Set `water.reflectionParameters.underwaterPostProcessing` to `false` to disable the underwater tint and blur.

## Remaining work

6. Fix terrain rendering issues in the existing OpenGL path.
7. Port terrain rendering to Vulkan and compare it against the corrected OpenGL terrain.
9. Verify editor parity on native Vulkan and OpenGL desktops, including camera input, native file dialogs and moving
   panels between displays with different scale factors.

10. Bake IBL in the background while rendering with the last completed maps. Capture one sky snapshot per bake, coalesce changes received during a bake, publish the complete map set after GPU completion, and retain old images until in-flight draws finish. Verify frame times and resource lifetime on both backends.

11. Add procedural terrain rendering.
12. Reactivate terrain collisions.
13. Improve physics.
14. Improve scripting.

### Engine maintenance

- Preserve named sub-objects from OBJ and native model imports, then expose their transforms and materials in the editor. Verify the scene round-trip on Sponza curtains. The current native loader uses PMF. Include PRM support if it refers to a separate format.

- Review the world file format and loader/editor round-trip. Compare existing worlds for schema consistency, asset references, defaults, validation and versioning. Propose improvements and a migration path that preserves existing worlds.

- Migrate native vector, quaternion, and matrix operations to GLM incrementally behind the existing math API. Start with matrix multiplication and inversion. Preserve transform order, angle and clip-space conventions, and GPU and scripting layouts. Run the [math suite](../tests/unit/math/MathTests.cpp) and smoke-test both renderers before changing public types.

### Vulkan editor

The [editor](../PrehistoricEditor/src/EditorApp.cpp) now defaults to Vulkan. Select
`OpenGL` in the [editor settings](../PrehistoricEditor/res/config/settings.json)
to run the same panels on a host with OpenGL 4.5. Both paths load
`res/world/testLevel.wrld` and add the sun.

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
