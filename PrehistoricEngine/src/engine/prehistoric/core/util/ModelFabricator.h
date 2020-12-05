#ifndef MODEL_FABRICATOR_H
#define MODEL_FABRICATOR_H

#include "engine/prehistoric/common/buffer/MeshVertexBuffer.h"
#include "engine/prehistoric/common/framework/Window.h"

#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	namespace ModelFabricator
	{
		MeshVertexBuffer* CreateQuad(Window* window);
	};
};

#endif