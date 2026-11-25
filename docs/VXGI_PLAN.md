# VXGI improvement plan

Use [Midnight Sponza](../PrehistoricEditor/res/world/sponza_midnight_gi.json) as the emissive-light benchmark. The
OpenGL editor opens it with VXGI enabled on supported hardware, a 256³ grid subject to the GPU memory budget, four
diffuse bounces, and directional mips. Half-resolution tracing is optional. The fixed sun sits below the planet, so the
three emissive cubes and one ground-floor curtain supply the voxel radiance. The startup camera disables bloom so the
curtain and mirror silhouette remain visible together.
Use [Milky Way Sponza](../Prehistoric/res/world/sponza_night_gi.json) for the HDR panorama comparison.

## References

- [IDKEngine VXGI showcase](https://www.youtube.com/watch?v=5m9fOVWaqdE)
- [IDKEngine source and voxelization write-up](https://github.com/BoyBaykiller/IDKEngine)
- [José Villegas, Deferred Voxel Shading for Real Time Global Illumination](https://jose-villegas.github.io/post/deferred_voxel_shading/)

IDKEngine describes rasterizing from three axes without a geometry shader, combining radiance with per-channel atomics, and optional vendor-specific acceleration. Its cone tracer exposes temporal sampling controls. Prehistoric uses a dominant-axis geometry shader and picks a bright representative voxel with a packed atomic max. Villegas stores voxel material attributes separately from lighting, retains static geometry when dynamic geometry changes, and traces diffuse and specular cones through directional radiance mips. [IDKEngine voxelization](https://github.com/BoyBaykiller/IDKEngine#voxel-global-illumination) · [IDKEngine cone tracer](https://github.com/BoyBaykiller/IDKEngine/blob/master/IDKEngine/Source/Render/VXGI/ConeTracing/ConeTracer.cs) · [Deferred voxel shading](https://jose-villegas.github.io/post/deferred_voxel_shading/)

## Implementation order

1. **Establish a GPU baseline.** Use the editor's **Voxel GPU Timings** to measure the occluder bake, radiance injection, bounce propagation, final mip generation, cone tracing, and filter/composite. The timers are in place. Record a repeatable fixed-camera and moving-camera sequence with bloom off. Keep output and voxel resolution, grid bounds, bounce count, GI strength, and exposure fixed. Collect warm frame samples for GI on and off, and measure cold bakes separately from light changes.
2. **Reduce per-frame tracing cost.** The half-resolution prototype traces once per 2×2 block and upsamples using surface position and normal. Compare it against full-resolution tracing around silhouettes, floor detail, and thin curtains. Test IDKEngine-style temporal sampling with reprojection and history rejection for disocclusion, camera cuts, or radiance changes. Measure cost, stability, and ghosting before changing the default. [IDKEngine cone tracer](https://github.com/BoyBaykiller/IDKEngine/blob/master/IDKEngine/Source/Render/VXGI/ConeTracing/ConeTracer.cs)
3. **Improve directional radiance filtering.** Six half-resolution mip chains now composite near voxels over far voxels and are used for bounce propagation and the final trace. Keep the isotropic toggle for A/B tests. Measure indirect energy on both sides of a solid wall or curtain from matched cameras and compare an exposed receiver alongside it to distinguish leaks from changes in overall bounce energy. Record bake time, trace time, and VRAM at matched grid and bounce settings. [Villegas's anisotropic voxels](https://jose-villegas.github.io/post/deferred_voxel_shading/#3-anisotropic-voxels)
4. **Add roughness-aware indirect specular.** Use the existing normal, roughness, metallic, and view direction to trace
   a reflection cone. Use the polished metal sphere between the ground-floor curtains. The nearby fabric has an emission
   override of 400. Compare the sphere at zero and increasing roughness. Check energy when composing voxel reflections
   with environment reflections and measure the extra GPU
   cost. [Villegas's voxel cone tracing](https://jose-villegas.github.io/post/deferred_voxel_shading/#4-voxel-cone-tracing)
5. **Decouple voxelization from relighting.** Retain static voxel surface attributes and reinject radiance when a light changes. Define invalidation for geometry, transform, and material edits, then test overlapping static and dynamic surfaces. Benchmark the existing dominant-axis geometry shader against IDKEngine's three-axis raster path with matched coverage, resolution, and collision rules. Time rasterization, clearing, resolve, and relighting separately before choosing a path. [Villegas's deferred voxel shading](https://jose-villegas.github.io/post/deferred_voxel_shading/) · [IDKEngine voxelization](https://github.com/BoyBaykiller/IDKEngine#voxel-global-illumination)

Diffuse cones on both backends sample the baked atmosphere or HDR cubemap when they exit the grid, weighted by remaining
visibility. The first propagation step includes sky light, and later steps propagate the resulting bounce. Valid traced
receivers replace the diffuse environment contribution. The PBR environment specular term still supplies the mirror
sphere.

## Measurements so far

At the default Midnight Sponza camera with 256³ and four bounces, a half-resolution spot check reduced cone tracing from 2.86 ms to 0.87 ms. The foreground floor showed more mottling, so full resolution remains the starting setting.

The directional A/B used the same startup camera, 256³, four bounces, full-resolution tracing, and bloom off. These warmed snapshots show GPU stage costs. Propagation includes intermediate mips.

| GPU stage | Isotropic | Directional |
| --- | ---: | ---: |
| Bounce propagation | 17.78 ms | 25.49 ms |
| Final radiance mips | 0.24 ms | 2.21 ms |
| Cone tracing per frame | 3.70 ms | 4.35 ms |

Directional chains add about 110 MiB at 256³. In a controlled OpenGL readback across all six directions, isotropic downsampling returned half the emitter's radiance through an opaque voxel layer. Directional downsampling returned zero from the blocked side and full radiance from the exposed side. Directional filtering brightened Sponza's aisle and far wall. The fixed-camera comparison leaves leakage through scene curtains and stone unquantified.
