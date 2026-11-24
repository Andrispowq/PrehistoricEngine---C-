#include "../pbr/camera.glsl"
#include "params.glsl"
layout(quads, fractional_odd_spacing, ccw) in;
layout(location=0) in vec2 mapCoord_TE[];
layout(set=2,binding=3) uniform sampler2D Dy;
layout(set=2,binding=4) uniform sampler2D Dx;
layout(set=2,binding=5) uniform sampler2D Dz;
#define scaleY waves.x
#define displacementScale waves.y
#define choppiness waves.z
#define displacementRange waves.w
#define wind animation.xy
#define motion animation.z
#define tiling animation.w
#define waterHeight optics.w
#include "../../shared/terrain/quadEvaluation.glsl"
#include "../../shared/water/displacement.glsl"
void evaluateWater(out vec4 world, out vec2 waveUV)
{
    vec2 uv;
    evaluateTerrainQuad(world, uv);
    displaceWater(world, uv, waveUV);
}
