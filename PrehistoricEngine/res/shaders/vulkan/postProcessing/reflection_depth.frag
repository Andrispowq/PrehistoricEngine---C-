#version 450
layout(location = 0) out float nearestDepth;
layout(binding = 0) uniform sampler2D positions;
layout(binding = 1) uniform sampler2D normals;
layout(push_constant) uniform Camera { mat4 view; } camera;

void main()
{
    ivec2 first = ivec2(gl_FragCoord.xy) * 8;
    ivec2 size = textureSize(normals, 0);
    nearestDepth = -3.402823e38;
    vec4 row = vec4(camera.view[0][2], camera.view[1][2], camera.view[2][2], camera.view[3][2]);
    for (int y = 0; y < 8; ++y)
    for (int x = 0; x < 8; ++x)
    {
        ivec2 p = first + ivec2(x, y);
        if (any(greaterThanEqual(p, size)) || texelFetch(normals, p, 0).a < 0.99) continue;
        vec3 world = texelFetch(positions, p, 0).xyz;
        float depth = (camera.view * vec4(world, 1.0)).z;
        // Round conservatively toward the camera, including cancellation when
        // world coordinates are large. A loose bound only does extra tracing.
        float error = 4e-6 * (dot(abs(row.xyz), abs(world)) + abs(row.w) + 1.0);
        nearestDepth = max(nearestDepth, depth + error);
    }
}
