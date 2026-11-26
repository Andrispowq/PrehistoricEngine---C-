#include "Includes.hpp"
#include "MeshVertexBuffer.h"

namespace Prehistoric {
    MeshVertexBuffer::MeshVertexBuffer(Window *window, const Model &model)
        : VertexBuffer(window), model(model) {
        type = VertexBufferType::MESH;
        submeshCount = (uint32_t) model.getMeshes().size();
        pickBounds.reserve(submeshCount);
        for (uint32_t i = 0; i < submeshCount; ++i) {
            const auto &points = model.getMesh(i).getVertices();
            const auto initial = points.empty() ? Vector3f(0) : points.front().position;
            Bounds bounds{initial, initial};
            for (const auto &vertex: points) {
                const auto &p = vertex.position;
                bounds.low = {std::min(bounds.low.x, p.x), std::min(bounds.low.y, p.y), std::min(bounds.low.z, p.z)};
                bounds.high = {
                    std::max(bounds.high.x, p.x), std::max(bounds.high.y, p.y), std::max(bounds.high.z, p.z)
                };
            }
            pickBounds.push_back(bounds);
        }
    }

    static bool intersectsBounds(const Vector3f &low, const Vector3f &high, const Vector3f &origin,
                                 const Vector3f &direction, float farDistance) {
        float nearDistance = 0;
        for (int axis = 0; axis < 3; ++axis) {
            if (direction[axis] == 0) {
                if (origin[axis] < low[axis] || origin[axis] > high[axis]) return false;
                continue;
            }
            float a = (low[axis] - origin[axis]) / direction[axis];
            float b = (high[axis] - origin[axis]) / direction[axis];
            if (a > b) std::swap(a, b);
            nearDistance = std::max(nearDistance, a);
            farDistance = std::min(farDistance, b);
            if (farDistance < nearDistance) return false;
        }
        return true;
    }

    std::optional<uint32_t> MeshVertexBuffer::Raycast(const Vector3f &origin, const Vector3f &direction,
                                                      FrontFace face, float &maxDistance) const {
        std::optional<uint32_t> hit;
        for (uint32_t i = 0; i < submeshCount; ++i) {
            const auto &bounds = pickBounds[i];
            if (!intersectsBounds(bounds.low, bounds.high, origin, direction, maxDistance)) continue;
            const auto &mesh = model.getMesh(i);
            const auto &points = mesh.getVertices();
            const auto &indices = mesh.getIndices();
            for (size_t j = 0; j + 2 < indices.size(); j += 3) {
                const auto &a = points[indices[j]].position;
                const auto e1 = points[indices[j + 1]].position - a;
                const auto e2 = points[indices[j + 2]].position - a;
                const auto p = direction.cross(e2);
                const float det = e1.dot(p);
                if (det == 0 || (face == FrontFace::CLOCKWISE && det < 0) ||
                    (face == FrontFace::COUNTER_CLOCKWISE && det > 0))
                    continue;
                const auto fromA = origin - a;
                const float u = fromA.dot(p) / det;
                if (u < 0 || u > 1) continue;
                const auto q = fromA.cross(e1);
                const float v = direction.dot(q) / det;
                if (v < 0 || u + v > 1) continue;
                const float t = e2.dot(q) / det;
                if (t >= 0 && t <= maxDistance) {
                    maxDistance = t;
                    hit = i;
                }
            }
        }
        return hit;
    }

    MeshVertexBuffer::~MeshVertexBuffer() {
    }
};