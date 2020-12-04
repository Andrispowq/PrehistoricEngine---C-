#include "engine/prehistoric/core/util/Includes.hpp"
#include "Texture.h"

int Texture::currID = 0;

Texture::Texture(uint32_t width, uint32_t height, ImageFormat format, ImageType type)
	: width(width), height(height), format(format), type(type)
{
	ID = currID++;
}

Texture::Texture()
	: width(0), height(0), format(R8G8B8A8_LINEAR), type(TEXTURE_2D)
{
	ID = currID++;
}

Texture::~Texture()
{
	ID = 0xFFFFFFFF;
}