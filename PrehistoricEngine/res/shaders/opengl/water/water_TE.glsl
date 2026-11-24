#version 430

layout (quads, fractional_odd_spacing, ccw) in;

in vec2 mapCoord_TE[];

out vec2 mapCoord_GS;

uniform int tiling;

uniform sampler2D Dy;
uniform sampler2D Dx;
uniform sampler2D Dz;
uniform float scaleY;
uniform float waterHeight;

uniform vec2 wind;
uniform float motion;
uniform float displacementScale;
uniform float choppiness;
uniform int displacementRange;
uniform vec3 cameraPosition;

#include "shared/terrain/quadEvaluation.glsl"

void main()
{
    vec4 p;
    vec2 uv;
    evaluateTerrainQuad(p, uv);
    mapCoord_GS = uv * tiling + wind * motion;
    float fade = clamp(1.0 - distance(p.xyz, cameraPosition) / max(float(displacementRange), 1.0), 0.0, 1.0);
    p.y = waterHeight + textureLod(Dy, uv, 0).r * scaleY;
    p.y += fade * textureLod(Dy, mapCoord_GS, 0).r * displacementScale;
    if (choppiness != 0.0)
    {
        p.xz -= fade * choppiness * vec2(textureLod(Dx, mapCoord_GS, 0).r, textureLod(Dz, mapCoord_GS, 0).r);
    }
    gl_Position = p;
}
