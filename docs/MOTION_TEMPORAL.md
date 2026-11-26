# Motion blur and temporal antialiasing

Both backends have independent, opt-in motion blur and native-resolution temporal
antialiasing controls under **Render Settings → Motion blur and temporal AA**.
Existing worlds default to both effects disabled. Temporal processing pauses in wireframe and voxel debug views. The
optional `temporal` world
object stores `motion_blur`, `antialiasing`, `shutter_angle`, `max_blur_pixels`, and
`samples`. Debug views, timing results, and history are not serialized.

## Frame and resource contract

The renderer snapshots the main camera and instance world transforms after scene
submission. Water captures use an unjittered projection and do not advance this
state. History advances after submission, not on simulation updates or swapchain
image acquisition. Vulkan descriptor/constant-buffer updates use the acquired
image's fence-protected slot; history images alternate by submitted main frame.
Render-pass dependencies order previous sampling before attachment reuse.

Motion is `previousUV - currentUV`, excluding raster jitter. The dedicated mesh
pass writes motion plus current and predicted previous positive view depth. At
MSAA edges, the resolve selects one nearest sample's complete record rather than
averaging velocities or mixing velocity from one surface with depth from another.
The current implementation packs this record in RGBA32F to retain depth precision;
it does not yet split velocity into RG16F and depth into separate attachments. If the requested multisample count is
unavailable for RGBA32F, the geometry target uses RGBA16F; resolved history metadata remains RGBA32F.
Current scene depth rejects unsupported surfaces that occlude the velocity pass.
OpenGL uses its existing position buffer for this comparison; Vulkan samples the
main scene depth attachment. Sky motion uses camera rotation only.

Stable component identities prevent deleted/recreated objects from inheriting
history through pointer reuse. New objects, projection/resolution changes, TAA
mode changes, long gaps between submissions, and large camera/object jumps reject
history. `Renderer::ResetTemporalHistory()` explicitly marks camera cuts or scene
changes. `RenderableComponent::InvalidateMotionHistory()` marks object teleports,
mesh replacement, or discontinuous deformation; changing its pipeline calls this
automatically. Automatic jump detection uses a conservative 50-world-unit
translation threshold; callers should explicitly invalidate smaller teleports.

## Processing

1. Resolve camera and rigid-mesh motion/depth and validity.
2. If TAA is enabled, reconstruct the current image on an unjittered pixel grid,
   reproject the HDR history, and compare stored historical depth against the
   surface's predicted previous depth. Reject missing/out-of-bounds history.
   Foreground velocity dilation, neighborhood variance clipping in compressed HDR,
   and reduced history weight for motion and shading changes limit ghosting.
3. Build 16-pixel velocity tiles and a radius-aware neighborhood maximum. Gather
   HDR samples along the current and dominant motion with depth/reach weighting.
   A 180-degree shutter integrates half the rendered-frame interval. Maximum blur
   is a radius in pixels, bounded to 64; sample count is bounded to 4–32.
4. Continue existing bloom, volumetric, underwater, DoF and tone-mapping processing.
   Current-frame selection alpha survives both filters, and UI is drawn afterwards.

The TAA history is captured before blur and lens/post effects. Diagnostic views
never feed their colors back into history. History uses bilinear filtering, which avoids negative-lobe ringing near
bright HDR emitters. Catmull–Rom filtering remains a live-image quality experiment, not an enabled option.

GPU timings expose motion geometry, motion resolve, TAA, and blur/diagnostics.
Queries are read only after availability/fence checks. Target memory estimates
exclude driver alignment, descriptors and small constant buffers. The initial
RGBA32F motion representation and multisampled geometry target have a substantial
memory cost; compare MSAA-off TAA against combined MSAA/TAA before enabling both.

## Limits

This is a screen-space approximation. It cannot reconstruct hidden surfaces,
correct nonlinear trajectories, or the independent motion of reflected content.
Rigid meshes and camera/sky motion are supported. Water/terrain deformation,
skeletal animation and transparent multi-layer motion do not have dedicated
velocity implementations; unmatched depth and transparent mesh records reject
history and blur rather than inherit background motion. Reflections and changing
shading use conservative color-based history weighting, not specular motion
vectors. Volumetrics remain outside temporal accumulation. No temporal upscaling,
frame generation, or dedicated GI/SSR denoiser is included.

## Validation

Run the full build and resource checks from [BUILDING.md](../BUILDING.md), plus:

```sh
cmake --preset tests
cmake --build --preset tests --parallel 4
ctest --preset tests
python3 tests/temporal_shader_contract.py
python3 tests/temporal_gpu_contract.py
python3 tests/temporal_render_smoke.py --api Vulkan --app PrehistoricEditor --samples 1
python3 tests/temporal_render_smoke.py --api Vulkan --app PrehistoricEditor --samples 8
python3 tests/temporal_render_smoke.py --api Vulkan --app Prehistoric --samples 1
python3 tests/temporal_render_smoke.py --api OpenGL --app Prehistoric --samples 1
```

