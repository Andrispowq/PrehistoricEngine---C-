#version 430

layout(location = 0) out vec4 outColour;

in vec2 texture_FS;

uniform sampler2D image0;
uniform sampler2D image1;

void main()
{
    vec3 colour0 = texture(image0, texture_FS).rgb;
    vec3 colour1 = texture(image1, texture_FS).rgb;

    outColour = vec4(colour0 + colour1, 1.0);
}