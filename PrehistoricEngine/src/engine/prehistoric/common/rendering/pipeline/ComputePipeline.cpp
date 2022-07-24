#include "Includes.hpp"

#include "ComputePipeline.h"
#include "RayTracingPipeline.h"

namespace Prehistoric
{
	void ComputePipeline::ResetBindings()
	{
		textureBindingTable.clear();
		ssboBindingTable.clear();
	}
}