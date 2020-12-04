#version 430

#define PI 3.1415926535897932384626433832795
#define iSteps 16
#define jSteps 8

precision highp float;

layout (location = 0) out vec4 outColour;

in vec3 worldPosition;

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

uniform float gamma;
uniform float exposure;

vec2 rsi(vec3 r0, vec3 rd, float sr)
{
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b * b) - 4.0 * a * c;
    if (d < 0.0) return vec2(1e5, -1e5);
    return vec2(
        (-b - sqrt(d)) / (2.0 * a),
        (-b + sqrt(d)) / (2.0 * a)
    );
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos,
				vec3 kRlh, float kMie, float shRlh, float shMie, float g) 
{
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) 
		return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0, 0, 0);
    vec3 totalMie = vec3(0, 0, 0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++)
	{
        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) 
		{
            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;
    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}  

void main() 
{
	vec2 ndc = vec2(gl_FragCoord.x / width * 2 - 1, gl_FragCoord.y / height * 2 - 1);
	vec4 ray_clip = vec4(ndc, -1.0, 1.0);
	vec4 ray_eye = inverse(m_projection) * ray_clip;
	ray_eye = vec4(ray_eye.xy, 1.0, 0.0);
	vec3 ray_world = (inverse(m_view) * ray_eye).xyz;
	ray_world.y += horizontalVerticalShift;
	
	if (isReflection == 1)
	{
		ray_world.y *= -1;
	}
	
	vec3 out_LightScattering = vec3(0);

    vec3 out_Color = atmosphere(
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

    // Apply exposure.
    out_Color = 1.0 - exp(-out_Color * exposure);
    //out_Color = pow(out_Color, vec3(gamma));
    //out_Color = pow(out_Color, vec3(1 / gamma));

	// no sun rendering when scene reflection
	if(sunradius < sunRadius && isReflection == 0)
	{
		sunradius /= sunRadius;
		float smoothRadius = smoothstep(0, 1, 0.1f / sunradius - 0.1f);
		out_Color = mix(out_Color, sunColour * 4, smoothRadius);
		
		smoothRadius = smoothstep(0, 1, 0.18f / sunradius - 0.2f);
		out_LightScattering = mix(vec3(0), sunColour, smoothRadius);
	}
	
	outColour = vec4(out_Color, 1);
}