#ifndef MODEL_FABRICATOR_H
#define MODEL_FABRICATOR_H

#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/common/framework/Window.h"

#include "prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	namespace ModelFabricator
	{
		PR_API MeshVertexBuffer* CreateQuad(Window* window);
	};
};

#endif