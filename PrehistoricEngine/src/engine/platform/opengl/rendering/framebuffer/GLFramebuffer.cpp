#include "Includes.hpp"
#include "GLFramebuffer.h"

#include "platform/opengl/texture/GLTexture.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLFramebuffer::GLFramebuffer(Window* window)
		: Framebuffer(window)
	{
		glGenFramebuffers(1, &id);
		depthAttachment = -1;
	}

	GLFramebuffer::~GLFramebuffer()
	{
		if (depthAttachment == -1)
			glDeleteRenderbuffers(1, &depthAttachment);

		for (auto attachment : multisampleAttachments)
		{
			glDeleteRenderbuffers(1, &attachment.second);
		}

		glDeleteFramebuffers(1, &id);
	}

	void GLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}

	void GLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLFramebuffer::Clear(const Vector4f& colour)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void GLFramebuffer::SetDrawAttachments(uint32_t n, uint32_t* attachments)
	{
		glDrawBuffers(n, attachments);
	}

	void GLFramebuffer::addDepthAttachment(uint32_t width, uint32_t height, bool multisample)
	{
		this->width = width;
		this->height = width;

		if (depthAttachment != -1)
			glDeleteRenderbuffers(1, &depthAttachment);

		glGenRenderbuffers(1, &depthAttachment);
		glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);

		if (multisample)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, __FrameworkConfig.windowNumSamples, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
		}
	}

	void GLFramebuffer::addColourAttachment2D(Texture* texture, uint32_t attachment, uint32_t mipLevel)
	{
		GLTexture* tex = (GLTexture*)texture;
		GLenum type = GL_COLOR_ATTACHMENT0 + attachment;
		if (tex->isDepth())
		{
			type = GL_DEPTH_ATTACHMENT;
		}

		if (tex->getType() == TEXTURE_ARRAY_2D)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->getTextureID(), 0);
			return;
		}

		if (tex->isMultisample())
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D_MULTISAMPLE, tex->getTextureID(), mipLevel);
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, tex->getTextureID(), mipLevel);
		}
	}

	void GLFramebuffer::addColourAttachmentMultisample2D(uint32_t attachment)
	{
		uint32_t id;
		glGenRenderbuffers(1, &id);
		glBindRenderbuffer(GL_RENDERBUFFER, id);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, __FrameworkConfig.windowNumSamples, GL_RGBA16F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_RENDERBUFFER, id);

		multisampleAttachments.push_back(std::make_pair(attachment, id));
	}

	void GLFramebuffer::addColourAttachment3D(Texture* texture, uint32_t face, uint32_t attachment, uint32_t mipLevel)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, static_cast<GLTexture*>(texture)->getTextureID(), mipLevel);
	}

	void GLFramebuffer::Check() const
	{
		uint32_t status;
		if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
		{
			PR_LOG_ERROR("ERROR: Framebuffer is not complete, status: %x\n", status);
		}
	}

	void GLFramebuffer::Blit(Framebuffer* destination, uint32_t width, uint32_t height, uint32_t source_attachment, uint32_t dest_attachment,
		uint32_t dest_width, uint32_t dest_height)
	{
		if (destination == nullptr)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
		else
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLFramebuffer*>(destination)->id);
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + source_attachment);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + dest_attachment);
		glBlitFramebuffer(0, 0, width, height, 0, 0, (dest_width == 0) ? width : dest_width, (dest_height == 0) ? height : dest_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
};
