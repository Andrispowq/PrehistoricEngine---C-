#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 0) uniform sampler3D radianceVolume;
layout(binding = 1) uniform sampler2D positionMetallic;
layout(binding = 2) uniform sampler2D albedoRoughness;
layout(binding = 3) uniform sampler2D normals;

uniform bool debugView;
uniform vec3 volumeMin;
uniform float volumeExtent;
uniform float giStrength;
uniform mat4 inverseViewProjection;

bool inside(vec3 p)
{
    return all(greaterThanEqual(p, vec3(0))) && all(lessThan(p, vec3(1)));
}

vec3 drawVoxels(vec2 ndc)
{
    vec4 nearPoint = inverseViewProjection * vec4(ndc, -1, 1);
    vec4 farPoint = inverseViewProjection * vec4(ndc, 1, 1);
    vec3 origin = (nearPoint.xyz / nearPoint.w - volumeMin) / volumeExtent;
    vec3 direction = normalize(farPoint.xyz / farPoint.w - nearPoint.xyz / nearPoint.w);
    // Keep the slab and grid intersection divisions finite for axis-aligned rays.
    direction = mix(direction, vec3(1e-7), lessThan(abs(direction), vec3(1e-7)));
    vec3 t0 = -origin / direction;
    vec3 t1 = (1.0 - origin) / direction;
    vec3 enter = min(t0, t1);
    vec3 exit = max(t0, t1);
    float start = max(0.0, max(enter.x, max(enter.y, enter.z)));
    float end = min(exit.x, min(exit.y, exit.z));
    vec3 background = vec3(0.025, 0.035, 0.05);
    if (start >= end) return background;

    int size = textureSize(radianceVolume, 0).x;
    vec3 p = (origin + direction * start) * float(size);
    ivec3 cell = clamp(ivec3(floor(p)), ivec3(0), ivec3(size - 1));
    ivec3 stepDirection = ivec3(sign(direction));
    vec3 delta = abs(1.0 / (direction * float(size)));
    vec3 next = (vec3(cell) + max(vec3(stepDirection), vec3(0)) - p)
              / (direction * float(size));
    int axis = enter.x >= max(enter.y, enter.z) ? 0 : (enter.y >= enter.z ? 1 : 2);
    for (int i = 0; i < 3 * size; ++i)
    {
        if (any(lessThan(cell, ivec3(0))) || any(greaterThanEqual(cell, ivec3(size)))) break;
        vec4 voxel = texelFetch(radianceVolume, cell, 0);
        if (voxel.a > 0.0)
        {
            float shade = axis == 0 ? 0.65 : (axis == 1 ? 1.0 : 0.8);
            return shade * pow(vec3(0.12, 0.2, 0.28) + 0.7 * voxel.rgb / (1.0 + voxel.rgb), vec3(1.0 / 2.2));
        }
        axis = next.x <= min(next.y, next.z) ? 0 : (next.y <= next.z ? 1 : 2);
        if (start + next[axis] >= end) break;
        next[axis] += delta[axis];
        cell[axis] += stepDirection[axis];
    }
    return background;
}

vec3 traceCone(vec3 origin, vec3 direction, float cellSize)
{
    vec4 sum = vec4(0);
    float distance = cellSize;
    for (int i = 0; i < 12 && sum.a < 0.95 && distance < 0.4; ++i)
    {
        vec3 p = origin + direction * distance;
        if (!inside(p)) break;
        float diameter = max(cellSize, distance * 0.9);
        vec4 sampleValue = textureLod(radianceVolume, p, log2(diameter / cellSize));
        sum += (1.0 - sum.a) * sampleValue;
        distance += diameter;
    }
    return sum.rgb;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);
    if (any(greaterThanEqual(pixel, size))) return;
    if (debugView)
    {
        vec2 ndc = (vec2(pixel) + 0.5) / vec2(size) * 2.0 - 1.0;
        imageStore(outputImage, pixel, vec4(drawVoxels(ndc), 1.0));
        return;
    }

    vec4 surface = texelFetch(normals, pixel, 0);
    if (surface.a < 0.99 || dot(surface.xyz, surface.xyz) < 0.01) return;
    vec4 position = texelFetch(positionMetallic, pixel, 0);
    vec3 origin = (position.xyz - volumeMin) / volumeExtent;
    if (!inside(origin)) return;

    vec3 normal = normalize(surface.xyz);
    vec3 tangent = normalize(cross(abs(normal.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0), normal));
    vec3 bitangent = cross(normal, tangent);
    float cellSize = 1.0 / float(textureSize(radianceVolume, 0).x);
    origin += normal * cellSize * 1.25;
    vec3 radiance = traceCone(origin, normal, cellSize) * 0.28;
    radiance += traceCone(origin, normalize(normal + tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - tangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal + bitangent), cellSize) * 0.18;
    radiance += traceCone(origin, normalize(normal - bitangent), cellSize) * 0.18;

    vec3 albedo = texelFetch(albedoRoughness, pixel, 0).rgb;
    vec4 scene = imageLoad(outputImage, pixel);
    scene.rgb += radiance * albedo * (1.0 - clamp(position.w, 0.0, 1.0)) * giStrength;
    imageStore(outputImage, pixel, scene);
}
