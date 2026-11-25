#include "Includes.hpp"
#include "GLVoxelisationShader.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric
{
    GLVoxelisationShader::GLVoxelisationShader()
    {
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_VS.glsl"), VERTEX_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_GS.glsl"), GEOMETRY_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_FS.glsl"), FRAGMENT_SHADER);
        CompileShader();

        for (const char* uniform : {"m_transform", "volumeMin", "volumeExtent", "albedo", "hasAlbedoMap", "emission", "injectRadiance"})
            AddUniform(uniform);
        AddShadowUniforms();
    }

    void GLVoxelisationShader::UpdateGlobalUniforms(Renderer* renderer) const
    {
        BindShadowUniforms(renderer, 1);
    }

    void GLVoxelisationShader::UpdateMaterialUniforms(Material* material, uint32_t descriptorIndex) const
    {
        const Vector3f colour = material ? material->getVector3f(COLOUR) : Vector3f(-1);
        Texture* albedoMap = material && material->exists(ALBEDO_MAP) ? material->getTexture(ALBEDO_MAP) : nullptr;
        const bool useTexture = colour.x < 0.0f && albedoMap;
        if (useTexture) albedoMap->Bind(0);
        SetUniformi("hasAlbedoMap", useTexture);
        SetUniform("albedo", colour.x < 0.0f ? Vector3f(1) : colour);
        SetUniformf("emission", material ? std::max(0.0f, material->getFloat(EMISSION)) : 0.0f);
    }

    void GLVoxelisationShader::UpdateObjectUniforms(GameObject* object, uint32_t instanceIndex) const
    {
        SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
    }
}
