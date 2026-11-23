#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rg32f) uniform writeonly image2D brdfLUT;

#include "opengl/environmentMap/shared/ggx.glsl"
#include "opengl/environmentMap/shared/brdf.glsl"

void main()
{
    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(brdfLUT);
    vec2 texCoord = vec2(x) / vec2(size);
	
    vec2 integratedBRDF = IntegrateBRDF(texCoord.x, texCoord.y, 1024u);
    imageStore(brdfLUT, x, vec4(integratedBRDF, 0.0, 1.0));
}