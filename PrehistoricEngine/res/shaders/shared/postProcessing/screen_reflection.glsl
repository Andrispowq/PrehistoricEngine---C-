// Hit colours are linear HDR from this frame, before reflections and post-processing.
vec4 traceScreenReflection(vec3 worldPosition, vec3 normal, vec3 reflection, float roughness)
{
    vec3 start = (m_view * vec4(worldPosition, 1.0)).xyz;
    vec3 ray = mat3(m_view) * reflection;
    ivec2 size = textureSize(normals, 0);
    float footprint = max(0.005, 2.0 * abs(start.z) / (size.y * abs(m_projection[1][1])));
    start += mat3(m_view) * normal * footprint * 1.5;
    float distance = footprint * 2.0;
    float stride = distance;
    float previous = 0.0;
    bool inFront = true;
    for (int i = 0; i < 56; ++i)
    {
        vec3 point = start + ray * distance;
        vec4 clip = m_projection * vec4(point, 1.0);
        if (clip.w <= 0.0) break;
        vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
        if (any(lessThanEqual(uv, vec2(0))) || any(greaterThanEqual(uv, vec2(1)))) break;
        ivec2 pixel = ivec2(uv * size);
        vec4 surface = texelFetch(normals, pixel, 0);
        if (surface.a >= 0.99)
        {
            vec3 hit = texelFetch(positionMetallic, pixel, 0).xyz;
            float gap = (m_view * vec4(hit, 1.0)).z - point.z;
            if (inFront && gap >= 0.0)
            {
                float lo = previous, hi = distance;
                for (int j = 0; j < 5; ++j)
                {
                    float mid = 0.5 * (lo + hi);
                    vec3 probe = start + mid * ray;
                    vec4 projected = m_projection * vec4(probe, 1.0);
                    vec2 tapUV = projected.xy / projected.w * 0.5 + 0.5;
                    ivec2 tap = clamp(ivec2(tapUV * size), ivec2(0), size - 1);
                    vec3 p = texelFetch(positionMetallic, tap, 0).xyz;
                    float dz = (m_view * vec4(p, 1.0)).z - probe.z;
                    if (texelFetch(normals, tap, 0).a >= 0.99 && dz >= 0.0)
                    {
                        hi = mid;
                        pixel = tap;
                        uv = tapUV;
                        hit = p;
                        gap = dz;
                    }
                    else lo = mid;
                }
                float thickness = max(3.0 * footprint, abs(point.z) * 0.008);
                vec2 edge = min(uv, 1.0 - uv);
                float confidence = smoothstep(0.0, 0.06, min(edge.x, edge.y))
                * (1.0 - smoothstep(0.0, thickness, gap))
                * smoothstep(0.02, 0.25, -dot(texelFetch(normals, pixel, 0).xyz, reflection))
                * (1.0 - smoothstep(0.18, 0.55, roughness));
                if (length(hit - worldPosition) > 3.0 * footprint && confidence > 0.0)
                return vec4(texelFetch(screenRadiance, pixel, 0).rgb, smoothstep(0.0, 0.95, confidence));
                // A foreground depth discontinuity hides what lies behind it.
                if (hi > 4.0 * footprint) break;
            }
            inFront = gap < 0.0;
        }
        else inFront = true;
        previous = distance;
        distance += stride;
        stride *= 1.13;
    }
    return vec4(0);
}
