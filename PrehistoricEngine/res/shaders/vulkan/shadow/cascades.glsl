layout(set = SHADOW_SET, binding = 7) uniform sampler2DArray shadowMap;
layout(set = SHADOW_SET, binding = 8, std140) uniform ShadowParameters
{
    mat4 m_LightSpace[7];
    mat4 shadowView;
    vec4 splits[2];
    vec4 sunDirection;
    vec4 sunRadiance;
    vec4 shadowSettings;
};
const int cascadeCount = 6;
#define shadowNearPlane shadowSettings.x
#define cascadeBlendFraction shadowSettings.y
#define shadowSunAngularRadius shadowSettings.z
float cascadeSplit(int layer) { return splits[layer / 4][layer % 4]; }

#include "../../shared/shadow/cascades.glsl"

