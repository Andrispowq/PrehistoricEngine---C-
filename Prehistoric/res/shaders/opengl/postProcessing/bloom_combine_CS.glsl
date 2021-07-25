#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;

uniform sampler2D bloom0;
uniform sampler2D bloom1;
uniform sampler2D bloom2;
uniform sampler2D bloom3;
uniform sampler2D bloom4;

uniform float bloom_factor;
uniform vec2 screenSize;

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / screenSize;

    vec3 sceneColour = texture(scene, texCoord).rgb;

    sceneColour += texture(bloom0, texCoord).rgb;
    sceneColour += texture(bloom1, texCoord).rgb;
    sceneColour += texture(bloom2, texCoord).rgb;
    sceneColour += texture(bloom3, texCoord).rgb;
    sceneColour += texture(bloom4, texCoord).rgb;
    
    imageStore(outColour, x, vec4(sceneColour, 1.0));
}