The shader check compiles all OpenGL/Vulkan temporal entry points, validates SPIR-V,
and checks std140 offsets. CPU tests cover motion signs, camera/object cancellation,
jitter, previous surface depth, submitted-frame history, resize, new identities and
teleports. These checks do not constitute full GPU renderer validation.
The GPU numerical contract dispatches the production shared filter functions with
analytic texture inputs through `VulkanComputeSmoke`. It checks 256 cases twice,
including coherent MSAA sample selection, history rejection/acceptance, alpha
preservation and a translating strip against an analytic exposure reference
(absolute intensity error below 0.06). This isolates algorithm behavior; the
application smoke tests exercise real sampled images and pipeline interfaces.

The native smoke builds a scratch executable against `build/debug` using Ninja's
actual compile/link commands. It copies configuration and worlds into a temporary
working tree, reduces environment bake sizes, and exercises 41 rendered frames.
Use `--no-gi` to isolate temporal processing, `--world` to choose a diagnostic world,
or `--water` to add a waterline/bloom/DoF stress test. `--benchmark` keeps the requested
window size and skips docking; the log records the actual framebuffer size and MSAA count. It checks world-settings
round-trip, moves the
camera and a mesh, resizes, toggles effects and wireframe, rebakes lighting, resets
history and exercises editor docking. On macOS it also minimizes and restores the
window. Vulkan requires the SDK validation and screenshot
layers; captures and logs are retained in the printed artifact directory. Native
window/GPU access is required. Inspect the captures in addition to reading logs.

## Verification record — 2026-11-26

- Full Debug engine, sample and editor build, including `EngineResources`: passed.
- CPU math/temporal suite: 34 tests passed.
- Temporal shader contracts: 12 OpenGL/Vulkan stages compiled; SPIR-V and uniform offsets passed.
- GPU numerical contract: 256 cases passed twice on Apple M1, zero Vulkan validation errors.
- Resource pipeline, Vulkan PBR and background contracts: passed.
- Native MoltenVK sample and editor scripted runs completed. Tested MSAA off and
  device-selected 4× MSAA (8× requested), camera/object movement, settings round-trip,
  resize, effect toggles, wireframe, docking, minimize/restore, environment rebake,
  explicit resets, and waterline crossings with volumetrics, bloom and DoF.
  Inspected captured frames; final runs reported zero Vulkan validation errors.
- Native OpenGL startup is blocked: the engine requests 4.5, but this macOS host
  supplies 4.1. OpenGL source/shader checks are not runtime parity verification.
- Native Vulkan startup also logs the existing Cocoa message that regular windows
  do not have icons; rendering continues. Explicit MoltenVK selection avoids an
  unrelated incompatible-architecture ICD loader error on this x86 Debug build.

Representative completed-frame GPU queries for Sponza on Apple M1/MoltenVK,
TAA and blur enabled, MSAA off (these are observations, not frame-rate guarantees):

| Render extent | Motion geometry |  Resolve |      TAA |     Blur |      Sum | Target memory |
|---------------|----------------:|---------:|---------:|---------:|---------:|--------------:|
| 1920×1080     |        0.459 ms | 0.218 ms | 0.861 ms | 2.133 ms | 3.671 ms |     150.4 MiB |
| 2560×1440     |        0.571 ms | 0.526 ms | 1.543 ms | 4.031 ms | 6.672 ms |     267.4 MiB |

A fixed-size 1440×840 water/volume test at 4× MSAA measured blur at a 1.950 ms median
across eight observations (1.892–2.369 ms). An earlier single observation was 35.609 ms;
use multiple observations rather than that outlier for budgeting. Target memory was
157.0 MiB in this MSAA case. Further memory packing and reducing full-resolution
passes are worthwhile follow-up optimizations. Cross-platform OpenGL visual parity,
Windows runs, and extensive motion-quality tuning remain unverified.

## References and choices

- [McGuire et al., A Reconstruction Filter for Plausible Motion Blur (2012)](https://www.casual-effects.com/research/McGuire2012Blur/index.html)
  and [KinoMotion](https://github.com/keijiro/KinoMotion): depth-aware reconstruction
  and velocity neighborhoods provide a practical raster baseline.
- [Yang, Liu and Salvi, A Survey of Temporal Antialiasing Techniques (2020)](https://www.leiy.cc/publications/TAA/TemporalAA.pdf):
  accumulation and history validation are separate responsibilities.
- [Microsoft's TAA sample](https://learn.microsoft.com/en-us/samples/microsoft/xbox-gdk-samples/temporalantialiasing/)
  and [NVIDIA Donut](https://github.com/NVIDIA-RTX/Donut): references for jitter,
  clipping, filtering and explicit history/resource lifetimes.
- [AMD FSR 3.1.4](https://gpuopen.com/manuals/fidelityfx_sdk/techniques/super-resolution-upscaler/):
  reactive content and disocclusion inform the input contract; SDK integration is
  outside this native-resolution milestone.
- [Reservoir Splatting (SIGGRAPH 2025)](https://research.nvidia.com/labs/rtr/publication/liu2025splatting/):
  recent motion-blur research, but its ReSTIR path reservoirs require a different
  rendering architecture.
