#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;
uniform sampler2D bloom;

uniform float bloom_factor;
uniform vec2 screenSize;

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / screenSize;

    vec3 sceneColour = texture(scene, texCoord).rgb;
    vec3 bloomColour = texture(bloom, texCoord).rgb;

    if (dot(bloomColour, vec3(0.0)) != 0.0)
    {
        sceneColour = bloomColour * bloom_factor;
    }
    
    imageStore(outColour, x, vec4(sceneColour+bloomColour, 1.0));
}