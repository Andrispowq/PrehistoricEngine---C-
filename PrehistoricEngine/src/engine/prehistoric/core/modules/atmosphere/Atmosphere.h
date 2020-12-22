#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/config/AtmosphereConfig.h"

#include "prehistoric/core/model/obj/OBJLoader.h"

#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereScatteringShader.h"
//#include "platform/opengl/rendering/shaders/atmosphere/VKAtmosphereScatteringShader.h"
#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereShader.h"
//#include "platform/opengl/rendering/shaders/atmosphere/VKAtmosphereShader.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	class AssembledAssetManager;

	class PR_API Atmosphere : public GameObject
	{
	public:
		Atmosphere(Window* window, AssembledAssetManager* manager);
		virtual ~Atmosphere() {}

		virtual void PreUpdate(CoreEngine* engine) override;

		inline Vector3f getSunPosition() const { return sunPosition; }

		Atmosphere(const Atmosphere&) = default;

	private:
		Window* window;

		Vector3f sunPosition;
	};
};

#endif