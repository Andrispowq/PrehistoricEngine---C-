#include "Includes.hpp"
#include "GLWaterWireframeShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    GLWaterWireframeShader::GLWaterWireframeShader() {
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_VS.glsl"), VERTEX_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TC.glsl"), TESSELLATION_CONTROL_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TE.glsl"), TESSELLATION_EVALUATION_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_wGS.glsl"), GEOMETRY_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_wFS.glsl"), FRAGMENT_SHADER);
        CompileShader();

        location_localMatrix = glGetUniformLocation(program, "localMatrix");
        location_worldMatrix = glGetUniformLocation(program, "worldMatrix");

        location_location = glGetUniformLocation(program, "location");
        location_index = glGetUniformLocation(program, "index");
        location_gap = glGetUniformLocation(program, "gap");
        location_lod = glGetUniformLocation(program, "lod");

        AddUniform("scaleY");
        AddUniform("waterHeight");

        for (int i = 0; i < 8; i++) {
            std::string uName = "lodMorphArea[" + std::to_string(i) + "]";
            AddUniform(uName);
        }

        AddUniform("tessellationFactor");
        AddUniform("tessellationSlope");
        AddUniform("tessellationShift");

        AddUniform("tiling");

        AddUniform("Dy");
        AddUniform("Dx");
        AddUniform("Dz");

        AddUniform("viewProjection");

        AddUniform("cameraPosition");
        AddUniform("wind");

        AddUniform("displacementScale");
        AddUniform("choppiness");
        AddUniform("motion");

        AddUniform("displacementRange");
    }

    void GLWaterWireframeShader::UpdateGlobalUniforms(Renderer *renderer) const {
        sceneRenderer = renderer;
        Camera *camera = renderer->getCamera();

        SetUniform("viewProjection", camera->getViewProjectionMatrix());
        SetUniform("cameraPosition", camera->getPosition());

        SetUniformi("tessellationFactor", __WaterConfig.tessellationFactor);
        SetUniformf("tessellationSlope", __WaterConfig.tessellationSlope);
        SetUniformf("tessellationShift", __WaterConfig.tessellationShift);

        for (unsigned int i = 0; i < 8; i++) {
            std::string uName = "lodMorphArea[" + std::to_string(i) + "]";

            SetUniformi(uName, i < __WaterConfig.lodMorphingAreas.size() ? __WaterConfig.lodMorphingAreas[i] : 0);
        }

        WaterConfig *config = &__WaterConfig;

        SetUniformf("scaleY", config->scaleY);

        SetUniformi("tiling", config->tiling);

        SetUniform("wind", config->windDirection);

        SetUniformf("displacementScale", config->displacementScale);
        SetUniformf("choppiness", config->choppy ? config->choppiness : 0.0f);

        SetUniformi("displacementRange", config->displacementRange);
    }

    void GLWaterWireframeShader::UpdateMaterialUniforms(Material *material, uint32_t descriptor_index) const {
        material->getTexture("Dy")->Bind(3);
        material->getTexture("Dx")->Bind(4);
        material->getTexture("Dz")->Bind(5);

        SetUniformi("Dy", 3);
        SetUniformi("Dx", 4);
        SetUniformi("Dz", 5);

        SetUniformf("waterHeight", material->getFloat("waterHeight"));
        SetUniformf("motion", material->getFloat("motion"));
    }

    void GLWaterWireframeShader::UpdateObjectUniforms(GameObject *object, uint32_t instance_index) const {
        ProceduralConfigScope scope(object);
        if (sceneRenderer) UpdateGlobalUniforms(sceneRenderer);
        WaterNode *node = (WaterNode *) object;

        SetUniform(location_localMatrix, node->getLocalTransform().getTransformationMatrix());
        SetUniform(location_worldMatrix, node->getWorldTransform().getTransformationMatrix());

        SetUniform(location_location, node->getLocation());
        SetUniform(location_index, node->getIndex());
        SetUniformf(location_gap, node->getGap());
        SetUniformi(location_lod, node->getLod());
    }
}