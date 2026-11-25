#define SHADOW_SET 0
#include "../shadow/cascades.glsl"

void shadeSurface(vec3 albedoColour, float metallic, float roughness, float occlusion, vec3 N, float emission)
{
	vec3 V = normalize((waterView.eyeAndCapture.w != 0.0 ? waterView.eyeAndCapture.xyz : cameraPosition) - position_FS);
	vec3 R = reflect(-V, N);

	vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColour, metallic);

    vec3 Lo = vec3(0);

    if (any(greaterThan(sunRadiance.rgb, vec3(0))))
    {
        float visibility = cascadeSunVisibility(position_FS, normalize(normal_FS), 0.65, false);
        Lo = visibility * directPbrLight(N, V, sunDirection.xyz, F0, albedoColour, metallic, roughness, sunRadiance.rgb);
    }

    // reflectance equation
	for(int i = 0; i < numberOfLights; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights.position[i].xyz - position_FS);

        float dist = length(lights.position[i].xyz - position_FS);
        if (dist > lights.intensity[i].g) continue;
        float attenuation = 1 / pow(dist, 2);
        vec3 radiance = lights.colour[i].rgb * lights.intensity[i].r * attenuation;

        Lo += directPbrLight(N, V, L, F0, albedoColour, metallic, roughness, radiance);
    }

	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedoColour * occlusion;

	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * maxReflectionLOD).rgb;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.01), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = kD * diffuse + specular;

	// Match OpenGL: scalar emission strength tinted by the material albedo.
	vec3 emissive = clamp(albedoColour, vec3(0.05), vec3(1.0)) * emission * emissionFactor;

	outColour = vec4(ambient + Lo + emissive, 1);
	outBloom = vec4(emissive, 0);
}
