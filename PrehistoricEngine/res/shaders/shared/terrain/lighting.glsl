vec3 terrainLight(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness,
	vec3 toLight, vec3 lightColour, vec2 intensityRadius)
{
	float range = clamp(length(toLight) / intensityRadius.y, 0.0, 1.0);
	vec3 radiance = lightColour * intensityRadius.x * (1.0 - range * range);
	return directPbrLight(N, V, normalize(toLight), F0, albedo, metallic, roughness, radiance);
}

vec3 terrainAmbient(vec3 N, vec3 V, vec3 F0, vec3 albedoColour, float metallic, float roughness, float occlusion,
	samplerCube irradianceMap, samplerCube prefilterMap, sampler2D brdfLUT, float maxReflectionLod)
{
    vec3 R = normalize(reflect(-V, N));
    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedoColour;

    float lod = roughness * maxReflectionLod;
    vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.01), roughness)).rg;
    vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

    return (kD * diffuse + specular) * occlusion;
}
