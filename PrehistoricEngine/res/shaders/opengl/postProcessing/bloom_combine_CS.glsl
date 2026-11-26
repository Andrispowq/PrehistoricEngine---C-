#version 430

layout (local_size_x = 4, local_size_y = 4) in;

layout (binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D image0;
uniform sampler2D image1;
uniform vec2 targetSize;

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(targetSize)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / targetSize;

    vec4 colour0 = texture(image0, texCoord);
    vec3 colour1 = texture(image1, texCoord).rgb;

    imageStore(outColour, x, vec4(colour0.rgb + colour1, colour0.a));
}
