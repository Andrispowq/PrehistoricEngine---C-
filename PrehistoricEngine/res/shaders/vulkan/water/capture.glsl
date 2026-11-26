layout(push_constant) uniform WaterCaptureView
{
    mat4 worldToClip;
    vec4 eyeAndCapture;
    uint selected;
} waterView;
vec4 sceneClip(mat4 projection, mat4 view, vec4 world)
{
    return waterView.eyeAndCapture.w != 0.0 ? waterView.worldToClip * world : projection * view * world;
}
