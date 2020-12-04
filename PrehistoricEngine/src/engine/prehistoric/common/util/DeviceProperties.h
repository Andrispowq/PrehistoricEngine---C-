#ifndef DEVICE_PROPERTIES
#define DEVICE_PROPERTIES

#include "engine/prehistoric/common/capabilities/Capabilities.h"

class DeviceProperties
{
public:
	void ListProperties(Capabilities& capabilities) const;
};

#endif