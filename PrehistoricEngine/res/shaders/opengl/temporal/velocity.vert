#version 430
layout(binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
uniform mat4 current;
uniform mat4 previous;
layout(location = 0) in vec3 position;
layout(location = 0) out vec4 currentClip;
layout(location = 1) out vec4 previousClip;
void main()
{
    currentClip = current * vec4(position, 1);
    previousClip = previous * vec4(position, 1);
    gl_Position = currentClip;
    gl_Position.xy += 2 * params.jitter.xy * gl_Position.w;
}
