#include "Includes.hpp"
#include "GLCapabilities.h"

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

namespace Prehistoric
{
	void GLCapabilities::QueryCapabilities(void* physicalDevice)
	{
		std::string name;
		name += reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		name += " - ";
		name += reinterpret_cast<const char*>(glGetString(GL_RENDERER));

		physicalDeviceCaps.name = name;
		physicalDeviceCaps.discrete = true;

		std::string version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		std::vector<std::string> versions = Util::Split(version, '.');
		driverCaps.apiVersionMajor = std::atoi(versions[0].c_str());
		driverCaps.apiVersionMinor = std::atoi(versions[1].c_str());
		driverCaps.apiVersionPatch = std::atoi(versions[2].c_str());
		driverCaps.driverVersionMajor = 0;
		driverCaps.driverVersionMinor = 0;
		driverCaps.driverVersionPatch = 0;

		shaderCaps.geometryShaderSupported = true;
		shaderCaps.tessellationShaderSupported = true;
		shaderCaps.computeShaderSupported = true;
		shaderCaps.shaderVersion = 1;
		shaderCaps.rayTracingSupported = false;
		shaderCaps.meshShadingSupported = false;

		int32_t size;
		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &size);
		memoryCaps.vramSize = (size) * 1024;
		memoryCaps.sharedRamSize = 0;

		int32_t maxTexSlots;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexSlots);
		limits.maxTextureSlots = maxTexSlots;

		int32_t maxTexSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
		limits.maxTextureResolution = maxTexSize;
	}
};