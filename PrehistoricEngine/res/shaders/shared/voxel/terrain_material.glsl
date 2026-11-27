// Explicit constant sampler indices keep this path within baseline Vulkan features.
float terrainHeight(vec2 uv) { return textureLod(terrainTextures[0], terrainHeightUV(uv), 0).r; }
vec3 terrainNormal(vec2 uv) { return normalize(textureLod(terrainTextures[1], uv, 0).rbg * 2.0 - 1.0); }
vec3 terrainWeights(vec2 uv) { return textureLod(terrainTextures[2], uv, 0).rgb; }
vec3 terrainDetail(vec2 uv)
{
    uv = terrainLayerUV(uv);
    return vec3(textureLod(terrainTextures[4], uv * terrainTiling.x, 0).a,
            textureLod(terrainTextures[7], uv * terrainTiling.y, 0).a,
            textureLod(terrainTextures[10], uv * terrainTiling.z, 0).a);
}
void terrainLayer(float weight, int i, vec2 uv, sampler2D albedo, sampler2D mrot, sampler2D glow,
        inout vec3 base, inout float metal, inout vec3 emission)
{
    uv *= terrainTiling[i];
    base += weight * pow(textureLod(albedo, uv, 0).rgb, vec3(2.2));
    metal += weight * (terrainMrot[i].x < 0 ? textureLod(mrot, uv, 0).r : terrainMrot[i].x);
    emission += weight * terrainEmission[i] * 3.0 * pow(textureLod(glow, uv, 0).rgb, vec3(2.2));
}
void terrainMaterial(vec2 uv, out vec3 base, out float metal, out vec3 emission)
{
    vec3 weights = terrainWeights(uv);
    uv = terrainLayerUV(uv);
    base = emission = vec3(0); metal = 0;
    terrainLayer(weights.x, 0, uv, terrainTextures[3], terrainTextures[4], terrainTextures[5], base, metal, emission);
    terrainLayer(weights.y, 1, uv, terrainTextures[6], terrainTextures[7], terrainTextures[8], base, metal, emission);
    terrainLayer(weights.z, 2, uv, terrainTextures[9], terrainTextures[10], terrainTextures[11], base, metal, emission);
}
