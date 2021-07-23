#version 430

layout (location = 0) out vec4 outColour;

layout (location = 0) in vec3 position_FS;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(-v.y));
	
    uv *= invAtan;
    uv += 0.5;
	
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(position_FS));
    vec3 colour = texture(equirectangularMap, uv).rgb;

    outColour = vec4(colour, 1.0);
}