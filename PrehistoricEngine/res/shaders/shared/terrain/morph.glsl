void morphTerrainEndpoint(inout vec2 uv, inout float height, float morphArea, vec2 location, vec2 index,
	float gap, mat4 worldMatrix, vec3 cameraPosition)
{
	vec2 shift = morph(uv, height, morphArea, location, index, gap, worldMatrix, cameraPosition);
	if (any(notEqual(shift, vec2(0))))
	{
		uv += shift;
		// Neighboring patches need the same endpoint height to select their edge tessellation.
		height = heightAt(uv);
	}
}
