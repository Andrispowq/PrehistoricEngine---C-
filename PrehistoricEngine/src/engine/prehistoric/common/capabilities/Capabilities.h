#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	struct MemoryCapabilities
	{
		uint64_t vramSize = -1; //in kilobytes
		uint64_t sharedRamSize = -1; // in kilobytes
	};

	struct PhysicalDeviceCapabilities
	{
		std::string name = "?";
		bool discrete = true;
	};

	struct DriverCapabilities
	{
		uint32_t driverVersionMajor = -1;
		uint32_t driverVersionMinor = -1;
		uint32_t driverVersionPatch = -1;

		uint32_t apiVersionMajor = -1;
		uint32_t apiVersionMinor = -1;
		uint32_t apiVersionPatch = -1;
	};

	struct ShaderCapabilites
	{
		bool geometryShaderSupported = false;
		bool tessellationShaderSupported = false;
		bool computeShaderSupported = false;
		uint32_t shaderVersion = -1;
		bool rayTracingSupported = false;
		bool meshShadingSupported = false;
	};

	struct CapabilityLimits
	{
		uint32_t maxTextureSlots = -1;
		uint32_t maxTextureResolution = -1;
	};

	class Capabilities
	{
	public:
		static Capabilities* getInstance();

		virtual void QueryCapabilities(void* physicalDevice) = 0;
	public:
		//The information we need to query for
		PhysicalDeviceCapabilities physicalDeviceCaps;
		DriverCapabilities driverCaps;
		MemoryCapabilities memoryCaps;
		ShaderCapabilites shaderCaps;
		CapabilityLimits limits;
	protected:
		static Capabilities* instance;
	};
};

#endif