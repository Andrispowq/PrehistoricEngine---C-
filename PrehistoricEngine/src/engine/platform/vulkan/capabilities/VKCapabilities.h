#ifndef VK_CAPABILITIES_H
#define VK_CAPABILITIES_H

#include "engine/prehistoric/common/capabilities/Capabilities.h"

#include <vulkan/vulkan.h>

namespace Prehistoric
{
	class VKCapabilities : public Capabilities
	{
	public:
		virtual void QueryCapabilities(void* physicalDevice) override;
	};
};

#endif