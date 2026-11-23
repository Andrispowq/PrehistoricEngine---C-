#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

struct Material
{
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D mrotMap;
	sampler2D emissionMap;
	
	float heightScale;
	float horizontalScale;
	vec4 mrot;
};

in vec2 mapCoord_GS[];

out vec2 mapCoord_FS;
out vec3 position_FS;
out vec3 tangent_FS;

uniform Material materials[3];
uniform sampler2D splatmap;
uniform sampler2D normalmap;
uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform int highDetailRange;

#include "shared/terrain/detail.glsl"
float terrainOffset(vec2 uv, float dist)
{
	vec3 samples, scales;
	for (int i = 0; i < 3; ++i)
	{
		samples[i] = texture(materials[i].mrotMap, uv * materials[i].horizontalScale).a;
		scales[i] = materials[i].heightScale;
	}
	return terrainDisplacement(samples, scales, texture(splatmap, uv).rgb, dist, highDetailRange);
}

void emitTerrainVertex(int i, float offset, vec3 tangent)
{
	vec4 p = gl_in[i].gl_Position;
	p.y += offset;
	mapCoord_FS = mapCoord_GS[i];
	position_FS = p.xyz;
	tangent_FS = tangent;
	gl_Position = viewProjection * p;
	EmitVertex();
}

#include "shared/terrain/geometry.glsl"
