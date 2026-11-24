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

#include "shared/water/displacement.glsl"

void main()
{
    vec4 p;
    vec2 uv;
    evaluateTerrainQuad(p, uv);
    displaceWater(p, uv, mapCoord_GS);
    gl_Position = p;
}
