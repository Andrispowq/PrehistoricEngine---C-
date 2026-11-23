# Rendering backlog

Step 4 established the [shared renderer frame order](../BUILDING.md#renderer-frame-order).

Step 5 added emissive bloom to Vulkan before HDR tone mapping.

## Remaining work

6. Fix terrain rendering issues in the existing OpenGL path.
7. Port terrain rendering to Vulkan and compare it against the corrected OpenGL terrain.
8. Repair OpenGL water rendering. Verify reflection, refraction, and the rendered water surface in a water-enabled scene.
