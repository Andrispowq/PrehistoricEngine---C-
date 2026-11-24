#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D tilde_h0k;
layout (binding = 1, rgba32f) writeonly uniform image2D tilde_h0minusk;

layout (binding = 2) uniform sampler2D noise_r0;
layout (binding = 3) uniform sampler2D noise_i0;
layout (binding = 4) uniform sampler2D noise_r1;
layout (binding = 5) uniform sampler2D noise_i1;

uniform	int N;
uniform int L;
uniform float amplitude;
uniform float intensity;
uniform vec2 direction;
uniform float l;
uniform float alignment;

const float g = 9.81;

// Box-Muller-Method

vec4 gaussRND()
{	
	vec2 texCoord = vec2(gl_GlobalInvocationID.xy)/float(N);
	
	float noise00 = clamp(texture(noise_r0, texCoord).r, 0.001, 1.0);
	float noise01 = clamp(texture(noise_i0, texCoord).r, 0.001, 1.0);
	float noise02 = clamp(texture(noise_r1, texCoord).r, 0.001, 1.0);
	float noise03 = clamp(texture(noise_i1, texCoord).r, 0.001, 1.0);
	
	float u0 = 2.0 * M_PI * noise00;
	float v0 = sqrt(-2.0 * log(noise01));
	float u1 = 2.0 * M_PI * noise02;
	float v1 = sqrt(-2.0 * log(noise03));
	
	vec4 rnd = vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));
	
	return rnd;
}


void main(void)
{
	vec2 x = vec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	
	vec2 k = vec2(2.0 * M_PI * x.x / L, 2.0 * M_PI * x.y / L);

	float L_ = (intensity * intensity) / g;
    float magSq = dot(k, k);
    float windSq = dot(direction, direction);
    float spectrum = 0.0;
    if (magSq > 0.0 && windSq > 0.0 && L_ > 0.0)
    {
        float alongWind = dot(k, direction) / sqrt(magSq * windSq);
        float power = amplitude * alongWind * alongWind * exp(-1.0 / (magSq * L_ * L_) - magSq * l * l);
        spectrum = min(sqrt(max(power, 0.0) * 0.5) / magSq, 4000.0);
    }

	vec4 gauss_random = gaussRND();
	
	imageStore(tilde_h0k, ivec2(gl_GlobalInvocationID.xy), vec4(gauss_random.xy * spectrum, 0, 1));
	imageStore(tilde_h0minusk, ivec2(gl_GlobalInvocationID.xy), vec4(gauss_random.zw * spectrum, 0, 1));
}