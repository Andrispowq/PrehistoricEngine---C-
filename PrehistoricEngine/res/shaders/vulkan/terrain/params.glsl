layout(set=2, binding=2, std140) uniform TerrainParams
{
	vec4 tessellation;
	vec4 tiling;
	vec4 layerMrot[3];
	vec4 heightScales; // w holds the high-detail range
	vec4 emissionEnabled;
};
