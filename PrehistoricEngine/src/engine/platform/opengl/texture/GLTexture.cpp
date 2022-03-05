#include "Includes.hpp"
#include "GLTexture.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

namespace Prehistoric
{
	static GLenum getTexType(ImageType type, bool multisample)
	{
		if (type == TEXTURE_2D)
			if (multisample)
				return GL_TEXTURE_2D_MULTISAMPLE;
			else
				return GL_TEXTURE_2D;
		else if (type == TEXTURE_ARRAY_2D)
			return GL_TEXTURE_2D_ARRAY;
		else if (type == TEXTURE_CUBE_MAP)
			return GL_TEXTURE_CUBE_MAP;

		return -1;
	}

	GLTexture::GLTexture(uint32_t width, uint32_t height, ImageFormat format, ImageType type, bool multisample, bool depth)
		: Texture(width, height, format, type), multisample(multisample), depth(depth)
	{
		glGenTextures(1, &id);
	}

	GLTexture::GLTexture(uint32_t id, uint32_t width, uint32_t height, ImageFormat format, ImageType type)
		: id(id), Texture(width, height, format, type)
	{
	}

	GLTexture::GLTexture()
	{
		glGenTextures(1, &id);
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &id);
	}

	void GLTexture::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(getTexType(type, multisample), id);
	}

	void GLTexture::Unbind() const
	{
		glBindTexture(getTexType(type, multisample), 0);
	}

	void GLTexture::UploadTextureData(ImageData data)
	{
		if (data.channels == 3)
		{
			if ((((int32_t)data.width) & 3) != 0)
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 2 - (((int32_t)data.width) & 1));
			}

			glTexImage2D(GL_TEXTURE_2D, 0, data.type == ImageData::ImageType::HDR ? GL_RGB32F : GL_RGB, data.width, data.height, 0, GL_RGB, data.type == ImageData::ImageType::HDR ? GL_FLOAT : GL_UNSIGNED_BYTE, (void*)data.ptr.dataUC);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, data.type == ImageData::ImageType::HDR ? GL_RGBA32F : GL_RGBA, data.width, data.height, 0, data.type == ImageData::ImageType::HDR ? GL_RGB : GL_RGBA, data.type == ImageData::ImageType::HDR ? GL_FLOAT : GL_UNSIGNED_BYTE, (void*)data.ptr.dataUC);
		}
	}

	void GLTexture::SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps)
	{
		GLenum type_ = getTexType(type, multisample);

		switch (filter)
		{
		case Nearest:
			glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case Bilinear:
			glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Trilinear:
			glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if (generate_mipmaps)
				glGenerateMipmap(type_);
			break;
		case Anisotropic:
			glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if (generate_mipmaps)
				glGenerateMipmap(type_);

			if (GL_EXT_texture_filter_anisotropic)
			{
				GLfloat maxAnisotropyLevel;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropyLevel);
				glTexParameterf(type_, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropyLevel);
			}
			else
			{
				PR_LOG_ERROR("Anisotropic filtering is not supported!\n");
			}

			break;
		default:
			PR_LOG_ERROR("This filtering type does not exist for textures! Type: %u\n", filter);
			break;
		}

		switch (wrapMode)
		{
		case ClampToEdge:
			glTexParameteri(type_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(type_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			if (type_ == GL_TEXTURE_CUBE_MAP)
				glTexParameteri(type_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			break;
		case ClampToBorder:
			glTexParameteri(type_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(type_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			if (type_ == GL_TEXTURE_CUBE_MAP)
				glTexParameteri(type_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			break;
		case Repeat:
			glTexParameteri(type_, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(type_, GL_TEXTURE_WRAP_T, GL_REPEAT);
			if (type_ == GL_TEXTURE_CUBE_MAP)
				glTexParameteri(type_, GL_TEXTURE_WRAP_R, GL_REPEAT);
			break;
		case MirrorRepeat:
			glTexParameteri(type_, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(type_, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			if (type_ == GL_TEXTURE_CUBE_MAP)
				glTexParameteri(type_, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
			break;
		default:
			PR_LOG_ERROR("This type of texture wrapping does not exist! Type: %u\n", wrapMode);
			break;
		}
	}

	void GLTexture::GenerateMipmaps()
	{
		glGenerateMipmap(getTexType(type, multisample));
	}

	Texture* GLTexture::Storage2D(uint32_t width, uint32_t height, uint32_t levels, ImageFormat format, SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps, bool multisample)
	{
		bool depth = (format == D32_LINEAR) || (format == D32_SFLOAT);
		Texture* texture = new GLTexture(width, height, format, TEXTURE_2D, multisample, depth);
		texture->Bind();

		if (multisample)
		{
			//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, __FrameworkConfig.windowNumSamples, getInternalFormat(format), width, height, false);
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, __FrameworkConfig.windowNumSamples, getInternalFormat(format), width, height, true);
		}
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, levels, getInternalFormat(format), width, height);
			texture->SamplerProperties(filter, wrapMode, generate_mipmaps);
		}

		texture->Unbind();
		return texture;
	}

	Texture* GLTexture::Storage2DArray(uint32_t width, uint32_t height, uint32_t levels, ImageFormat format, SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps)
	{
		bool depth = (format == D32_LINEAR) || (format == D32_SFLOAT);
		Texture* texture = new GLTexture(width, height, format, TEXTURE_ARRAY_2D, false, depth);
		texture->Bind();

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, getInternalFormat(format), width, height, levels, 0, getFormat(format), GL_FLOAT, nullptr);


		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		texture->SamplerProperties(filter, wrapMode, generate_mipmaps);
		texture->Unbind();
		return texture;
	}

	Texture* GLTexture::Storage3D(uint32_t width, uint32_t height, uint32_t levels, ImageFormat format, SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps)
	{
		Texture* texture = new GLTexture(width, height, format, TEXTURE_CUBE_MAP);
		texture->Bind();

		for (uint32_t i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, getInternalFormat(format), width, height, 0, getFormat(format), GL_FLOAT, nullptr);
		}
		
		texture->SamplerProperties(filter, wrapMode, generate_mipmaps);
		texture->Unbind();
		return texture;
	}

	GLenum GLTexture::getImageType(ImageType type, bool multisample)
	{
		if (type == TEXTURE_2D)
			if (multisample)
				return GL_TEXTURE_2D_MULTISAMPLE;
			else
				return GL_TEXTURE_2D;
		else if (type == TEXTURE_ARRAY_2D)
			return GL_TEXTURE_2D_ARRAY;
		else if (type == TEXTURE_CUBE_MAP)
			return GL_TEXTURE_CUBE_MAP;

		PR_LOG_ERROR("Unsupported texture type: %u\n", type);

		return -1;
	}

	GLenum GLTexture::getInternalFormat(ImageFormat format)
	{
		if (format == R8_SRGB || format == R8_LINEAR)
			return GL_R8;
		if (format == R16_SFLOAT || format == R16_LINEAR)
			return GL_R16;
		if (format == R32_SFLOAT || format == R32_LINEAR)
			return GL_R32F;
		if (format == R8G8_SRGB || format == R8G8_LINEAR)
			return GL_RG16;
		if (format == R16G16_SFLOAT || format == R16G16_LINEAR)
			return GL_RG32F;
		if (format == R8G8B8_16_SRGB || format == R8G8B8_16_LINEAR)
			return GL_RGB16F;
		if (format == R8G8B8_SRGB || format == R8G8B8_LINEAR)
			return GL_RGB32F;
		if (format == R8G8B8A8_SRGB || format == R8G8B8A8_LINEAR)
			return GL_RGBA32F;
		if (format == D32_LINEAR || format == D32_SFLOAT)
			return GL_DEPTH_COMPONENT32F;

		PR_LOG_ERROR("Unsupported texture format: %u\n", format);

		return -1;
	}

	GLenum GLTexture::getFormat(ImageFormat format)
	{
		if (format == R8_SRGB || format == R8_LINEAR)
			return GL_RED;
		if (format == R16_SFLOAT || format == R16_LINEAR)
			return GL_RED;
		if (format == R32_SFLOAT || format == R32_LINEAR)
			return GL_RED;
		if (format == R8G8_SRGB || format == R8G8_LINEAR)
			return GL_RG;
		if (format == R16G16_SFLOAT || format == R16G16_LINEAR)
			return GL_RG;
		if (format == R8G8B8_16_SRGB || format == R8G8B8_16_LINEAR)
			return GL_RGB;
		if (format == R8G8B8_SRGB || format == R8G8B8_LINEAR)
			return GL_RGB;
		if (format == R8G8B8A8_SRGB || format == R8G8B8A8_LINEAR)
			return GL_RGBA;
		if (format == D32_LINEAR || format == D32_SFLOAT)
			return GL_DEPTH_COMPONENT;

		PR_LOG_ERROR("Unsupported texture format: %u\n", format);

		return -1;
	}

	GLenum GLTexture::getFormatForComputeShader(ImageFormat format)
	{
		if (format == R8_SRGB || format == R8_LINEAR)
			return GL_R8;
		if (format == R16_SFLOAT || format == R16_LINEAR)
			return GL_R16;
		if (format == R32_SFLOAT || format == R32_LINEAR)
			return GL_R32F;
		if (format == R8G8_SRGB || format == R8G8_LINEAR)
			return GL_RG16;
		if (format == R16G16_SFLOAT || format == R16G16_LINEAR)
			return GL_RG32F;
		if (format == R8G8B8_16_SRGB || format == R8G8B8_16_LINEAR)
			return GL_RGBA16F;
		if (format == R8G8B8_SRGB || format == R8G8B8_LINEAR)
			return GL_RGBA32F;
		if (format == R8G8B8A8_SRGB || format == R8G8B8A8_LINEAR)
			return GL_RGBA32F;

		PR_LOG_ERROR("Unsupported texture format: %u\n", format);

		return -1;
	}
};