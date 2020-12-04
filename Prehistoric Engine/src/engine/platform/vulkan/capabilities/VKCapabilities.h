#ifndef VK_CAPABILITIES_H
#define VK_CAPABILITIES_H

#include <vulkan/vulkan.h>

#include "engine/prehistoric/common/capabilities/Capabilities.h"

class VKCapabilities : public Capabilities
{
public:
	virtual void QueryCapabilities(void* physicalDevice) override;
};

#endif