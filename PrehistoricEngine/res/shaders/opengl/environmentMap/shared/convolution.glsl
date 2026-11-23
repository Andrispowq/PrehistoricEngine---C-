float SourceLOD(float pdf, uint count)
{
    float size = float(textureSize(environmentMap, 0).x);
    float texelArea = 4.0 * PI / (6.0 * size * size);
    return 0.5 * log2(1.0 / (float(count) * pdf * texelArea));
}

vec3 ConvolveEnvironment(vec3 N, float roughness, uint count, bool diffuse)
{
    if (!diffuse && roughness == 0.0) return textureLod(environmentMap, N, 0.0).rgb;
    vec3 colour = vec3(0.0);
    float weight = 0.0;
    for (uint i = 0; i < count; ++i)
    {
        vec3 H = ImportanceSampleGGX(Hammersley(i, count), N, diffuse ? 1.0 : roughness);
        vec3 L = diffuse ? H : normalize(reflect(-N, H));
        float cosine = max(dot(N, L), 0.0);
        if (cosine <= 0.0) continue;
        float lod;
        if (diffuse)
        {
            float pdf = cosine / (4.0 * PI) + 0.0001;
            lod = SourceLOD(pdf + 0.0001 / float(count), count);
        }
        else
        {
            float ndh = max(dot(N, H), 0.0);
            float a2 = pow(roughness, 4.0);
            float denominator = ndh * ndh * (a2 - 1.0) + 1.0;
            float distribution = a2 / (PI * denominator * denominator);
            lod = max(SourceLOD(distribution * 0.25 + 0.0001, count) + 1.0, 0.0);
        }
        colour += textureLod(environmentMap, L, lod).rgb * cosine;
        weight += cosine;
    }
    return colour * (diffuse ? PI : 1.0) / max(weight, 0.00001);
}
