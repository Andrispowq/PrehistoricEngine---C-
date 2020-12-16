#ifndef GL_FRAMEBUFFER_H
#define GL_FRAMEBUFFER_H

#include "prehistoric/common/rendering/framebuffer/Framebuffer.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLFramebuffer : public Framebuffer
	{
	public:
		GLFramebuffer(Window* window);
		virtual ~GLFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Clear(const Vector4f& colour) override;
		virtual void SetDrawAttachments(uint32_t attachments...) override; //TODO: this is OpenGL only!

		virtual void addDepthAttachment(uint32_t width, uint32_t height) override;
		virtual void addColourAttachment2D(Texture* texture, uint32_t attachment = 0, uint32_t mipLevel = 0) override;
		virtual void addColourAttachment3D(Texture* texture, uint32_t face, uint32_t attachment = 0, uint32_t mipLevel = 0) override;

		virtual void Blit(Framebuffer* destination, uint32_t width, uint32_t height, uint32_t source_attachment = 0, uint32_t dest_attachment = 0) override;

	private:
		uint32_t id;

		uint32_t depthAttachment;
	};
};

#endif