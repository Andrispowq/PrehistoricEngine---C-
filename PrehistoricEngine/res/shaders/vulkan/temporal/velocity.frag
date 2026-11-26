#version 450
#extension GL_GOOGLE_include_directive : require
layout(location = 0) in vec4 currentClip;
layout(location = 1) in vec4 previousClip;
layout(location = 0) out vec4 motion;
void main()
{
    bool valid = previousClip.w > 0 && currentClip.w > 0;
    vec2 velocity = valid ? (previousClip.xy / previousClip.w - currentClip.xy / currentClip.w) * 0.5 : vec2(0);
    motion = vec4(velocity, currentClip.w, valid ? previousClip.w : -1);
}
