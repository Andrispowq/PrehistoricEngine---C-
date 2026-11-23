#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColour;
layout(binding = 0) uniform sampler2D source;
layout(binding = 1) uniform sampler2D smaller;
layout(push_constant) uniform Settings
{
    vec2 stepUV;
    float combine;
} bloom;

void main()
{
    const float weights[] = float[](0.21460643, 0.18987923, 0.13151412, 0.07130343);
    vec3 colour = texture(source, uv).rgb;
    if (bloom.combine > 0.0)
    {
        colour += texture(smaller, uv).rgb;
    }
    else
    {
        colour *= weights[0];
        for (int i = 1; i < weights.length(); ++i)
        {
            vec2 offset = bloom.stepUV * i;
            colour += (texture(source, uv + offset).rgb + texture(source, uv - offset).rgb) * weights[i];
        }
    }
    outColour = vec4(colour, 1.0);
}
