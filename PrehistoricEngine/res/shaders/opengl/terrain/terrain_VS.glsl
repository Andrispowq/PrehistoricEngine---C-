#version 430

layout (location = 0) in vec2 position_VS;

out vec2 mapCoord_TC;

uniform sampler2D heightmap;
uniform mat4 worldMatrix;
uniform mat4 localMatrix;
uniform vec3 cameraPosition;
uniform vec2 location;
uniform vec2 index;
uniform float scaleY;
uniform float gap;
uniform int lodMorphArea[8];
uniform int lod;

float morphLatitude(vec2 position)
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

float morphLongitude(vec2 position)
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

vec2 morph(vec2 localPosition, float height, int morphArea)
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

	/*float planarFactor;
	if (cameraPosition.y > abs(scaleY))
		planarFactor = 1;
	else
		planarFactor = cameraPosition.y / abs(scaleY);
		
	distLatitude = length(cameraPosition - (worldMatrix
		* vec4(fixPointLatitude.x, planarFactor, fixPointLatitude.y, 1)).xyz);
	distLongitude = length(cameraPosition - (worldMatrix 
		* vec4(fixPointLongitude.x, planarFactor, fixPointLongitude.y, 1)).xyz);*/

	distLatitude = length(cameraPosition - (worldMatrix
		* vec4(fixPointLatitude.x, height, fixPointLatitude.y, 1)).xyz);
	distLongitude = length(cameraPosition - (worldMatrix
		* vec4(fixPointLongitude.x, height, fixPointLongitude.y, 1)).xyz);
	
	if(distLatitude > morphArea)
		morphing.x = morphLatitude(localPosition);
	if(distLongitude > morphArea)
		morphing.y = morphLongitude(localPosition);
		
	return morphing;
}

void main()
{
	vec2 localPosition = (localMatrix * vec4(position_VS.x, 0, position_VS.y, 1)).xz;

	float height = texture(heightmap, localPosition).r;
	
	if (lod > 0)
		localPosition += morph(localPosition, height, lodMorphArea[lod - 1]);

	mapCoord_TC = localPosition;
	gl_Position = worldMatrix * vec4(localPosition.x, height, localPosition.y, 1);
}