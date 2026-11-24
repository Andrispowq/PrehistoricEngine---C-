#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

uniform int underwater;
uniform float gamma;
uniform float exposure;
uniform vec2 resolution;

#include "shared/postProcessing/hdr.glsl"

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(resolution)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / resolution;

    vec3 sceneColour = texture(scene, texCoord).rgb;
       
    if (underwater != 0)
    {
        vec2 stepUV = 2.0 / resolution;
        vec3 haze = (texture(scene, texCoord + vec2(stepUV.x, 0)).rgb +
                     texture(scene, texCoord - vec2(stepUV.x, 0)).rgb +
                     texture(scene, texCoord + vec2(0, stepUV.y)).rgb +
                     texture(scene, texCoord - vec2(0, stepUV.y)).rgb) * 0.25;
        sceneColour = mix(sceneColour, haze, 0.45) * vec3(0.45, 0.79, 0.84) + vec3(0.008, 0.035, 0.045);
    }
    sceneColour = ToneMapHDR(sceneColour, exposure, gamma);

    imageStore(outColour, x, vec4(sceneColour, 1.0));
}