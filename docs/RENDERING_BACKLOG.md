# Rendering backlog

1. Fix the reported OpenGL resize crash involving `_shadowMap`. Check the shadow texture lifetime when the framebuffer is recreated and verify both PBR and volumetric consumers after repeated resizes. The report concerns OpenGL.
2. Repair OpenGL water rendering. Verify reflection, refraction, and the rendered water surface in a water-enabled scene.
