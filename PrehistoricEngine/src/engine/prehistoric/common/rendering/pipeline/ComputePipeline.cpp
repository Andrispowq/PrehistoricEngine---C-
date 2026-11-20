#include "Includes.hpp"
#include "ComputePipeline.h"

namespace Prehistoric
{
	void ComputePipeline::ResetBindings()
	{
		textureBindingTable.clear();
		ssboBindingTable.clear();
	}
};
