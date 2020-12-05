#ifndef GL_CAPABILITIES_H
#define GL_CAPABILITIES_H

#include "prehistoric/common/capabilities/Capabilities.h"

#include <glew.h>

namespace Prehistoric
{
	class GLCapabilities : public Capabilities
	{
	public:
		virtual void QueryCapabilities(void* physicalDevice) override;
	};
};

#endif