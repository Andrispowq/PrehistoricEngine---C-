#version 430

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

in vec3 worldPosition;

precision highp float;

uniform mat4 m_projection;
uniform mat4 m_view;

uniform vec3 sunPosition;
uniform float sunRadius;
uniform vec3 sunColour;
uniform float sunIntensity;

uniform float planetRadius;
uniform float atmosphereRadius;

uniform vec3 rayleigh;
uniform float rayleighHeightScale;
uniform float mie;
uniform float mieHeightScale;
uniform float mieDirection;

uniform float horizontalVerticalShift;
uniform int width;
uniform int height;
uniform int isReflection;
uniform int isCubemap;

uniform float exposure;

#include "shared/atmosphere/scattering.glsl"

void main() 
{
	vec2 ndc = vec2(gl_FragCoord.x / width * 2 - 1, gl_FragCoord.y / height * 2 - 1);
	vec4 ray_clip = vec4(ndc, -1.0, 1.0);
	vec4 ray_eye = inverse(m_projection) * ray_clip;
	ray_eye = vec4(ray_eye.xy, -1.0, 0.0);
	vec3 ray_world = (inverse(m_view) * ray_eye).xyz;
	
	if (isReflection == 1)
	{
		ray_world.y *= -1;
	}
	
	ray_world.y += horizontalVerticalShift;
	


    vec3 out_Colour = atmosphere(
        normalize(ray_world),        	        // normalized ray direction
        vec3(0, planetRadius + 1000, 0),        // ray origin, originally: vec3(0, 6372e3, 0)
        sunPosition,                  	        // position of the sun
        sunIntensity,                           // intensity of the sun
        planetRadius,                           // radius of the planet in meters
        atmosphereRadius,                       // radius of the atmosphere in meters
        rayleigh,                               // Rayleigh scattering coefficient
        mie,                                    // Mie scattering coefficient
        rayleighHeightScale,                    // Rayleigh scale height
        mieHeightScale,                         // Mie scale height
        mieDirection                            // Mie preferred direction
    );
	
	float sunradius = length(normalize(ray_world) - normalize(sunPosition));

    out_Colour = 1.0 - exp(-out_Colour * exposure);

	// no sun rendering when scene reflection
	if(sunradius < sunRadius)
	{
		float smoothRadius = SunDiskWeight(sunradius, sunRadius);
        out_Colour = mix(out_Colour, sunColour * 4, smoothRadius);
        if (isCubemap != 0)
        {
            out_Colour += sunColour * SunCaptureIntensity(sunradius, sunRadius);
        }
	}
	
    outColour = vec4(out_Colour, 0.0);
    outPositionMetallic = vec4(ray_world * 10000, 0);
    outAlbedoRoughness = vec4(0.0);
    outNormal = vec4(-normalize(ray_world), 0);
    outBloom = vec4(0.0);
}