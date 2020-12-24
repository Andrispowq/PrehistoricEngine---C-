#ifndef TEXTURE_H
#define TEXTURE_H

namespace Prehistoric
{
	//from vulkan.h
	enum ImageFormat
	{
		R8G8B8A8_SRGB, R8G8B8_SRGB, R8G8B8_16_SRGB, D32_SFLOAT, R16_SFLOAT, R32_SFLOAT, R8_SRGB, R8G8_SRGB, R16G16_SFLOAT,
		R8G8B8A8_LINEAR, R8G8B8_LINEAR, R8G8B8_16_LINEAR, D32_LINEAR, R16_LINEAR, R32_LINEAR, R8_LINEAR, R8G8_LINEAR, R16G16_LINEAR
	};

	enum ImageType
	{
		TEXTURE_2D, TEXTURE_ARRAY_2D, TEXTURE_CUBE_MAP
	};

	enum SamplerFilter
	{
		Nearest, Bilinear, Trilinear, Anisotropic
	};

	enum TextureWrapMode
	{
		ClampToEdge, ClampToBorder, Repeat, MirrorRepeat
	};

	class Texture
	{
	public:
		Texture(uint32_t width, uint32_t height, ImageFormat format = R8G8B8A8_LINEAR, ImageType type = TEXTURE_2D);
		Texture();

		virtual ~Texture() = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual void UploadTextureData(unsigned char* data, ImageFormat format) = 0;
		virtual void Generate() = 0;

		virtual void SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps = true) = 0;
		virtual void GenerateMipmaps() = 0;

		inline uint32_t getWidth() const { return width; }
		inline uint32_t getHeight() const { return height; }
		inline uint32_t getID() const { return ID; }

		inline ImageFormat getFormat() const { return format; }
		inline ImageType getType() const { return type; }

		inline void setWidth(uint32_t width) { this->width = width; }
		inline void setHeight(uint32_t height) { this->height = height; }
		inline void setID(uint32_t id) { this->ID = id; }

		inline void setFormat(ImageFormat format) { this->format = format; }
		inline void setType(ImageType type) { this->type = type; }

		//Non-copyable, but movable
		Texture(Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
	protected:
		uint32_t width, height;
		uint32_t ID;

		ImageFormat format;
		ImageType type;

		static int currID;
	};
};

#endif