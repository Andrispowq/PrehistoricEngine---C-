#version 430
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outPositionMetallic;
layout(location = 2) out vec4 outAlbedoRoughness;
layout(location = 3) out vec4 outNormal;
layout(location = 4) out vec4 outBloom;

in vec4 position_FS;
in vec2 mapCoord_FS;
in vec3 normal_FS;

uniform sampler2D waterReflection;
uniform sampler2D waterRefraction;
uniform sampler2D refractionDepth;
uniform float refractionDepthScale;
uniform float waterHeight;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform vec3 waterColour;
uniform vec3 sunDirection;
uniform vec3 sunRadiance;
uniform float specularFactor;
uniform float kReflection;
uniform float kRefraction;
uniform float reflectionBlendFactor;
uniform float capillarStrength;
uniform float capillarDownsampling;
uniform float dudvDownsampling;
uniform int isCameraUnderWater;

void main()
{
    vec3 eye = normalize(cameraPosition - position_FS.xyz);
    vec3 detail = texture(normalMap, mapCoord_FS * capillarDownsampling).xyz * 2.0 - 1.0;
    vec3 n = normalize(normalize(normal_FS) + vec3(detail.x, 0, detail.y) * capillarStrength);
    if (isCameraUnderWater != 0) n = -n;
    float cosine = clamp(dot(n, eye), 0.0, 1.0);
    float fresnel = 0.02 + 0.98 * pow(1.0 - cosine, 5.0);
    if (isCameraUnderWater != 0 && cosine < 0.66) fresnel = 1.0;
    vec4 clip = viewProjection * position_FS;
    vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
    vec2 slope = n.xz + (texture(dudvMap, mapCoord_FS * dudvDownsampling).rg * 2.0 - 1.0) * 0.25;
    vec3 reflected = texture(waterReflection, clamp(uv + slope * kReflection, 0.001, 0.999)).rgb;
    // The oblique near plane encodes signed distance to the water plane divided by clip W.
    float depth = texture(refractionDepth, uv).r;
    float rayY = position_FS.y - cameraPosition.y;
    float denominator = rayY - depth * refractionDepthScale * clip.w;
    float floorFraction = abs(denominator) > 1e-6 ? (waterHeight - cameraPosition.y) / denominator : 1.0;
    float thickness = depth >= 0.99999 ? 100.0 : max(0.0, floorFraction - 1.0) * distance(position_FS.xyz, cameraPosition);
    vec2 refractedUV = clamp(uv - slope * kRefraction * smoothstep(0.0, 2.0, thickness), 0.001, 0.999);
    vec3 transmitted = texture(waterRefraction, refractedUV).rgb;
    float pathLength = isCameraUnderWater != 0 ? 0.0 : thickness;
    vec3 transmission = exp(-vec3(0.09, 0.035, 0.02) * pathLength * max(reflectionBlendFactor, 0.0));
    vec3 colour = mix(waterColour + (transmitted - waterColour) * transmission, reflected, fresnel);
    float glint = pow(max(dot(reflect(-sunDirection, n), eye), 0.0), max(specularFactor, 1.0));
    if (isCameraUnderWater == 0) colour += sunRadiance * glint * max(dot(n, sunDirection), 0.0) * fresnel;
    outColour = vec4(colour, 1);
    outPositionMetallic = vec4(position_FS.xyz, 0);
    outAlbedoRoughness = vec4(colour, 0.08);
    outNormal = vec4(n, 1);
    outBloom = vec4(0);
}
