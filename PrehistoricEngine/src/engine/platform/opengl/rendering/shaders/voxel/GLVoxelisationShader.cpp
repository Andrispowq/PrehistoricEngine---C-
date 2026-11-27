#include "Includes.hpp"
#include "GLVoxelisationShader.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric {
    GLVoxelisationShader::GLVoxelisationShader() {
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_VS.glsl"), VERTEX_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_GS.glsl"), GEOMETRY_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxelisation_FS.glsl"), FRAGMENT_SHADER);
        CompileShader();

        for (const char *uniform: {
                 "m_transform", "volumeMin", "volumeExtent", "albedo", "hasAlbedoMap", "emission", "injectRadiance",
                 "sunDirection", "sunRadiance", "lightViewProjection", "shadowBias",
                 "captureBounceSurfaces", "metallic", "useVoxelSunShadows", "useTerrainOcclusion",
                 "voxelOccluders", "voxelOcclusionMin", "voxelOcclusionExtent"
             })
            AddUniform(uniform);
    }

    GLVoxelisationShader::Surface GLVoxelisationShader::GetSurface(Material *material, bool captureBounceSurfaces) {
        const Vector3f colour = material ? material->getVector3f(COLOUR) : Vector3f(-1);
        Texture *albedoMap = colour.x < 0.0f && material && material->exists(ALBEDO_MAP)
                                 ? material->getTexture(ALBEDO_MAP)
                                 : nullptr;
        const float metallic = captureBounceSurfaces && material ? material->getVector4f(MROT).r : 0.0f;
        Texture *metallicMap = metallic < 0.0f ? material->getTexture(MROT_MAP) : nullptr;
        return {
            colour.x < 0.0f ? Vector3f(1) : colour, albedoMap,
            material ? std::max(0.0f, material->getFloat(EMISSION)) : 0.0f, metallic, metallicMap
        };
    }

    void GLVoxelisationShader::SetSurface(const Surface &surface) const {
        if (surface.metallicMap) surface.metallicMap->Bind(2);
        SetUniformf("metallic", surface.metallicMap ? surface.metallic : std::max(0.0f, surface.metallic));
        if (surface.albedoMap) surface.albedoMap->Bind(0);
        SetUniformi("hasAlbedoMap", surface.albedoMap != nullptr);
        SetUniform("albedo", surface.albedo);
        SetUniformf("emission", surface.emission);
    }

    void GLVoxelisationShader::UpdateObjectUniforms(GameObject *object, uint32_t instanceIndex) const {
        SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
    }
}