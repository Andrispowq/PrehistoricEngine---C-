#include "engine/prehistoric/core/util/Includes.hpp"
#include "DeviceProperties.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	void DeviceProperties::ListProperties(Capabilities& capabilities) const
	{
		PR_LOG_MESSAGE("GPU name: %s\n", capabilities.physicalDeviceCaps.name.c_str());
		PR_LOG_MESSAGE("GPU type: %s\n", capabilities.physicalDeviceCaps.discrete ? "Dedicated" : "Integrated");

		PR_LOG_MESSAGE("Driver capabilities:\n");
		PR_LOG_MESSAGE("\tDriver version: %u.%u.%u\n", capabilities.driverCaps.driverVersionMajor, capabilities.driverCaps.driverVersionMinor, capabilities.driverCaps.driverVersionPatch);
		PR_LOG_MESSAGE("\tAPI version: %u.%u.%u\n", capabilities.driverCaps.apiVersionMajor, capabilities.driverCaps.apiVersionMinor, capabilities.driverCaps.apiVersionPatch);

		PR_LOG_MESSAGE("Memory capabilities:\n");
		PR_LOG_MESSAGE("\tAmount of VRAM: %f MB, %f GB\n", capabilities.memoryCaps.vramSize / (1024.0), capabilities.memoryCaps.vramSize / (1024.0 * 1024.0));
		PR_LOG_MESSAGE("\tAmount of shared RAM: %f MB, %f GB\n", capabilities.memoryCaps.sharedRamSize / (1024.0), capabilities.memoryCaps.sharedRamSize / (1024.0 * 1024.0));

		PR_LOG_MESSAGE("Shader capabilities:\n");
		PR_LOG_MESSAGE("\tGeometry shader supported (required): %s\n", capabilities.shaderCaps.geometryShaderSupported ? "true" : "false");
		PR_LOG_MESSAGE("\tTessellation shader supported (required): %s\n", capabilities.shaderCaps.tessellationShaderSupported ? "true" : "false");
		PR_LOG_MESSAGE("\tCompute shader supported (optional): %s\n", capabilities.shaderCaps.computeShaderSupported ? "true" : "false");
		PR_LOG_MESSAGE("\tShader version: %u\n", capabilities.shaderCaps.shaderVersion);
		PR_LOG_MESSAGE("\tRay tracing supported (optional): %s\n", capabilities.shaderCaps.rayTracingSupported ? "true" : "false");
		PR_LOG_MESSAGE("\tMesh shading supported (optional): %s\n", capabilities.shaderCaps.meshShadingSupported ? "true" : "false");

		PR_LOG_MESSAGE("GPU limits:\n");
		PR_LOG_MESSAGE("\tMaximum number of textures bound: %u\n", capabilities.limits.maxTextureSlots);
		PR_LOG_MESSAGE("\tMaximum texture resolution: %u * %u\n", capabilities.limits.maxTextureResolution, capabilities.limits.maxTextureResolution);
	}
};