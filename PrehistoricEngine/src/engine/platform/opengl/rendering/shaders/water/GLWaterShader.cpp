#include "Includes.hpp"
#include "GLWaterShader.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    GLWaterShader::GLWaterShader() {
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_VS.glsl"), VERTEX_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TC.glsl"), TESSELLATION_CONTROL_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TE.glsl"), TESSELLATION_EVALUATION_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_GS.glsl"), GEOMETRY_SHADER);
        AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_FS.glsl"), FRAGMENT_SHADER);
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

        AddUniform("waterReflection");
        AddUniform("waterRefraction");
        AddUniform("refractionDepth");
        AddUniform("refractionDepthScale");
        AddUniform("normalMap");
        AddUniform("dudvMap");

        AddUniform("worldMatrix");

        AddUniform("waterColour");
        AddUniform("sunDirection");
        AddUniform("sunRadiance");
        AddUniform("specularFactor");
        AddUniform("wind");

        AddUniform("kReflection");
        AddUniform("kRefraction");
        AddUniform("reflectionBlendFactor");
        AddUniform("capillarStrength");
        AddUniform("capillarDownsampling");
        AddUniform("dudvDownsampling");


        AddUniform("isCameraUnderWater");
        AddUniform("waterVoxelBounds");
        AddUniform("waterVoxelOptions");
    }

    void GLWaterShader::UpdateGlobalUniforms(Renderer *renderer) const {
        sceneRenderer = renderer;
        static_cast<GLRenderer *>(renderer)->getVoxelPass()->BindWaterLighting(*this);
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

        SetUniform("waterColour", config->baseColour);
        Vector3f sunDirection(0, 1, 0), sunRadiance(0);
        for (Light *light: renderer->getLights()) {
            if (light->castShadows()) {
                sunDirection = light->getParent()->getWorldTransform().getPosition().normalise();
                sunRadiance = light->getColour() * config->specularAmplifier;
                break;
            }
        }
        SetUniform("sunDirection", sunDirection);
        SetUniform("sunRadiance", sunRadiance);
        SetUniformf("specularFactor", config->specularFactor);

        SetUniformf("kReflection", config->kReflection);
        SetUniformf("kRefraction", config->kRefraction);
        SetUniformf("reflectionBlendFactor", config->reflectionBlendfactor);
        SetUniformf("capillarStrength", config->capillarStrength);
        SetUniformf("capillarDownsampling", config->capillarDownsampling);
        SetUniformf("dudvDownsampling", config->dudvDownsampling);
    }

    void GLWaterShader::UpdateMaterialUniforms(Material *material, uint32_t descriptor_index) const {
        material->getTexture("Dy")->Bind(3);
        material->getTexture("Dx")->Bind(4);
        material->getTexture("Dz")->Bind(5);

        SetUniformi("Dy", 3);
        SetUniformi("Dx", 4);
        SetUniformi("Dz", 5);

        material->getTexture("waterReflection")->Bind(6);
        material->getTexture("waterRefraction")->Bind(7);
        material->getTexture("normalMap")->Bind(8);
        material->getTexture("dudvMap")->Bind(9);

        SetUniformi("waterReflection", 6);
        SetUniformi("waterRefraction", 7);
        material->getTexture("refractionDepth")->Bind(10);
        SetUniformi("refractionDepth", 10);
        SetUniformf("refractionDepthScale", material->getFloat("refractionDepthScale"));
        SetUniformi("normalMap", 8);
        SetUniformi("dudvMap", 9);

        SetUniformf("waterHeight", material->getFloat("waterHeight"));

        SetUniformf("motion", material->getFloat("motion"));
        SetUniformi("isCameraUnderWater", material->getInt("underwater"));
    }

    void GLWaterShader::UpdateObjectUniforms(GameObject *object, uint32_t instance_index) const {
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