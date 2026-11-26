#version 450
#extension GL_GOOGLE_include_directive : require
layout(set = 0, binding = 6, std140) uniform TemporalParameters
{
    mat4 reprojection;
    vec4 jitter;
    vec4 settings;
    vec4 options;
    vec4 depthRow;
} params;
layout(push_constant) uniform ObjectMotion { mat4 current; mat4 previous; } objectMotion;
layout(location = 0) in vec3 position;
layout(location = 0) out vec4 currentClip;
layout(location = 1) out vec4 previousClip;
void main()
{
    currentClip = objectMotion.current * vec4(position, 1);
    previousClip = objectMotion.previous * vec4(position, 1);
    gl_Position = currentClip;
    gl_Position.xy += 2 * params.jitter.xy * gl_Position.w;
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
}
