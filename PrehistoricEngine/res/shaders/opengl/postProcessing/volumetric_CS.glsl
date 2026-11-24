#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

const int MaxSpheres = 8;
const int StepsPerInterval = 24;
const float Pi = 3.141592653589793;

uniform sampler2D scene;
uniform sampler2D sceneDepth;
uniform sampler2DArray shadowMap;
uniform vec3 cameraPosition;
uniform mat4 inverseViewProjection;
uniform mat4 m_view;
uniform vec3 sunDirection;
uniform vec3 sunRadiance;
uniform int sphereCount;
uniform vec4 sphereBounds[MaxSpheres];
uniform vec4 sphereMedia[MaxSpheres];
uniform float sphereAnisotropy[MaxSpheres];

uniform int cascadeCount;
uniform float cascadePlaneDistances[16];
uniform float farPlane;
layout(std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 m_LightSpace[16];
};

float sunVisibility(vec3 position)
{
    float viewDepth = -(m_view * vec4(position, 1.0)).z;
    int layer = 0;
    while (layer < cascadeCount && viewDepth >= cascadePlaneDistances[layer])
        ++layer;

    vec4 lightPosition = m_LightSpace[layer] * vec4(position, 1.0);
    vec3 shadowCoord = lightPosition.xyz / lightPosition.w * 0.5 + 0.5;
    if (any(lessThan(shadowCoord, vec3(0.0))) || any(greaterThan(shadowCoord, vec3(1.0))))
        return 1.0;

    float cascadeFar = layer < cascadeCount ? cascadePlaneDistances[layer] : farPlane;
    float bias = 0.01 / max(cascadeFar, 1.0);
    ivec2 size = textureSize(shadowMap, 0).xy;
    vec2 texel = shadowCoord.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(texel));
    vec2 fraction = fract(texel);
    float visibility = 0.0;
    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            ivec2 samplePixel = clamp(base + ivec2(x, y), ivec2(0), size - 1);
            float storedDepth = texelFetch(shadowMap, ivec3(samplePixel, layer), 0).r;
            vec2 weight = mix(1.0 - fraction, fraction, vec2(x, y));
            if (shadowCoord.z - bias <= storedDepth)
                visibility += weight.x * weight.y;
        }
    }
    return visibility;
}

float phaseFunction(float cosine, float g)
{
    g = clamp(g, -0.95, 0.95);
    return (1.0 - g * g) / (4.0 * Pi * pow(1.0 + g * g - 2.0 * g * cosine, 1.5));
}

vec2 intersectSphere(vec3 origin, vec3 direction, vec4 sphere, float surfaceDistance)
{
    vec3 toCenter = sphere.xyz - origin;
    float centerDistance = dot(toCenter, direction);
    vec3 perpendicular = toCenter - direction * centerDistance;
    float discriminant = sphere.w * sphere.w - dot(perpendicular, perpendicular);
    if (discriminant <= 0.0) return vec2(0.0);

    float halfChord = sqrt(discriminant);
    return clamp(vec2(centerDistance - halfChord, centerDistance + halfChord), 0.0, surfaceDistance);
}

vec3 compositeVolumes(vec3 background, vec3 origin, vec3 direction, float surfaceDistance)
{
    vec2 intervals[MaxSpheres];
    vec3 scatter[MaxSpheres];
    float cosine = clamp(dot(direction, sunDirection), -1.0, 1.0);
    for (int i = 0; i < sphereCount; ++i)
    {
        intervals[i] = intersectSphere(origin, direction, sphereBounds[i], surfaceDistance);
        scatter[i] = clamp(sphereMedia[i].rgb, 0.0, 1.0) * phaseFunction(cosine, sphereAnisotropy[i]);
    }

    float transmittance = 1.0;
    vec3 radiance = vec3(0.0);
    float start = 0.0;
    // Split at sphere boundaries so small or disjoint volumes still receive samples.
    for (int segment = 0; segment < 2 * sphereCount && start < surfaceDistance; ++segment)
    {
        float end = surfaceDistance;
        int activeMask = 0;
        for (int i = 0; i < sphereCount; ++i)
        {
            vec2 interval = intervals[i];
            if (interval.x >= interval.y) continue;
            if (interval.x > start)
                end = min(end, interval.x);
            else if (interval.y > start)
            {
                end = min(end, interval.y);
                activeMask |= 1 << i;
            }
        }

        if (activeMask != 0)
        {
            float stepLength = (end - start) / float(StepsPerInterval);
            for (int step = 0; step < StepsPerInterval; ++step)
            {
                vec3 position = origin + direction * (start + (float(step) + 0.5) * stepLength);
                float extinction = 0.0;
                vec3 scattering = vec3(0.0);
                for (int i = 0; i < sphereCount; ++i)
                {
                    if ((activeMask & (1 << i)) == 0) continue;
                    float radialDistance = length(position - sphereBounds[i].xyz) / sphereBounds[i].w;
                    float density = sphereMedia[i].w * (1.0 - smoothstep(0.0, 1.0, radialDistance));
                    extinction += density;
                    scattering += density * scatter[i];
                }

                if (extinction <= 0.0) continue;
                float segmentTransmittance = exp(-extinction * stepLength);
                if (any(greaterThan(sunRadiance, vec3(0.0))))
                    radiance += transmittance * (1.0 - segmentTransmittance) *
                        (scattering / extinction) * sunRadiance * sunVisibility(position);
                transmittance *= segmentTransmittance;
            }
        }
        start = end;
    }
    return background * transmittance + radiance;
}

vec3 unproject(vec2 ndc, float depth)
{
    vec4 world = inverseViewProjection * vec4(ndc, depth * 2.0 - 1.0, 1.0);
    return world.xyz / world.w;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outColour);
    if (any(greaterThanEqual(pixel, size))) return;

    vec2 ndc = (vec2(pixel) + 0.5) / vec2(size) * 2.0 - 1.0;
    float depth = texelFetch(sceneDepth, pixel, 0).r;
    vec3 origin = unproject(ndc, 0.0);
    vec3 direction = normalize(origin - cameraPosition);
    float surfaceDistance = max(0.0, dot(unproject(ndc, depth) - origin, direction));
    vec3 background = texelFetch(scene, pixel, 0).rgb;
    imageStore(outColour, pixel, vec4(compositeVolumes(background, origin, direction, surfaceDistance), 1.0));
}
