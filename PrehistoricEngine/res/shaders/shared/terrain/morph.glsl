float morphLatitude(vec2 position, vec2 location, vec2 index, float gap)
{
	vec2 frac = position - location;

	if (index == vec2(0, 0))
	{
		float morph = frac.x - frac.y;
		if (morph > 0)
			return morph;
	}
	else if (index == vec2(1, 0))
	{
		float morph = gap - frac.x - frac.y;
		if (morph > 0)
			return morph;
	}
	else if (index == vec2(0, 1))
	{
		float morph = frac.x + frac.y - gap;
		if (morph > 0)
			return -morph;
	}
	else if (index == vec2(1, 1))
	{
		float morph = frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}

	return 0;
}

float morphLongitude(vec2 position, vec2 location, vec2 index, float gap)
{
	vec2 frac = position - location;

	if (index == vec2(0, 0))
	{
		float morph = frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}
	else if (index == vec2(1, 0))
	{
		float morph = frac.y - (gap - frac.x);
		if (morph > 0)
			return morph;
	}
	else if (index == vec2(0, 1))
	{
		float morph = gap - frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}
	else if (index == vec2(1, 1))
	{
		float morph = frac.x - frac.y;
		if (morph > 0)
			return morph;
	}

	return 0;
}

vec2 morph(vec2 localPosition, float height, float morphArea, vec2 location, vec2 index, float gap, mat4 worldMatrix, vec3 cameraPosition)
{
	vec2 morphing = vec2(0);

	vec2 fixPointLatitude = vec2(0);
	vec2 fixPointLongitude = vec2(0);
	float distLatitude;
	float distLongitude;

	if (index == vec2(0, 0))
	{
		fixPointLatitude = location + vec2(gap, 0);
		fixPointLongitude = location + vec2(0, gap);
	}
	else if (index == vec2(1, 0))
	{
		fixPointLatitude = location;
		fixPointLongitude = location + vec2(gap, gap);
	}
	else if (index == vec2(0, 1))
	{
		fixPointLatitude = location + vec2(gap, gap);
		fixPointLongitude = location;
	}
	else if (index == vec2(1, 1))
	{
		fixPointLatitude = location + vec2(0, gap);
		fixPointLongitude = location + vec2(gap, 0);
	}

	distLatitude = length(cameraPosition - (worldMatrix
		* vec4(fixPointLatitude.x, height, fixPointLatitude.y, 1)).xyz);
	distLongitude = length(cameraPosition - (worldMatrix
		* vec4(fixPointLongitude.x, height, fixPointLongitude.y, 1)).xyz);

	if(distLatitude > morphArea)
		morphing.x = morphLatitude(localPosition, location, index, gap);
	if(distLongitude > morphArea)
		morphing.y = morphLongitude(localPosition, location, index, gap);

	return morphing;
}

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
