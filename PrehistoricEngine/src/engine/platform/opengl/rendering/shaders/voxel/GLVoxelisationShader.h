#ifndef GL_VOXELISATION_SHADER_H
#define GL_VOXELISATION_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
    class GLVoxelisationShader : public GLShader
    {
    public:
        struct Surface
        {
            Vector3f albedo;
            Texture* albedoMap;
            float emission;

            bool operator==(const Surface& other) const
            {
                return albedo == other.albedo && albedoMap == other.albedoMap && emission == other.emission;
            }
        };

        GLVoxelisationShader();

        static Surface GetSurface(Material* material);

        void UpdateGlobalUniforms(Renderer* renderer) const override;
        void UpdateMaterialUniforms(Material* material, uint32_t descriptorIndex = 0) const override;
        void UpdateObjectUniforms(GameObject* object, uint32_t instanceIndex = 0) const override;
    };
}

#endif
