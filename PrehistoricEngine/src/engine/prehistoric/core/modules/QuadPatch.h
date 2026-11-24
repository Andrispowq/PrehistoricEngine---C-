#pragma once
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	struct QuadPatch
	{
		Vector4f locationGapMorph;
		Vector4f quadrant;
	};

    inline std::vector<Vector2f> QuadPatchVertices()
    {
        std::vector<Vector2f> vertices;
        for (float z : {0.0f, 0.333f, 0.666f, 1.0f})
            for (float x : {0.0f, 0.333f, 0.666f, 1.0f})
                vertices.emplace_back(x,z);
        return vertices;
    }
}
