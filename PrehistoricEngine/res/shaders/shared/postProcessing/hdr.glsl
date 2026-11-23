vec3 ToneMapHDR(vec3 radiance, float exposure, float gamma)
{
    vec3 mapped = 1.0 - exp(-max(radiance, vec3(0.0)) * exposure);
    return pow(mapped, vec3(1.0 / gamma));
}
