#include "params.glsl"
#include "../../shared/terrain/detail.glsl"
layout(set=1, binding=3) uniform sampler2D mrot0;
layout(set=1, binding=6) uniform sampler2D mrot1;
layout(set=1, binding=9) uniform sampler2D mrot2;
layout(set=1, binding=11) uniform texture2D terrainSplat;
layout(set=1, binding=15) uniform sampler mapSampler;

float terrainOffset(vec2 uv, float dist)
{
	vec2 tiled = vec2(uv.x, -uv.y);
	vec3 samples = vec3(textureLod(mrot0, tiled * tiling.x, 0).a,
		textureLod(mrot1, tiled * tiling.y, 0).a, textureLod(mrot2, tiled * tiling.z, 0).a);
	return terrainDisplacement(samples, heightScales.xyz, textureLod(sampler2D(terrainSplat, mapSampler), uv, 0).rgb, dist, heightScales.w);
}
