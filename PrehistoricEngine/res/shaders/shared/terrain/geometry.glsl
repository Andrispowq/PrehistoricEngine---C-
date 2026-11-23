void main()
{
	float distances[3];
	for (int i = 0; i < 3; ++i)
		distances[i] = distance(gl_in[i].gl_Position.xyz, cameraPosition);

	bool detailed = (distances[0] + distances[1] + distances[2]) / 3.0 < highDetailRange;
	vec3 tangent = vec3(0);
	if (detailed)
		tangent = terrainTangent(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz,
			gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz,
			mapCoord_GS[1] - mapCoord_GS[0], mapCoord_GS[2] - mapCoord_GS[0]);

	for (int i = 0; i < 3; ++i)
	{
		float offset = detailed ? terrainOffset(mapCoord_GS[i], distances[i]) : 0.0;
		emitTerrainVertex(i, offset, tangent);
	}
	EndPrimitive();
}
