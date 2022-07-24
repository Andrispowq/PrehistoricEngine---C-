#include "Includes.hpp"
#include "Water.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric
{
	Water::Water(Window* window, Camera* camera, AssembledAssetManager* manager, Vector3f position)
		: window(window), camera(camera), manager(manager), fft{ nullptr }, reflectionFbo{ nullptr }, refractionFbo{ nullptr }
	{
		fft = std::make_unique<FFT>(manager, window, &__WaterConfig);		
		normalMapRenderer = std::make_unique<NormalMapRenderer>(window, manager, __WaterConfig.normalStrength, __WaterConfig.fftResolution);

		AssetManager* man = manager->getAssetManager();
		dudv = man->loadTexture("res/textures/water/dudv1.jpg").value();
		man->addReference<Texture>(dudv.handle);

		//Create framebuffers
		reflectionTexture = man->storeTexture(GLTexture::Storage2D(refl_size, refl_size, 1, R8G8B8A8_LINEAR, Bilinear));
		refractionTexture = man->storeTexture(GLTexture::Storage2D(refr_size, refr_size, 1, R8G8B8A8_LINEAR, Bilinear));
		man->addReference<Texture>(reflectionTexture.handle);
		man->addReference<Texture>(refractionTexture.handle);

		reflectionFbo = std::make_unique<GLFramebuffer>(window);
		reflectionFbo->Bind();
		reflectionFbo->addColourAttachment2D(reflectionTexture.pointer);
		reflectionFbo->addDepthAttachment(refl_size, refl_size);
		reflectionFbo->Unbind();

		refractionFbo = std::make_unique<GLFramebuffer>(window);
		refractionFbo->Bind();
		refractionFbo->addColourAttachment2D(refractionTexture.pointer);
		refractionFbo->addDepthAttachment(refr_size, refr_size);
		refractionFbo->Unbind();

		//data
		texData = new TexturesData();
		texData->Dy = fft->getDyTexture();
		texData->Dx = fft->getDxTexture();
		texData->Dz = fft->getDzTexture();

		texData->waterReflection = reflectionTexture.pointer;
		texData->waterRefraction = refractionTexture.pointer;
		texData->dudvMap = dudv.pointer;

		//Add water
		quadtree = new WaterQuadtree(window, camera, manager, texData, position);
		AddChild("Quadtree", quadtree);
	}

	Water::~Water()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(reflectionTexture.handle);
		man->removeReference<Texture>(refractionTexture.handle);

		man->removeReference<Texture>(dudv.handle);
	}

	void Water::PreRender(Renderer* renderer)
	{
		//Very bad
		cameraUnderwater = renderer->getCamera()->getPosition().y < getWorldTransform().getPosition().y;
		glEnable(GL_CLIP_DISTANCE0);

		__WaterConfig.distortionDelta += distortionDelta;
		motion += __WaterConfig.wavemotion;
		texData->motion = motion;
		texData->underwater = cameraUnderwater;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		//flipping upside down
		Matrix4f refl = Matrix4f::Identity();
		refl.m[1 * 4 + 1] = -1;
		refl.m[3 * 4 + 1] = 2 * -30;

		Camera* camera = renderer->getCamera();

		Matrix4f view = camera->getViewMatrix();
		Matrix4f refl_view = refl * view;
		camera->setViewMatrix(refl_view);
		camera->setViewProjectionMatrix(camera->getProjectionMatrix() * refl_view);

		//reflection
		reflectionFbo->Bind();
		reflectionFbo->Clear(0.0f);
		window->getSwapchain()->SetWindowSize(refl_size, refl_size);
		renderer->PrepareRendering();
		glFrontFace(GL_CCW);
		//Consts.CLIP_PLANE = Consts.REFLECTION_PLANE;
		//Consts.RENDER_STAGE = Consts.REFLECTION_STAGE;

		if (!cameraUnderwater)
		{
			RenderObjects(renderer);
		}

		glFinish();
		glFrontFace(GL_CW);
		reflectionFbo->Unbind();

		//flipping back
		camera->setViewMatrix(view);
		camera->setViewProjectionMatrix(camera->getProjectionMatrix() * view);

		//refraction
		refractionFbo->Bind();
		refractionFbo->Clear(0.0f);
		window->getSwapchain()->SetWindowSize(refr_size, refr_size);
		renderer->PrepareRendering();
		//Consts.CLIP_PLANE = Consts.REFRACTION_PLANE;
		//Consts.RENDER_STAGE = Consts.REFRACTION_STAGE;

		RenderObjects(renderer);

		glFinish();

		refractionFbo->Unbind();
		window->getSwapchain()->SetWindowSize(width, height);
		glDisable(GL_CLIP_DISTANCE0);
		//Consts.CLIP_PLANE = Consts.DEFAULT_PLANE;
		//Consts.RENDER_STAGE = Consts.DEFAULT_STAGE;

		//FFT stuff
		fft->Render();
		normalMapRenderer->Render(fft->getDyTexture());

		texData->normalMap = normalMapRenderer->getNormalmap().pointer;

		//Water rendering
		UpdateQuadtree();
		Node::PreRender(renderer);
	}

	void Water::UpdateQuadtree()
	{
		if (camera->isChanged())
		{
			quadtree->UpdateQuadtree();
		}
	}

	void Water::RenderObjects(Renderer* renderer) const
	{
		EnvironmentMapRenderer::instance->RenderCube(camera);
		GLRenderer* rend = (GLRenderer*)renderer;

		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(nullptr);
			pl->getShader()->UpdateGlobalUniforms(rend);

			GLLightCullingPass* lightCullingPass = rend->getLightCullingPass();
			lightCullingPass->getLightBuffer()->BindBase(nullptr, 0);
			lightCullingPass->getVisibleLightIndicesBuffer()->BindBase(nullptr, 1);

			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateMaterialUniforms(material.first, 0);

				for (auto renderer : material.second)
				{
					renderer->BatchRender();
				}
			}

			lightCullingPass->getVisibleLightIndicesBuffer()->UnbindBase(1);
			lightCullingPass->getLightBuffer()->UnbindBase(0);

			pl->UnbindPipeline();
		}
	}
}