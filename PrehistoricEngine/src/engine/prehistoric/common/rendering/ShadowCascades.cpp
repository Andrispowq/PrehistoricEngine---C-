#include "Includes.hpp"
#include "ShadowCascades.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric {
    ShadowCascades::ShadowCascades() {
        float farPlane = __EngineConfig.rendererFarPlane;
        const float nearPlane = __EngineConfig.rendererNearPlane;
        const float fractions[] = {0.004f, 0.01f, 0.02f, 0.04f, 0.1f, 0.5f};
        for (size_t i = 0; i < splits.size(); ++i)
            splits[i] = nearPlane + (farPlane - nearPlane) * fractions[i];
    }

    const Light *ShadowCascades::FindSun(const Renderer &renderer) {
        for (const auto *light: renderer.getLights()) {
            if (light->castShadows() && light->getParent()->getWorldTransform().getPosition().lengthSquared() > 0.0f)
                return light;
        }
        return nullptr;
    }

    static std::vector<Vector4f> GetFrustumCornersWorldSpace(Matrix4f proj, Matrix4f view) {
        Matrix4f inv = (proj * view).Invert();

        std::vector<Vector4f> frustumCorners;

        for (uint32_t x = 0; x < 2; ++x) {
            for (uint32_t y = 0; y < 2; ++y) {
                for (uint32_t z = 0; z < 2; ++z) {
                    Vector4f pt =
                            inv * Vector4f(
                                2.0f * x - 1.0f,
                                2.0f * y - 1.0f,
                                2.0f * z - 1.0f,
                                1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }

        return frustumCorners;
    }

    static Matrix4f GetViewProjMatrix(const Matrix4f &shadowView, float fov, float aspect, unsigned resolution,
                                      Vector3f lightDir, float nearPlane, float farPlane) {
        Matrix4f cam_proj = Matrix4f::PerspectiveProjection(fov, aspect, nearPlane, farPlane);
        std::vector<Vector4f> corners = GetFrustumCornersWorldSpace(cam_proj, shadowView);

        //view matrix
        Vector3f centre;
        for (const auto &v: corners) {
            centre += v.xyz();
        }
        centre /= float(corners.size());

        Vector3f up = std::abs(lightDir.y) > 0.9999f ? Vector3f(0, 0, 1) : Vector3f(0, 1, 0);
        Vector3f right = lightDir.cross(up).normalise();
        up = right.cross(lightDir).normalise();
        Matrix4f lightView = Matrix4f::View(lightDir, up);

        const Vector4f lightCenter = lightView * Vector4f(centre.x, centre.y, centre.z, 1.0f);
        float radius = 0.0f;
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        for (const auto &corner: corners) {
            radius = std::max(radius, (corner.xyz() - centre).length());
            const float z = (lightView * corner).z;
            minZ = std::min(minZ, z);
            maxZ = std::max(maxZ, z);
        }

        // Stabilize the extent as the camera rotates and snap its center to texels.
        radius = std::ceil(radius * 16.0f) / 16.0f;
        // Reserve room for the soft-shadow kernel, plus the snapped center.
        constexpr float filterPadding = 24.0f;
        radius *= resolution / (resolution - 2.0f * filterPadding);
        const float texel = 2.0f * radius / resolution;
        const float x = std::round(lightCenter.x / texel) * texel;
        const float y = std::round(lightCenter.y / texel) * texel;

        // Include casters on the sunward side of the camera slice.
        const float casterReach = std::max(150.0f, farPlane - nearPlane);
        Matrix4f lightProj = Matrix4f::OrthographicProjection(
            x - radius, x + radius, y - radius, y + radius, -(maxZ + casterReach), -minZ);

        return lightProj * lightView;
    }

    void ShadowCascades::Update(const Matrix4f &view, float fov, float aspect, Vector3f lightDir, unsigned resolution) {
        const float cameraNear = __EngineConfig.rendererNearPlane;
        for (size_t i = 0; i <= splits.size(); ++i) {
            const float sliceNear = i == 0 ? cameraNear : splits[i - 1];
            const float sliceFar = i < splits.size() ? splits[i] : __EngineConfig.rendererFarPlane;
            const float previousNear = i > 1 ? splits[i - 2] : cameraNear;
            const float overlap = (sliceNear - previousNear) * BlendFraction;
            matrices[i] = GetViewProjMatrix(view, fov, aspect, resolution, lightDir, sliceNear - overlap, sliceFar);
        }
    }
}