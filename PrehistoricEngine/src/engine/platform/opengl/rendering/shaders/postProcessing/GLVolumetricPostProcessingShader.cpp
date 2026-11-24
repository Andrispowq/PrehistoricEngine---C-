#include "Includes.hpp"
#include "GLVolumetricPostProcessingShader.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	namespace
	{
		constexpr size_t MaxSpheres = 8;
	}

	GLVolumetricPostProcessingShader::GLVolumetricPostProcessingShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/volumetric_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("sceneDepth");
		AddUniform("cameraPosition");
		AddUniform("inverseViewProjection");
		AddUniform("m_view");
		AddUniform("sunDirection");
		AddUniform("sunRadiance");
		AddUniform("sphereCount");
		for (size_t i = 0; i < MaxSpheres; ++i)
		{
			const std::string index = "[" + std::to_string(i) + "]";
			AddUniform("sphereBounds" + index);
			AddUniform("sphereMedia" + index);
			AddUniform("sphereAnisotropy" + index);
		}
		AddShadowUniforms();
	}

	void GLVolumetricPostProcessingShader::UpdateUniforms(Texture* scene, Texture* depth, Renderer* renderer) const
	{
		scene->Bind(0);
		SetUniformi("scene", 0);
		depth->Bind(1);
		SetUniformi("sceneDepth", 1);
		BindShadowUniforms(renderer, 2);

		const Camera* camera = renderer->getCamera();
		SetUniform("cameraPosition", camera->getPosition());
		SetUniform("inverseViewProjection", camera->getViewProjectionMatrix().Invert());
		SetUniform("m_view", camera->getViewMatrix());

		Vector3f sunDirection(0, 1, 0);
		Vector3f sunRadiance(0);
		for (const auto* light : renderer->getLights())
		{
			if (!light->castShadows()) continue;

			const Vector3f position = light->getParent()->getWorldTransform().getPosition();
			if (position.dot(position) > 0.0f)
			{
				sunDirection = position.normalise();
				sunRadiance = light->getColour() * light->getIntensity();
			}
			break; // Match the directional light chosen by GLShadowDepthPass.
		}
		SetUniform("sunDirection", sunDirection);
		SetUniform("sunRadiance", sunRadiance);

		const auto& spheres = renderer->getVolumetricSpheres();
		size_t count = 0;
		for (const auto& sphere : spheres)
		{
			if (sphere.radius <= 0.0f || sphere.density <= 0.0f) continue;

			const std::string index = "[" + std::to_string(count) + "]";
			SetUniform("sphereBounds" + index, Vector4f(sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius));
			SetUniform("sphereMedia" + index, Vector4f(sphere.scatteringTint.x, sphere.scatteringTint.y,
				sphere.scatteringTint.z, sphere.density));
			SetUniformf("sphereAnisotropy" + index, sphere.anisotropy);
			if (++count == MaxSpheres) break;
		}
		SetUniformi("sphereCount", static_cast<int>(count));
	}
}
