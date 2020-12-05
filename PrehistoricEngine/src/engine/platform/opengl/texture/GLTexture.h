#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "engine/prehistoric/common/texture/Texture.h"

#include <glew.h>

namespace Prehistoric
{
	class GLTexture : public Texture
	{
	public:
		GLTexture(uint32_t width, uint32_t height, ImageFormat format = R8G8B8A8_LINEAR, ImageType type = TEXTURE_2D);
		GLTexture();

		virtual ~GLTexture() override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void Unbind() const override;

		virtual void UploadTextureData(unsigned char* data, ImageFormat format) override;
		virtual void Generate() override {}

		void UploadHDRTextureData(float* data);

		virtual void SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode) override;

		inline GLuint getTextureID() const { return id; }
	public:
		//These methods give the caller the responsibility of deleting the generated textures! They must be registered to the AssetManagers list
		static Texture* GenTexture(const std::string& file, SamplerFilter filter = Trilinear, TextureWrapMode wrapMode = Repeat);
		static Texture* Storage3D(uint32_t width, uint32_t height, uint32_t level = 0, ImageFormat format = R8G8B8A8_LINEAR, SamplerFilter filter = Trilinear, TextureWrapMode wrapMode = Repeat);
		static Texture* Storage2D(uint32_t width, uint32_t height, uint32_t levels = 1, ImageFormat format = R8G8B8A8_LINEAR, SamplerFilter filter = Trilinear, TextureWrapMode wrapMode = Repeat);

		static GLenum getImageType(ImageType imageType);
		static GLenum getInternalFormat(ImageFormat format);
		static GLenum getFormat(ImageFormat format);
	private:
		GLuint id;
	};
};

#endif