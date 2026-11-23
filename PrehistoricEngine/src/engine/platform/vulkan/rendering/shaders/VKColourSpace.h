#ifndef VK_COLOUR_SPACE_H
#define VK_COLOUR_SPACE_H

#include <vulkan/vulkan.h>

namespace Prehistoric {
    // An sRGB attachment encodes linear fragment output on write. UNORM
    // attachments still need the shader's configured display gamma.
    inline float VKPBRShaderGamma(VkFormat attachmentFormat, float configuredGamma) {
        switch (attachmentFormat) {
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                return 1.0f;
            default:
                return configuredGamma;
        }
    }
}

#endif