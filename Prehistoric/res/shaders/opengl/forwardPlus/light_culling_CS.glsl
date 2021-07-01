#version 430

#define TILE_SIZE 16
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

struct PointLight
{
	vec4 position;
	vec4 colour;
	vec4 intensity_radius;
};

struct VisibleIndex
{
	int index;
};

layout (std430, binding = 0) readonly buffer LightBuffer
{
	PointLight data[];
} lightBuffer;

layout (std430, binding = 1) writeonly buffer VisibleLightIndicesBuffer
{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

uniform sampler2D depthMap;
uniform mat4 m_view;
uniform mat4 m_projection;
uniform vec2 scrnSize;
uniform int lightCount;

shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];

shared int visibleLightIndices[1024];
shared mat4 m_viewProj;

void main()
{
	vec2 screenSize = vec2(scrnSize);

	ivec2 location = ivec2(gl_GlobalInvocationID.xy);
	ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
	ivec2 tileID = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	uint index = tileID.y * tileNumber.x + tileID.x;

	// Initialize shared global values for depth and light count
	if (gl_LocalInvocationIndex == 0) 
	{
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
		m_viewProj = m_projection * m_view;
	}

	barrier();

	// Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
	float maxDepth, minDepth;
	vec2 text = vec2(location) / screenSize;
	float depth = texture(depthMap, text).r;
	// Linearize the depth value from depth buffer (must do this because we created it using projection)
	depth = (0.5 * m_projection[3][2]) / (depth + 0.5 * m_projection[2][2] - 0.5);

	// Convert depth to uint so we can do atomic min and max comparisons between the threads
	uint depthInt = floatBitsToUint(depth);
	atomicMin(minDepthInt, depthInt);
	atomicMax(maxDepthInt, depthInt);

	barrier();

	// Step 2: One thread should calculate the frustum planes to be used for this tile
	if (gl_LocalInvocationIndex == 0) 
	{
		// Convert the min and max across the entire tile back to float
		minDepth = uintBitsToFloat(minDepthInt);
		maxDepth = uintBitsToFloat(maxDepthInt);

		// Steps based on tile sale
		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

		// Transform the first four planes
		for (uint i = 0; i < 4; i++) 
		{
			frustumPlanes[i] *= m_viewProj;
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// Transform the depth planes
		frustumPlanes[4] *= m_view;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] *= m_view;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
	}

	barrier();

	// Step 3: Cull lights.
	// Parallelize the threads against the lights now.
	// Can handle 256 simultaniously. Anymore lights than that and additional passes are performed
	uint threadCount = TILE_SIZE * TILE_SIZE;
	uint passCount = (lightCount + threadCount - 1) / threadCount;
	
	for (uint i = 0; i < passCount; i++)
	{
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
		if (lightIndex >= lightCount) 
		{
			break;
		}

		vec4 position = lightBuffer.data[lightIndex].position;
		float radius = lightBuffer.data[lightIndex].intensity_radius.g;

		// We check if the light exists in our frustum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++) 
		{
			distance = dot(position, frustumPlanes[j]) + radius;

			// If one of the tests fails, then there is no intersection
			if (distance <= 0.0)
			{
				break;
			}
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0) 
		{
			// Add index to the shared array of visible indices
			uint offset = atomicAdd(visibleLightCount, 1);
			visibleLightIndices[offset] = int(lightIndex);
		}
	}

	barrier();

	// One thread should fill the global light buffer
	if (gl_LocalInvocationIndex == 0) 
	{
		uint offset = index * 1024; // Determine bosition in global buffer
		for (uint i = 0; i < visibleLightCount; i++) 
		{
			visibleLightIndicesBuffer.data[offset + i].index = visibleLightIndices[i];
		}

		if (visibleLightCount != 1024) 
		{
			// Unless we have totally filled the entire array, mark it's end with -1
			// Final shader step will use this to determine where to stop (without having to pass the light count)
			visibleLightIndicesBuffer.data[offset + visibleLightCount].index = -1;
		}
	}
}
