#pragma once
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
    struct WaterCapture
    {
        Matrix4f view, projection, clippedProjection;
        Vector3f eye;
        float depthScale;
        bool sky, visible;
    };

    inline WaterCapture MakeWaterCapture(const Matrix4f& view, Matrix4f projection, Vector3f eye,
        float height, bool reflection, bool vulkan)
    {
        Matrix4f mirror = Matrix4f::Identity();
        mirror.m[5] = -1;
        mirror.m[13] = 2 * height;
        const float side = eye.y >= height ? 1.0f : -1.0f;
        const float keep = reflection ? side : -side;
        WaterCapture result{};
        result.view = reflection ? view * mirror : view;
        if (vulkan && reflection) projection.m[0] *= -1; // Preserve winding in the reflected view.
        result.projection = projection;
        result.eye = reflection ? Vector3f(eye.x, 2 * height - eye.y, eye.z) : eye;
        result.sky = keep > 0;
        Matrix4f clipped = projection * result.view;
        const Matrix4f inverse = clipped.Invert();
        float farDistance = 0;
        for (float x : {-1.0f, 1.0f})
        {
            for (float y : {-1.0f, 1.0f})
            {
                const Vector4f corner = inverse * Vector4f(x,y,1,1);
                farDistance = (std::max)(farDistance, keep * (corner.y - height * corner.w));
            }
        }
        result.depthScale = keep * farDistance;
        result.visible = farDistance > 0.00001f;
        if (result.visible)
        {
            for (int column=0; column<4; ++column)
            {
                const float plane = column == 1 ? keep : column == 3 ? -keep * height : 0;
                clipped.m[column*4+2] = vulkan ? plane / farDistance : 2 * plane / farDistance - clipped.m[column*4+3];
            }
            result.clippedProjection = clipped * result.view.Invert();
        }
        return result;
    }
}
