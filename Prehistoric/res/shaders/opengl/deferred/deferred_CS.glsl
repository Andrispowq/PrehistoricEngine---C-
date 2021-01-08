#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

const float PI = 3.141592653589793;
const float emissionFactor = 3;
const int max_lights = 10;

struct Light
{
	vec3 position;
	vec3 colour;
	vec3 intensity;
};

uniform Light lights[max_lights];

uniform sampler2DMS positionMetallic;
uniform sampler2DMS albedoRoughness;
uniform sampler2DMS normalLit;
uniform sampler2DMS emissionExtra;

uniform sampler2D alphaCoverage;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 cameraPosition;

uniform int numberOfLights;
uniform int samples;
uniform float gamma;
uniform float exposure;

uniform vec2 dimension;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / dimension;

    vec2 texelSize = vec2(1.0) / dimension;

    vec4 coverage = texture(alphaCoverage, texCoord);
    vec3 colour = vec3(0.0);

    /*vec4 pM = texelFetch(positionMetallic, x, 0);
    vec4 aR = texelFetch(albedoRoughness, x, 0);
    vec4 nL = texelFetch(normalLit, x, 0);
    vec4 eE = texelFetch(emissionExtra, x, 0);

    vec3 position = pM.rgb;
    vec3 albedo = aR.rgb;
    vec3 normal = nL.rgb;
    vec3 emission = eE.rgb;

    float metallic = pM.a;
    float roughness = aR.a;
    float lit = nL.a;
    float extra = eE.a;

    if (lit == 0.0)
    {
        //TODO: this means that the emission value is a light scattering value instead
        if (extra == 1.0)
        {
            imageStore(outColour, x, vec4(albedo, 1.0));
            return;
        }

        imageStore(outColour, x, vec4(albedo, 1.0));
        return;
    }
    else if (lit == 0.5)
    {
        albedo = 1.0 - exp(-albedo * exposure);
        albedo = pow(albedo, vec3(1.0 / gamma));

        imageStore(outColour, x, vec4(albedo, 1.0));
        return;
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - position);
    vec3 R = normalize(reflect(-V, N));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0);
    for (int i = 0; i < numberOfLights; ++i)
    {
        vec3 L = normalize(lights[i].position - position);
        vec3 H = normalize(V + L);
        float dist = length(lights[i].position - position);
        float attenuation = 1 / pow(dist, 2);
        vec3 radiance = lights[i].colour * lights[i].intensity * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1) - kS;
        kD *= 1 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
        vec3 specular = numerator / max(denominator, 0.001);

        float NdotL = max(dot(N, L), 0);
        if(lit == 0.9)
            Lo += (kD * albedo / PI) * radiance * NdotL;
        else
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

    //The extra in a lit mode represents the occlusion value
    vec3 ambient = (kD * diffuse + specular) * extra;
    vec3 colour_ = ambient + Lo + max(emission * emissionFactor, 0.0);

    colour_ = 1.0 - exp(-colour_ * exposure);
    colour_ = pow(colour_, vec3(1.0 / gamma));

    imageStore(outColour, x, vec4(colour_, 1));
    return;*/

    if (coverage.r == 1.0)
    {
        for (int j = 0; j < samples; j++)
        {
            vec4 pM = texelFetch(positionMetallic, x, j);
            vec4 aR = texelFetch(albedoRoughness, x, j);
            vec4 nL = texelFetch(normalLit, x, j);
            vec4 eE = texelFetch(emissionExtra, x, j);

            vec3 position = pM.rgb;
            vec3 albedo = aR.rgb;
            vec3 normal = nL.rgb;
            vec3 emission = eE.rgb;

            float metallic = pM.a;
            float roughness = aR.a;
            float lit = nL.a;
            float extra = eE.a;

            if (lit == 0.0)
            {
                //TODO: this means that the emission value is a light scattering value instead
                if (extra == 1.0)
                {
                    imageStore(outColour, x, vec4(albedo, 1.0));
                    return;
                }

                imageStore(outColour, x, vec4(albedo, 1.0));
                return;
            }
            else if (lit == 0.5)
            {
                albedo = 1.0 - exp(-albedo * exposure);
                albedo = pow(albedo, vec3(1.0 / gamma));

                imageStore(outColour, x, vec4(albedo, 1.0));
                return;
            }

            vec3 N = normalize(normal);
            vec3 V = normalize(cameraPosition - position);
            vec3 R = normalize(reflect(-V, N));

            vec3 F0 = vec3(0.04);
            F0 = mix(F0, albedo, metallic);

            vec3 Lo = vec3(0);
            for (int i = 0; i < numberOfLights; ++i)
            {
                vec3 L = normalize(lights[i].position - position);
                vec3 H = normalize(V + L);
                float dist = length(lights[i].position - position);
                float attenuation = 1 / pow(dist, 2);
                vec3 radiance = lights[i].colour * lights[i].intensity * attenuation;

                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1) - kS;
                kD *= 1 - metallic;

                vec3 numerator = NDF * G * F;
                float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
                vec3 specular = numerator / max(denominator, 0.001);

                float NdotL = max(dot(N, L), 0);
                //Specular lighting looks weird on terrain while there is no shadow mapping, so we disable it with the magic value of lit == 0.9
                if (lit == 0.9)
                    Lo += (kD * albedo / PI) * radiance * NdotL;
                else
                    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            }

            vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

            vec3 kS = F;
            vec3 kD = 1.0 - kS;
            kD *= 1.0 - metallic;

            vec3 irradiance = texture(irradianceMap, N).rgb;
            vec3 diffuse = irradiance * albedo;

            const float MAX_REFLECTION_LOD = 4.0;
            float lod = roughness * MAX_REFLECTION_LOD;
            vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
            vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
            vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

            //The extra in a lit mode represents the occlusion value
            vec3 ambient = (kD * diffuse + specular) * extra;

            colour += ambient + Lo + max(emission * emissionFactor, 0.0);
        }

        colour /= float(samples);
    }
    else
    {
        vec4 pM = texelFetch(positionMetallic, x, 0);
        vec4 aR = texelFetch(albedoRoughness, x, 0);
        vec4 nL = texelFetch(normalLit, x, 0);
        vec4 eE = texelFetch(emissionExtra, x, 0);

        vec3 position = pM.rgb;
        vec3 albedo = aR.rgb;
        vec3 normal = nL.rgb;
        vec3 emission = eE.rgb;

        float metallic = pM.a;
        float roughness = aR.a;
        float lit = nL.a;
        float extra = eE.a;

        if (lit == 0.0)
        {
            //TODO: this means that the emission value is a light scattering value instead
            if (extra == 1.0)
            {
                imageStore(outColour, x, vec4(albedo, 1.0));
                return;
            }

            imageStore(outColour, x, vec4(albedo, 1.0));
            return;
        }
        else if (lit == 0.5)
        {
            albedo = 1.0 - exp(-albedo * exposure);
            albedo = pow(albedo, vec3(1.0 / gamma));

            imageStore(outColour, x, vec4(albedo, 1.0));
            return;
        }

        vec3 N = normalize(normal);
        vec3 V = normalize(cameraPosition - position);
        vec3 R = normalize(reflect(-V, N));

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        vec3 Lo = vec3(0);
        for (int i = 0; i < numberOfLights; ++i)
        {
            vec3 L = normalize(lights[i].position - position);
            vec3 H = normalize(V + L);
            float dist = length(lights[i].position - position);
            float attenuation = 1 / pow(dist, 2);
            vec3 radiance = lights[i].colour * lights[i].intensity * attenuation;

            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0), F0);

            vec3 kS = F;
            vec3 kD = vec3(1) - kS;
            kD *= 1 - metallic;

            vec3 numerator = NDF * G * F;
            float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
            vec3 specular = numerator / max(denominator, 0.001);

            float NdotL = max(dot(N, L), 0);
            //Specular lighting looks weird on terrain while there is no shadow mapping, so we disable it with the magic value of lit == 0.9
            if (lit == 0.9)
                Lo += (kD * albedo / PI) * radiance * NdotL;
            else
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }

        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);

        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        float lod = roughness * MAX_REFLECTION_LOD;
        vec3 prefilteredColour = textureLod(prefilterMap, R, lod).rgb;
        vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);

        //The extra in a lit mode represents the occlusion value
        vec3 ambient = (kD * diffuse + specular) * extra;

        colour = ambient + Lo + max(emission * emissionFactor, 0.0);
    }

    colour = 1.0 - exp(-colour * exposure);
    colour = pow(colour, vec3(1.0 / gamma));

    imageStore(outColour, x, vec4(colour, 1.0));
}

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
    float k = (r * r) / 2.0;

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