#version 430

layout (location = 0) out vec4 outColour;

layout (location = 0) in vec3 worldPosition;

#define PI 3.1415926535897932384626433832795
#define iSteps 16
#define jSteps 8

#define saturate(a) clamp( a, 0.0, 1.0 )

precision highp float;

uniform mat4 m_view;
uniform mat4 m_projection;

uniform mat4 m_inverseView;
uniform mat4 m_inverseProjection;

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

vec3 _ScreenToWorld(vec3 pos) 
{
    vec4 posP = vec4(pos.xyz * 2.0 - 1.0, 1.0);
    vec4 posVS = m_inverseProjection * posP;
    vec4 posWS = m_inverseView * vec4((posVS.xyz / posVS.w), 1.0);
	
    return posWS.xyz;
}

float _SoftLight(float a, float b) 
{
    return (b < 0.5 ?
        (2.0 * a * b + a * a * (1.0 - 2.0 * b)) :
        (2.0 * a * (1.0 - b) + sqrt(a) * (2.0 * b - 1.0))
    );
}
  
vec3 _SoftLight(vec3 a, vec3 b) 
{
    return vec3(
        _SoftLight(a.x, b.x),
        _SoftLight(a.y, b.y),
        _SoftLight(a.z, b.z)
    );
}

vec2 RaySphereIntersect(vec3 centre, float radius, vec3 ray_start, vec3 ray_direction)
{
    vec3 oc = ray_start - centre;
	
    float a = dot(ray_direction, ray_direction);
    float b = 2.0 * dot(oc, ray_direction);
    float c = dot(oc, oc) - radius * radius;
    float d =  b * b - 4.0 * a * c;
	
    if (d <= 0.0) 
	{
		return vec2(1, -1);
    }
	
    float r0 = (-b - sqrt(d)) / (2.0 * a);
    float r1 = (-b + sqrt(d)) / (2.0 * a);
	
    float t0 = min(r0, r1);
    float t1 = max(r0, r1);
	
    return vec2(t0, t1);
}

vec3 atmosphere(vec3 ray_origin, vec3 ray_direction, vec3 sun_position, float sun_intensitiy, 
			float planet_radius, float atmosphere_radius,
			vec3 reyleigh_coef, float mie_coef, float reyleigh_scale_height, float mie_scale_height, 
			float mie_direction) 
{
    // Normalize the sun and view directions.
    sun_position = normalize(sun_position);
    ray_direction = normalize(ray_direction);

    // Calculate the step size of the primary ray.
    vec2 int_points = RaySphereIntersect(vec3(0), atmosphere_radius, ray_origin, ray_direction);
    if (int_points.x > int_points.y) 
		return vec3(0, 0, 0);
	
    int_points.y = min(int_points.y, RaySphereIntersect(vec3(0), planet_radius, ray_origin, ray_direction).x);
    float iStepSize = (int_points.y - int_points.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0, 0, 0);
    vec3 totalMie = vec3(0, 0, 0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(ray_direction, sun_position);
    float mumu = mu * mu;
    float gg = mie_direction * mie_direction;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * mie_direction, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++)
	{
        // Calculate the primary ray sample position.
        vec3 iPos = ray_origin + ray_direction * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - planet_radius;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / reyleigh_scale_height) * iStepSize;
        float odStepMie = exp(-iHeight / mie_scale_height) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = RaySphereIntersect(vec3(0.0), atmosphere_radius, iPos, sun_position).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) 
		{
            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + sun_position * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - planet_radius;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / reyleigh_scale_height) * jStepSize;
            jOdMie += exp(-jHeight / mie_scale_height) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attenuation = exp(-(mie_coef * (iOdMie + jOdMie) + reyleigh_coef * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attenuation;
        totalMie += odStepMie * attenuation;

        // Increment the primary ray time.
        iTime += iStepSize;
    }

    // Calculate and return the final color.
    return sun_intensitiy * (pRlh * reyleigh_coef * totalRlh + pMie * mie_coef * totalMie);
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
        vec3(0, planetRadius + 1000, 0),        // ray origin, originally: vec3(0, 6372e3, 0)
        normalize(ray_world),        	        // normalized ray direction
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