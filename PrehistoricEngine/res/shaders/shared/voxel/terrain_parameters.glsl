// Backend supplies terrain texture sampling and storeTerrainVoxel.
layout(std430, binding = 1) readonly buffer TerrainVoxelParameters
{
    mat4 terrainModel;
    vec4 terrainGrid, terrainEye, terrainTiling, terrainHeights;
    vec4 terrainMrot[3];
    vec4 terrainEmission, terrainRegion, terrainOptions;
};

