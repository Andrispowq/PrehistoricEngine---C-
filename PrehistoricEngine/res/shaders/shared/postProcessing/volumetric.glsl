const int MaxStepsPerInterval = 96;
const float Pi = 3.141592653589793;

vec3 unprojectVolumeRay(vec2 ndc, float clipDepth)
{
    vec4 world = inverseViewProjection * vec4(ndc, clipDepth, 1.0);
    return world.xyz / world.w;
}

// Stagger nearby rays so samples do not form visible planes across shafts.
float volumeSampleOffset(ivec2 pixel)
{
    return fract(52.9829189 * fract(dot(vec2(pixel), vec2(0.06711056, 0.00583715))));
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

vec3 compositeVolumes(vec3 background, vec3 origin, vec3 direction, float surfaceDistance, float sampleOffset)
{
    vec2 intervals[MaxSpheres];
    float start = surfaceDistance;
    for (int i = 0; i < sphereCount; ++i)
    {
        intervals[i] = intersectSphere(origin, direction, sphereBounds[i], surfaceDistance);
        if (intervals[i].x < intervals[i].y) start = min(start, intervals[i].x);
    }
    if (start >= surfaceDistance) return background;

    vec3 scatter[MaxSpheres];
    vec3 albedo[MaxSpheres];
    // Approximate isotropic sky and ground lighting with two hemispheres.
    // Divide the baked irradiance by pi to recover mean radiance.
    vec3 ambient = (textureLod(irradianceMap, vec3(0, 1, 0), 0.0).rgb +
    textureLod(irradianceMap, vec3(0, -1, 0), 0.0).rgb) / (2.0 * Pi);
    float cosine = clamp(dot(direction, sunDirection.xyz), -1.0, 1.0);
    for (int i = 0; i < sphereCount; ++i)
    {
        albedo[i] = clamp(sphereMedia[i].rgb, 0.0, 1.0);
        scatter[i] = albedo[i] * phaseFunction(cosine, volumeAnisotropy(i));
    }

    float transmittance = 1.0;
    vec3 radiance = vec3(0.0);
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
            int steps = int(ceil(clamp((end - start) * 1.5, 12.0, float(MaxStepsPerInterval))));
            float stepLength = (end - start) / float(steps);
            for (int step = 0; step < steps; ++step)
            {
                vec3 position = origin + direction * (start + (float(step) + sampleOffset) * stepLength);
                float extinction = 0.0;
                vec3 scattering = vec3(0.0);
                vec3 ambientScattering = vec3(0.0);
                for (int i = 0; i < sphereCount; ++i)
                {
                    if ((activeMask & (1 << i)) == 0) continue;
                    float radialDistance = length(position - sphereBounds[i].xyz) / sphereBounds[i].w;
                    float density = sphereMedia[i].w * (1.0 - smoothstep(0.0, 1.0, radialDistance));
                    extinction += density;
                    scattering += density * scatter[i];
                    ambientScattering += density * albedo[i];
                }

                if (extinction <= 0.0) continue;
                float segmentTransmittance = exp(-extinction * stepLength);
                vec3 source = ambientScattering * ambient;
                if (shaftStrength > 0.0 && any(greaterThan(sunRadiance.rgb, vec3(0.0))))
                source += scattering * shaftStrength * sunRadiance.rgb * directionalVisibility(position, vec3(0.0), 0.25, true);
                radiance += transmittance * (1.0 - segmentTransmittance) * (source / extinction);
                transmittance *= segmentTransmittance;
            }
        }
        start = end;
    }
    return background * transmittance + radiance;
}
