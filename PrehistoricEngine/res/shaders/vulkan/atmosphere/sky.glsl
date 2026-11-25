layout(push_constant) uniform Atmosphere
{
    layout(offset = 48) vec4 sunPositionRadius;
    vec4 sunColourIntensity;
    vec4 rayleighMie;
    vec4 radiiAndScaleHeights;
    vec2 anisotropyAndShift;
} sky;

#include "../../shared/atmosphere/scattering.glsl"

vec3 SkyRadiance(vec3 direction, bool capture)
{
    vec3 ray = normalize(normalize(direction) + vec3(0.0, sky.anisotropyAndShift.y, 0.0));
    vec3 sun = normalize(sky.sunPositionRadius.xyz);
    vec4 scales = sky.radiiAndScaleHeights;
    vec3 observer = vec3(0.0, scales.x + 1000.0, 0.0);
    vec3 radiance = atmosphere(ray, observer, sun, sky.sunColourIntensity.w,
        scales.x, scales.y, sky.rayleighMie.rgb, sky.rayleighMie.w, scales.z, scales.w, sky.anisotropyAndShift.x);
    if (HitsPlanet(observer, ray, scales.x)) return radiance;
    float distanceToSun = length(ray - sun);
    float disk = SunDiskWeight(distanceToSun, sky.sunPositionRadius.w);
    vec3 colour = mix(radiance, sky.sunColourIntensity.rgb * 4.0, disk);
    if (capture) colour += sky.sunColourIntensity.rgb * SunCaptureIntensity(distanceToSun, sky.sunPositionRadius.w);
    return colour;
}
