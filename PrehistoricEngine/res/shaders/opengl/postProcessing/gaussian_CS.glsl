#version 430

layout (local_size_x = 4, local_size_y = 4) in;

layout (binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;
uniform int horizontal;
uniform vec2 targetSize;
uniform vec2 sceneSize;

#define KERNEL_SIZE 4

/*0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216*/
uniform float weights[KERNEL_SIZE] = float[](0.214606428562373, 0.1898792328888381, 0.13151412084312236, 0.07130343198685299);

void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(x, ivec2(targetSize)))) return;
    vec2 texCoord = (vec2(x) + 0.5) / targetSize;

    vec2 stepUV = horizontal == 1 ? vec2(1.0 / sceneSize.x, 0.0) : vec2(0.0, 1.0 / sceneSize.y);
    vec3 combined = texture(scene, texCoord).rgb * weights[0];
    for (int i = 1; i < KERNEL_SIZE; ++i)
    {
        vec2 offset = stepUV * i;
        combined += (texture(scene, texCoord + offset).rgb + texture(scene, texCoord - offset).rgb) * weights[i];
    }

    imageStore(outColour, x, vec4(combined, 1.0));
}
