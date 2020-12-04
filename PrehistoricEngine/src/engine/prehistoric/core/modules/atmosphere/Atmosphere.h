#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "engine/prehistoric/core/node/GameObject.h"

#include "engine/prehistoric/core/config/AtmosphereConfig.h"

#include "engine/prehistoric/core/model/obj/OBJLoader.h"

#include "engine/platform/opengl/rendering/shaders/atmosphere/GLAtmosphereScatteringShader.h"
//#include "engine/platform/opengl/rendering/shaders/atmosphere/VKAtmosphereScatteringShader.h"
#include "engine/platform/opengl/rendering/shaders/atmosphere/GLAtmosphereShader.h"
//#include "engine/platform/opengl/rendering/shaders/atmosphere/VKAtmosphereShader.h"
#include "engine/platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "engine/platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"

class AssembledAssetManager;

class Atmosphere : public GameObject
{
public:
	Atmosphere(Window* window, AssembledAssetManager* manager);
	virtual ~Atmosphere() {}

	virtual void PreUpdate(Engine* engine) override;

	inline Vector3f getSunPosition() const { return sunPosition; }

	Atmosphere(const Atmosphere&) = default;

private:
	Window* window;

	Vector3f sunPosition;
};

#endif