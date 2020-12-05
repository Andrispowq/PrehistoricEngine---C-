#ifndef DEVICE_PROPERTIES
#define DEVICE_PROPERTIES

#include "prehistoric/common/capabilities/Capabilities.h"

namespace Prehistoric
{
	class DeviceProperties
	{
	public:
		void ListProperties(Capabilities& capabilities) const;
	};
};

#endif