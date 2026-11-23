float terrainLodFactor(float distanceToCamera, float factor, float slope, float shift)
{
	return clamp(factor / pow(max(distanceToCamera, 0.001), slope) + shift, 0.0, 1.0);
}
