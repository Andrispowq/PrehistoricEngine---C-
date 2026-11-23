#include "../pbr/camera.glsl"
#include "../pbr/model.glsl"
layout(set=1, binding=0) uniform sampler2D heightMap;

float heightAt(vec2 uv)
{
	return textureLod(heightMap, vec2(uv.x, 1.0 - uv.y), 0).r;
}
