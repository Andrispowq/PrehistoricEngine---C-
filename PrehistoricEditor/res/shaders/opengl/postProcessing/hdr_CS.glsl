#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

uniform float gamma;
uniform float exposure;
uniform vec2 screenSize;

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / screenSize;

    vec3 sceneColour = texture(scene, texCoord).rgb;
       
    sceneColour = 1.0 - exp(-sceneColour * exposure);
    sceneColour = pow(sceneColour, vec3(1.0 / gamma));

    imageStore(outColour, x, vec4(sceneColour, 1.0));
}