float screenSpaceVisibility(vec3 worldPosition, vec3 worldNormal, vec2 settings, ivec2 size)
{
    vec3 center = (m_view * vec4(worldPosition, 1.0)).xyz;
    vec3 normal = normalize(mat3(m_view) * worldNormal);
    float radius = settings.x;
    vec2 pixels = 0.5 * vec2(size) * abs(vec2(m_projection[0][0], m_projection[1][1]))
    * radius / max(-center.z, 0.001);
    if (max(pixels.x, pixels.y) < 1.0) return 1.0;
    vec2 direction = vec2(0.9238795, 0.3826834);
    const mat2 rotate = mat2(0.7071068, 0.7071068, -0.7071068, 0.7071068);
    float blocked = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        float horizon = 0.0;
        float covered = 0.0;
        for (int j = 0; j < 2; ++j)
        {
            vec2 pixel = floor(gl_FragCoord.xy + direction * pixels * (j == 0 ? 0.28 : 0.78));
            if (any(lessThan(pixel, vec2(0))) || any(greaterThanEqual(pixel, vec2(size)))) continue;
            vec3 samplePosition;
            if (!occluderPosition(ivec2(pixel), size, samplePosition)) continue;
            covered += 0.5;
            vec3 delta = samplePosition - center;
            float distanceToSample = length(delta);
            if (distanceToSample < radius * 0.005 || distanceToSample >= radius) continue;
            float elevation = max(dot(normal, delta / distanceToSample) - 0.075, 0.0);
            horizon = max(horizon, elevation * (1.0 - smoothstep(0.0, radius, distanceToSample)));
        }
        if (covered < 1.0 && HAS_AO_VOXELS)
        {
            // Trace the missing part of this azimuth in the receiver's hemisphere.
            vec3 axis = transpose(mat3(m_view)) * vec3(direction * sign(vec2(m_projection[0][0], m_projection[1][1])), 0);
            vec3 tangent = axis - worldNormal * dot(axis, worldNormal);
            if (dot(tangent, tangent) > 0.0001)
            {
                vec3 ray = normalize(normalize(tangent) + worldNormal);
                float visibility = voxelVisibility(worldPosition, worldNormal, ray, radius);
                horizon = max(horizon, (1.0 - covered) * (1.0 - visibility));
            }
        }
        blocked += horizon;
        direction = rotate * direction;
    }
    return clamp(1.0 - settings.y * blocked / 8.0, 0.0, 1.0);
}
