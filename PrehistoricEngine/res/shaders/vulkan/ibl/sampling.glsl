const float PI = 3.141592653589793;

vec2 Hammersley(uint index, uint count)
{
    return vec2(float(index) / float(count), float(bitfieldReverse(index)) * 2.3283064365386963e-10);
}

vec3 SampleEnvironment(vec3 direction, float lod)
{
    vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * PI) + 0.5,
            0.5 - asin(clamp(direction.y, -1.0, 1.0)) / PI);
    return max(textureLod(environmentMap, uv, lod).rgb, vec3(0.0));
}

// Choose source LOD from the solid angle covered by one integration sample.
// Equirectangular texels shrink towards the poles; cube-map texel area is not
// applicable to this source projection.
float SourceLOD(vec3 direction, float pdf, uint count)
{
    vec2 size = vec2(textureSize(environmentMap, 0));
    float latitudeScale = max(sqrt(max(1.0 - direction.y * direction.y, 0.0)), sin(0.5 * PI / size.y));
    float sampleArea = 1.0 / max(float(count) * pdf, 0.000001);
    float angularWidth = sqrt(sampleArea);
    vec2 footprint = angularWidth * vec2(size.x / (2.0 * PI * latitudeScale), size.y / PI);
    // Use the footprint's major axis. An area-only LOD still leaves distinct
    // longitudes at a pole, where atan is undefined, and creates a new seam.
    return clamp(log2(max(footprint.x, footprint.y)), 0.0, float(textureQueryLevels(environmentMap) - 1));
}

// Two analytic orthonormal frames, each used away from its singular pole.
// Blend convolution estimates in the overlap, not the frame vectors: blending
// vectors can collapse a basis, and a hard frame switch moves the sampling seam.
mat3 TangentBasis(vec3 normal, bool north)
{
    vec3 n = north ? normal : -normal;
    float a = 1.0 / (1.0 + n.z);
    float b = -n.x * n.y * a;
    vec3 tangent = vec3(1.0 - n.x * n.x * a, b, -n.x);
    vec3 bitangent = vec3(b, 1.0 - n.y * n.y * a, -n.y);
    return mat3(tangent, north ? bitangent : -bitangent, normal);
}

vec3 ImportanceSampleGGX(vec2 xi, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * xi.x;
    float cosine = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sine = sqrt(max(1.0 - cosine * cosine, 0.0));
    return vec3(cos(phi) * sine, sin(phi) * sine, cosine);
}

vec3 ConvolveFrame(vec3 N, mat3 basis, float roughness, uint count, bool diffuse)
{
    vec3 colour = vec3(0.0);
    float weight = 0.0;
    for (uint i = 0; i < count; ++i)
    {
        vec2 xi = Hammersley(i, count);
        if (diffuse)
        {
            float phi = 2.0 * PI * xi.x;
            vec3 L = basis * vec3(sqrt(xi.y) * cos(phi), sqrt(xi.y) * sin(phi), sqrt(1.0 - xi.y));
            float pdf = max(dot(N, L), 0.0) / PI;
            colour += SampleEnvironment(L, SourceLOD(L, pdf, count));
            weight += 1.0;
        }
        else
        {
            vec3 H = basis * ImportanceSampleGGX(xi, roughness);
            vec3 L = normalize(2.0 * dot(N, H) * H - N);
            float NdotL = max(dot(N, L), 0.0);
            if (NdotL > 0.0)
            {
                float NdotH = max(dot(N, H), 0.0);
                float a2 = pow(roughness, 4.0);
                float denominator = NdotH * NdotH * (a2 - 1.0) + 1.0;
                float distribution = a2 / max(PI * denominator * denominator, 0.0000001);
                // V = N during prefiltering, so D * NdotH / (4 * VdotH) = D / 4.
                float pdf = distribution * 0.25;
                colour += SampleEnvironment(L, SourceLOD(L, pdf, count)) * NdotL;
                weight += NdotL;
            }
        }
    }
    // Cosine-weighted diffuse stores irradiance / PI.
    return colour / max(weight, 0.00001);
}

vec3 ConvolveEnvironment(vec3 N, float roughness, uint count, bool diffuse)
{
    if (!diffuse && roughness == 0.0) return SampleEnvironment(N, 0.0);

    float blend = smoothstep(-0.25, 0.25, N.z);
    if (blend == 0.0) return ConvolveFrame(N, TangentBasis(N, false), roughness, count, diffuse);
    if (blend == 1.0) return ConvolveFrame(N, TangentBasis(N, true), roughness, count, diffuse);
    vec3 south = ConvolveFrame(N, TangentBasis(N, false), roughness, count, diffuse);
    vec3 north = ConvolveFrame(N, TangentBasis(N, true), roughness, count, diffuse);
    return mix(south, north, blend);
}
