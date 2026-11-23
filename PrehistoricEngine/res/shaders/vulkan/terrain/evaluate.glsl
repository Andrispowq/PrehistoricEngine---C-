#include "patch.glsl"
layout(quads, fractional_odd_spacing, ccw) in;
layout(location=0) in vec2 mapCoord_TE[];
#include "../../shared/terrain/quadEvaluation.glsl"
void evaluateSurface(out vec4 p, out vec2 uv)
{
	evaluateTerrainQuad(p, uv);
	p.y = heightAt(uv) * m_transform[1][1];
}
