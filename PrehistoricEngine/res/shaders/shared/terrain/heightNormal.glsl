vec3 terrainHeightNormal(sampler2D heightmap, vec2 texCoord, vec2 texelSize, float strength)
{
	float z0 = textureLod(heightmap, texCoord + vec2(-texelSize.x, -texelSize.y), 0.0).r;
	float z1 = textureLod(heightmap, texCoord + vec2(0, -texelSize.y), 0.0).r;
	float z2 = textureLod(heightmap, texCoord + vec2(texelSize.x, -texelSize.y), 0.0).r;
	float z3 = textureLod(heightmap, texCoord + vec2(-texelSize.x, 0), 0.0).r;
	float z4 = textureLod(heightmap, texCoord + vec2(texelSize.x, 0), 0.0).r;
	float z5 = textureLod(heightmap, texCoord + vec2(-texelSize.x, texelSize.y), 0.0).r;
	float z6 = textureLod(heightmap, texCoord + vec2(0, texelSize.y), 0.0).r;
	float z7 = textureLod(heightmap, texCoord + vec2(texelSize.x, texelSize.y), 0.0).r;

	vec3 normal = vec3(0);

	//Sobel filter
	normal.z = 1.0 / strength;
	normal.x = z0 + 2 * z3 + z5 - z2 - 2 * z4 - z7;
	normal.y = z0 + 2 * z1 + z2 - z5 - 2 * z6 - z7;
	return normalize(normal);
}
