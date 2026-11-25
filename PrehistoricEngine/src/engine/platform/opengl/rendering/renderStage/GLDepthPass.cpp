#include "Includes.hpp"
#include "GLDepthPass.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

namespace Prehistoric
{
	GLDepthPass::GLDepthPass(Renderer* renderer)
		: RenderStage(renderer), framebuffer{nullptr}
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		depthTexture = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		man->addReference<Shader>(depthShader.handle);

		framebuffer = std::make_unique<GLFramebuffer>(window);
		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	GLDepthPass::~GLDepthPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Shader>(depthShader.handle);
		man->removeReference<Texture>(depthTexture.handle);
	}

	void GLDepthPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(depthTexture.handle);
		depthTexture = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

    void GLDepthPass::DrawBatches(const ModelBatches& batches, Shader& shader)
    {
        VertexBuffer* vertices = nullptr;
        Renderer::VisitBatches(batches, [&](Pipeline* pipeline)
        {
            vertices = static_cast<GLGraphicsPipeline*>(pipeline)->getVertexBuffer();
            if (vertices->getType() == VertexBufferType::MESH) vertices->Bind(nullptr);
        }, [](Pipeline*, Material*) {}, [&](Pipeline*, RenderableComponent* object)
        {
            if (vertices->getType() != VertexBufferType::MESH) return;
            shader.UpdateObjectUniforms(object->getParent());
            for (uint32_t i = 0; i < vertices->getSubmeshCount(); ++i)
            {
                vertices->Draw(nullptr, i);
            }
        }, [&](Pipeline*)
        {
            if (vertices->getType() == VertexBufferType::MESH) vertices->Unbind();
        });
    }

	void GLDepthPass::Render()
	{
		PR_PROFILE("Depth pass");

		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		framebuffer->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		framebuffer->SetDrawAttachments(1, arr);
		framebuffer->Clear(0.0f);

		depthShader->Bind(nullptr);
		depthShader->UpdateGlobalUniforms(rend);

		if (rend->getAtmosphere())
		{
			rend->getAtmospherePass().DrawDepth(*rend->getAtmosphere(), *static_cast<GLDepthPassShader*>(depthShader.pointer));
		}
		DrawBatches(rend->getModels3D(), *depthShader.pointer);
		DrawBatches(rend->getModelsTransparency(), *depthShader.pointer);

		depthShader->Unbind();
		framebuffer->Unbind();
	}
};
