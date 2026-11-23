float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 directPbrLight(vec3 N, vec3 V, vec3 L, vec3 F0, vec3 albedo, float metallic, float roughness, vec3 radiance)
{
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 diffuse = (1.0 - F) * (1.0 - metallic) * albedo / PI;
    float denominator = max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.001);
    vec3 specular = DistributionGGX(N, H, roughness) * GeometrySmith(N, V, L, roughness) * F / denominator;
    return (diffuse + specular) * radiance * NdotL;
}
