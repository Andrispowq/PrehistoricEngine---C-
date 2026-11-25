#ifndef GL_VOXELISATION_SHADER_H
#define GL_VOXELISATION_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
    class GLVoxelisationShader : public GLShader
    {
    public:
        GLVoxelisationShader();

        void UpdateGlobalUniforms(Renderer* renderer) const override;
        void UpdateMaterialUniforms(Material* material, uint32_t descriptorIndex = 0) const override;
        void UpdateObjectUniforms(GameObject* object, uint32_t instanceIndex = 0) const override;
    };
}

#endif
