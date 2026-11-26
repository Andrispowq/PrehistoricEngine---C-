#include "Includes.hpp"
#include "GLEnvironmentMapShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric {
    GLEnvironmentMapShader::GLEnvironmentMapShader() {
        AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environmentMap_CS.glsl"), COMPUTE_SHADER);

        CompileShader();

        AddUniform("equirectangularMapResolution");
        AddUniform("resolution");
        AddUniform("worldToEnvironment");
    }

    void GLEnvironmentMapShader::UpdateUniforms(Vector2f equirectMapRes, const Matrix4f &rotation) const {
        SetUniform("equirectangularMapResolution", equirectMapRes);
        SetUniform("worldToEnvironment", rotation);
        SetUniformf("resolution", (float) __EnvironmentMapConfig.environmentMapResolution);
    }
};