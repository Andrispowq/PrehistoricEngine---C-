#pragma once

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	class Atmosphere;
	class GLDepthPassShader;
	class Camera;

	class GLAtmospherePass
	{
	public:
		explicit GLAtmospherePass(AssembledAssetManager& manager);
		~GLAtmospherePass();

		void DrawScene(const Atmosphere& atmosphere, const Camera& camera);
		void DrawCapture(const Atmosphere& atmosphere, const Matrix4f& view, const Matrix4f& projection);
		void DrawDepth(const Atmosphere& atmosphere, const GLDepthPassShader& shader);

	private:
		void Draw(const Atmosphere& atmosphere, const Matrix4f& view, const Matrix4f& projection, bool capture);
		Pipeline* GetPipeline();

		AssembledAssetManager& manager;
		PipelineHandle simplePipeline;
		PipelineHandle scatteringPipeline;
	};
}
