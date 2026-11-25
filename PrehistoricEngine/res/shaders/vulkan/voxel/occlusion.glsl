// Word offsets into the device-local grid. Each radiance sample stores four half floats.
int volumeCount() { return voxelGridSize() * voxelGridSize() * voxelGridSize(); }
int pyramidCount() { return (8 * volumeCount() - 1) / 7; }
int mipOffset(int level) { return (8 * (volumeCount() - (volumeCount() >> (3 * level)))) / 7; }
int linearCell(ivec3 p, int n) { return p.x + n * (p.y + n * p.z); }
int cellAt(ivec3 p, int level) { return mipOffset(level) + linearCell(p, voxelGridSize() >> level); }
int voxelTopLevel() { return findMSB(voxelGridSize()); }
bool occupiedBlock(ivec3 p, int level) { return cells[cellAt(p, level)] != 0u; }
#include "../../shared/voxel/voxel_visibility.glsl"
