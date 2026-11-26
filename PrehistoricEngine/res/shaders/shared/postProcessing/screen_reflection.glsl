// Repeated ray steps often land in the same texel. Cache its view depth,
// retaining every original ray step and every binary refinement.
struct ReflectionSurface
{
    ivec2 pixel;
    vec4 normal;
    vec3 position;
    float depth;
};
void loadReflectionSurface(ivec2 pixel, inout ReflectionSurface surface)
{
    if (all(equal(pixel, surface.pixel))) return;
    surface.pixel = pixel;
    surface.normal = texelFetch(normals, pixel, 0);
    surface.position = vec3(0);
    surface.depth = 0.0;
    if (surface.normal.a >= 0.99)
    {
        surface.position = texelFetch(positionMetallic, pixel, 0).xyz;
        surface.depth = (m_view * vec4(surface.position, 1.0)).z;
    }
}

// Hit colours are linear HDR from this frame, before reflections and post-processing.
vec4 traceScreenRay(vec3 worldPosition, vec3 normal, vec3 reflection, out float hitDistance, out ivec2 hitPixel)
{
    hitDistance = 0.0;
    hitPixel = ivec2(0);
    vec3 start = (m_view * vec4(worldPosition, 1.0)).xyz;
    vec3 ray = mat3(m_view) * reflection;
    ivec2 size = textureSize(normals, 0);
    float footprint = max(0.005, 2.0 * abs(start.z) / (size.y * abs(m_projection[1][1])));
    start += mat3(m_view) * normal * footprint * 1.5;
    float distance = footprint * 2.0;
    float stride = distance;
    float previous = 0.0;
    bool inFront = true;
    #ifdef SSR_DEPTH_BOUNDS
    ivec2 cachedTile = ivec2(-1);
    float nearestDepth = 0.0;
    #endif
    ReflectionSurface cached;
    cached.pixel = ivec2(-1);
    for (int i = 0; i < 56; ++i)
    {
        vec3 point = start + ray * distance;
        vec4 clip = m_projection * vec4(point, 1.0);
        if (clip.w <= 0.0) break;
        vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
        if (any(lessThanEqual(uv, vec2(0))) || any(greaterThanEqual(uv, vec2(1)))) break;
        ivec2 pixel = ivec2(uv * size);
        #ifdef SSR_DEPTH_BOUNDS
        ivec2 tile = pixel / 8;
        if (any(notEqual(tile, cachedTile)))
        {
            cachedTile = tile;
            nearestDepth = texelFetch(reflectionDepth, tile, 0).r;
        }
        if (point.z > nearestDepth)
        {
            inFront = true;
            previous = distance;
            distance += stride;
            stride *= 1.13;
            continue;
        }
        #endif
        loadReflectionSurface(pixel, cached);
        vec4 surface = cached.normal;
        if (surface.a >= 0.99)
        {
            vec3 hit = cached.position;
            float gap = cached.depth - point.z;
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
                    loadReflectionSurface(tap, cached);
                    vec3 p = cached.position;
                    float dz = cached.depth - probe.z;
                    if (cached.normal.a >= 0.99 && dz >= 0.0)
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
                * smoothstep(0.02, 0.25, -dot(texelFetch(normals, pixel, 0).xyz, reflection));
                if (length(hit - worldPosition) > 3.0 * footprint && confidence > 0.0)
                {
                    hitDistance = length(hit - worldPosition);
                    hitPixel = pixel;
                    return vec4(texelFetch(screenRadiance, pixel, 0).rgb, smoothstep(0.0, 0.95, confidence));
                }
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

// Estimate the projected glossy footprint at the first hit. An edge probe can
// locate a thin source that lies inside the cone but outside its center ray.
vec4 traceScreenReflection(vec3 worldPosition, vec3 normal, vec3 reflection, float roughness)
{
    float distance;
    ivec2 hitPixel;
    vec4 hit = traceScreenRay(worldPosition, normal, reflection, distance, hitPixel);
    if (roughness <= 0.08) return hit;
    // Wider lobes are increasingly supplied by the prefiltered voxel/environment path.
    // Bound the screen kernel so a sparse probe cannot produce detached copies of a thin source.
    float alpha = roughness * roughness;
    float spread = 0.045 * alpha / (0.045 + alpha) * smoothstep(0.08, 0.18, roughness);
    if (hit.a == 0.0)
    {
        vec3 tangent = normalize(cross(reflection, abs(reflection.y) < 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0)));
        vec3 bitangent = cross(reflection, tangent);
        for (int i = 0; i < 4 && hit.a == 0.0; ++i)
        {
            vec3 axis = i < 2 ? tangent : bitangent;
            vec3 probe = normalize(reflection + (i % 2 == 0 ? spread : -spread) * axis);
            if (dot(normal, probe) > 0.0)
            hit = traceScreenRay(worldPosition, normal, probe, distance, hitPixel);
        }
        if (hit.a == 0.0) return hit;
    }

    ivec2 size = textureSize(normals, 0);
    vec3 center = worldPosition + reflection * distance;
    vec4 clip = m_projection * m_view * vec4(center, 1);
    if (clip.w <= 0.0) return vec4(0);
    vec2 projected = (clip.xy / clip.w * 0.5 + 0.5) * vec2(size);
    float worldRadius = spread * distance;
    vec2 radius = 0.5 * vec2(size) * abs(vec2(m_projection[0][0], m_projection[1][1])) * worldRadius / clip.w;
    float roughnessWeight = 1.0 - smoothstep(0.32, 0.55, roughness);
    if (max(radius.x, radius.y) < 1.0) return vec4(hit.rgb, hit.a * roughnessWeight);
    vec3 reference = texelFetch(positionMetallic, hitPixel, 0).xyz;
    vec3 targetNormal = texelFetch(normals, hitPixel, 0).xyz;
    vec3 sum = vec3(0);
    float covered = 0.0, total = 0.0;
    for (int y = -2; y <= 2; ++y)
    for (int x = -2; x <= 2; ++x)
    {
        vec2 offset = vec2(x, y) * 0.5;
        float weight = exp(-2.0 * dot(offset, offset));
        total += weight;
        ivec2 tap = ivec2(floor(projected + offset * radius));
        if (any(lessThan(tap, ivec2(0))) || any(greaterThanEqual(tap, size))) continue;
        vec4 n = texelFetch(normals, tap, 0);
        if (n.a < 0.99 || dot(n.xyz, reflection) >= 0.0) continue;
        vec3 delta = texelFetch(positionMetallic, tap, 0).xyz - reference;
        if (abs(dot(delta, targetNormal)) > max(worldRadius, 0.001)) continue;
        covered += weight;
        sum += weight * texelFetch(screenRadiance, tap, 0).rgb;
    }
    if (covered <= 0.0) return vec4(0);
    float confidence = hit.a * covered / total * roughnessWeight;
    return vec4(sum / covered, confidence);
}
