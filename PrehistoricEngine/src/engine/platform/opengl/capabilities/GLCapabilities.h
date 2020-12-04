#ifndef GL_CAPABILITIES_H
#define GL_CAPABILITIES_H

#include <glew.h>

#include "engine/prehistoric/common/capabilities/Capabilities.h"

class GLCapabilities : public Capabilities
{
public:
	virtual void QueryCapabilities(void* physicalDevice) override;
};

#endif