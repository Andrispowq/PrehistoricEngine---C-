#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

uniform int underwater;
uniform float gamma;
uniform float exposure;
uniform vec2 resolution;

#include "shared/postProcessing/hdr.glsl"

#include "shared/water/underwater.glsl"

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(resolution)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / resolution;

    vec3 sceneColour = texture(scene, texCoord).rgb;
       
    if (underwater != 0) sceneColour = underwaterColour(scene, texCoord, resolution, sceneColour);
    sceneColour = ToneMapHDR(sceneColour, exposure, gamma);

    imageStore(outColour, x, vec4(sceneColour, 1.0));
}