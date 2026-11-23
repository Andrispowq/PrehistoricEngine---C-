void evaluateTerrainQuad(out vec4 position, out vec2 mapCoord)
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	position =
	((1 - u) * (1 - v) * gl_in[12].gl_Position +
	u * (1 - v) * gl_in[0].gl_Position +
	u * v * gl_in[3].gl_Position +
	(1 - u) * v * gl_in[15].gl_Position);

	mapCoord =
	((1 - u) * (1 - v) * mapCoord_TE[12] +
	u * (1 - v) * mapCoord_TE[0] +
	u * v * mapCoord_TE[3] +
	(1 - u) * v * mapCoord_TE[15]);
}
