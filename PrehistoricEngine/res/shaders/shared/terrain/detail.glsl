vec3 terrainTangent(vec3 e1, vec3 e2, vec2 uv1, vec2 uv2)
{
	float determinant = uv1.x * uv2.y - uv1.y * uv2.x;
	vec3 t = (e1 * uv2.y - e2 * uv1.y) * sign(determinant);
	return dot(t, t) > 0.0 ? normalize(t) : vec3(1, 0, 0);
}

float terrainDetailFade(float dist, float detailRange)
{
	return clamp(1.0 - dist / max(detailRange - 50.0, 0.0001), 0.0, 1.0);
}

float terrainDisplacement(vec3 samples, vec3 heightScales, vec3 weights, float dist, float detailRange)
{
	return dot(samples * heightScales, weights) * terrainDetailFade(dist, detailRange);
}

vec3 terrainDetailNormal(vec3 N, vec3 tangent, vec3 bumpNormal, float dist, float detailRange)
{
	vec3 bump = normalize(bumpNormal);
	bump.xz *= terrainDetailFade(dist, detailRange);
	return normalize(mat3(tangent, N, normalize(cross(tangent, N))) * bump);
}
