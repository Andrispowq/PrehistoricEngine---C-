uniform int cascadeCount;
uniform float cascadePlaneDistances[16];
uniform float shadowNearPlane;
uniform float cascadeBlendFraction;
uniform mat4 shadowView;
uniform sampler2DArray shadowMap;

layout(std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 m_LightSpace[16];
};

float cascadeVisibility(vec3 position, int layer, float biasInTexels)
{
    mat4 lightMatrix = m_LightSpace[layer];
    vec4 lightPosition = lightMatrix * vec4(position, 1.0);
    vec3 coord = lightPosition.xyz / lightPosition.w * 0.5 + 0.5;
    if (any(lessThan(coord, vec3(0.0))) || any(greaterThan(coord, vec3(1.0))))
        return 1.0;

    ivec2 size = textureSize(shadowMap, 0).xy;
    float xScale = length(vec3(lightMatrix[0].x, lightMatrix[1].x, lightMatrix[2].x));
    float zScale = length(vec3(lightMatrix[0].z, lightMatrix[1].z, lightMatrix[2].z));
    float bias = biasInTexels * zScale / (xScale * float(size.x));

    vec2 texel = coord.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(texel));
    vec2 fraction = fract(texel);
    float visibility = 0.0;
    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            ivec2 pixel = clamp(base + ivec2(x, y), ivec2(0), size - 1);
            float storedDepth = texelFetch(shadowMap, ivec3(pixel, layer), 0).r;
            vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
            if (coord.z - bias <= storedDepth)
                visibility += weight.x * weight.y;
        }
    }
    return visibility;
}

float directionalVisibility(vec3 position, float biasInTexels)
{
    float depth = -(shadowView * vec4(position, 1.0)).z;
    if (depth < shadowNearPlane) return 1.0;

    int layer = 0;
    while (layer < cascadeCount && depth >= cascadePlaneDistances[layer])
        ++layer;

    float visibility = cascadeVisibility(position, layer, biasInTexels);
    if (layer == cascadeCount) return visibility;

    float near = layer == 0 ? shadowNearPlane : cascadePlaneDistances[layer - 1];
    float far = cascadePlaneDistances[layer];
    float blendStart = mix(far, near, cascadeBlendFraction);
    if (depth <= blendStart) return visibility;
    return mix(visibility, cascadeVisibility(position, layer + 1, biasInTexels),
               smoothstep(blendStart, far, depth));
}
