#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly imageCube environmentMap;
layout(binding = 1, rgba32f) uniform readonly image2D equirectangularMap;

uniform vec2 equirectangularMapResolution;
uniform float resolution;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	
    uv *= invAtan;
    uv += 0.5;
	
    return uv;
}

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = vec2(x) / resolution;
	texCoord.y = 1.0 - texCoord.y;
	texCoord *= 2.0;
	texCoord -= 1.0;
	
	uint face = gl_GlobalInvocationID.z;
	
	vec3 sampleDir = vec3(0.0);
	switch (face)
	{
	case 0: //pos x
		sampleDir = vec3(1.0, texCoord.y, texCoord.x);
		break;
	case 1: //neg x
		sampleDir = vec3(-1.0, texCoord.y, -texCoord.x);
		break;
	case 2: //pos y
		sampleDir = vec3(texCoord.x, 1.0, texCoord.y);
		break;
	case 3: //neg y
		sampleDir = vec3(texCoord.x, -1.0, -texCoord.y);
		break;
	case 4: //pos z
		sampleDir = vec3(texCoord.x, texCoord.y, -1.0);
		break;
	case 5: //neg z
		sampleDir = vec3(-texCoord.x, texCoord.y, 1.0);
		break;
	}
	
    vec2 uv = SampleSphericalMap(normalize(sampleDir));
	uv *= equirectangularMapResolution;

    vec3 colour = imageLoad(equirectangularMap, ivec2(uv)).rgb;
	
    imageStore(environmentMap, ivec3(gl_GlobalInvocationID), vec4(colour, 1.0));
}