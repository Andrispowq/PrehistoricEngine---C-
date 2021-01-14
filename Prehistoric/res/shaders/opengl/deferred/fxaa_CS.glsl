#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D sceneSampler;
uniform vec2 dimension;

const float EDGE_THRESHOLD_MIN = 0.0312;
const float EDGE_THRESHOLD_MAX = 0.125;
const int ITERATIONS = 7;
const float QUALITY[7] = {1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f};
const float SUBPIXEL_QUALITY = 0.75;

float rgb2luma(vec3 rgb)
{
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

void main()
{
	ivec2 computeCoord = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = computeCoord / dimension;

	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MUL = 1.0 / 8.0;
	float FXAA_REDUCE_MIN = 1.0 / 128.0;

	vec3 rgbNW = texture2D(sceneSampler, uv + (vec2(-1.0, -1.0) / dimension)).xyz;
	vec3 rgbNE = texture2D(sceneSampler, uv + (vec2(1.0, -1.0) / dimension)).xyz;
	vec3 rgbSW = texture2D(sceneSampler, uv + (vec2(-1.0, 1.0) / dimension)).xyz;
	vec3 rgbSE = texture2D(sceneSampler, uv + (vec2(1.0, 1.0) / dimension)).xyz;
	vec3 rgbM = texture2D(sceneSampler, uv).xyz;

	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM = dot(rgbM, luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
		max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
			dir * rcpDirMin)) / dimension;

	vec3 rgbA = (1.0 / 2.0) * (
		texture2D(sceneSampler, uv + dir * (1.0 / 3.0 - 0.5)).xyz +
		texture2D(sceneSampler, uv + dir * (2.0 / 3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) * (
		texture2D(sceneSampler, uv + dir * (0.0 / 3.0 - 0.5)).xyz +
		texture2D(sceneSampler, uv + dir * (3.0 / 3.0 - 0.5)).xyz);
	float lumaB = dot(rgbB, luma);

	if ((lumaB < lumaMin) || (lumaB > lumaMax))
	{
		imageStore(outColour, computeCoord, vec4(rgbA, 1.0));
	}
	else 
	{
		imageStore(outColour, computeCoord, vec4(rgbB, 1.0));
	}

	/*vec3 rgb = texture(sceneSampler, uv).rgb;

	float lumaCenter = rgb2luma(rgb);
	float lumaDown = rgb2luma(texture(sceneSampler, uv + vec2(0, -1.0 / dimension.y)).rgb);
	float lumaUp = rgb2luma(texture(sceneSampler, uv + vec2(0, 1.0 / dimension.y)).rgb);
	float lumaLeft = rgb2luma(texture(sceneSampler, uv + vec2(-1.0 / dimension.x, 0)).rgb);
	float lumaRight = rgb2luma(texture(sceneSampler, uv + vec2(1.0 / dimension.x, 0)).rgb);

	float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	float lumaRange = lumaMax - lumaMin;

	// If the luma variation is lower than a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
	{
		imageStore(outColour, computeCoord, vec4(rgb, 1.0));
		return;
	}

	float lumaDownLeft = rgb2luma(texture(sceneSampler, uv + vec2(-1.0 / dimension.x, -1.0 / dimension.y)).rgb);
	float lumaUpRight = rgb2luma(texture(sceneSampler, uv + vec2(1.0 / dimension.x, 1.0 / dimension.y)).rgb);
	float lumaUpLeft = rgb2luma(texture(sceneSampler, uv + vec2(-1.0 / dimension.x, 1.0 / dimension.y)).rgb);
	float lumaDownRight = rgb2luma(texture(sceneSampler, uv + vec2(1.0 / dimension.x, -1.0 / dimension.y)).rgb);

	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;

	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;

	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0
		+ abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0
		+ abs(-2.0 * lumaDown + lumaDownCorners);

	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;

	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	bool is1Steepest = abs(gradient1) >= abs(gradient2);
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	float lumaLocalAverage = 0.0;
	if (is1Steepest)
	{
		stepLength = -stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else
	{
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}

	vec2 currentUv = uv;
	if (isHorizontal)
	{
		currentUv.y += stepLength * 0.5;
	}
	else
	{
		currentUv.x += stepLength * 0.5;
	}

	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);
	vec2 uv1 = currentUv - offset;
	vec2 uv2 = currentUv + offset;
	float lumaEnd1 = rgb2luma(texture(sceneSampler, uv1).rgb);
	float lumaEnd2 = rgb2luma(texture(sceneSampler, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	if (!reached1)
	{
		uv1 -= offset;
	}
	if (!reached2)
	{
		uv2 += offset;
	}

	if (!reachedBoth)
	{
		for (int i = 0; i < ITERATIONS; i++)
		{
			if (!reached1)
			{
				lumaEnd1 = rgb2luma(texture(sceneSampler, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			if (!reached2)
			{
				lumaEnd2 = rgb2luma(texture(sceneSampler, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}

			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			if (!reached1)
			{
				uv1 -= offset * QUALITY[i];
			}

			if (!reached2)
			{
				uv2 += offset * QUALITY[i];
			}

			if (reachedBoth)
			{
				break;
			}
		}
	}

	float distance1 = isHorizontal ? (computeCoord.x - uv1.x) : (computeCoord.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - computeCoord.x) : (uv2.y - computeCoord.y);

	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);

	float edgeThickness = (distance1 + distance2);
	float pixelOffset = -distanceFinal / edgeThickness + 0.5;

	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

	float finalOffset = correctVariation ? pixelOffset : 0.0;
	float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	finalOffset = max(finalOffset, subPixelOffsetFinal);

	vec2 finalUV = uv;
	if (isHorizontal)
	{
		finalUV.y += int(finalOffset * stepLength);
	}
	else
	{
		finalUV.x += int(finalOffset * stepLength);
	}

	// Read the color at the new UV coordinates, and use it.
	vec3 finalColour = texture(sceneSampler, finalUV).rgb;

	imageStore(outColour, computeCoord, vec4(finalColour, 1.0));*/
}
