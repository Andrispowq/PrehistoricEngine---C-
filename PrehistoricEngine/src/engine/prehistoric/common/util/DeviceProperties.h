#ifndef DEVICE_PROPERTIES
#define DEVICE_PROPERTIES

#include "engine/prehistoric/common/capabilities/Capabilities.h"

namespace Prehistoric
{
	class DeviceProperties
	{
	public:
		void ListProperties(Capabilities& capabilities) const;
	};
};

#endif