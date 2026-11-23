vec3 terrainLayerWeights(float upwardNormal)
{
    float grass = smoothstep(0.45, 0.55, upwardNormal);
    float rockOrGrass = smoothstep(0.30, 0.40, upwardNormal);
    return vec3(grass, rockOrGrass - grass, 1.0 - rockOrGrass);
}
