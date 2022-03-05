#ifndef MODEL_FABRICATOR_H
#define MODEL_FABRICATOR_H

#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	namespace ModelFabricator
	{
		MeshVertexBuffer* CreateQuad(Window* window);
	};
};

#endif