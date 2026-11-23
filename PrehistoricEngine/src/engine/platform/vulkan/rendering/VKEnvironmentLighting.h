#ifndef VK_ENVIRONMENT_LIGHTING_H
#define VK_ENVIRONMENT_LIGHTING_H

#include <vulkan/vulkan.h>
#include <vector>

namespace Prehistoric
{
    struct AtmosphereParameters;

    // Owns immutable, linear HDR IBL maps. Baking completes before scene rendering.
    class VKEnvironmentLighting
    {
    public:
        static constexpr uint32_t DefaultLutSize = 256;
        static constexpr uint32_t DefaultSamples = 512;

        VKEnvironmentLighting(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue,
                              uint32_t queueFamily, VkDescriptorImageInfo source,
                              uint32_t irradianceSize, uint32_t prefilterSize, uint32_t levels,
                              uint32_t cubeSize, const std::vector<char>& conversion, const std::vector<char>& shader,
                              uint32_t lutSize = DefaultLutSize, uint32_t samples = DefaultSamples,
                              const AtmosphereParameters* sky = nullptr, VKEnvironmentLighting* reuseLutFrom = nullptr);

        ~VKEnvironmentLighting();

        VKEnvironmentLighting(const VKEnvironmentLighting &) = delete;
        VKEnvironmentLighting &operator=(const VKEnvironmentLighting &) = delete;

        VkDescriptorImageInfo getEnvironment() const;
        VkDescriptorImageInfo getIrradiance() const;
        VkDescriptorImageInfo getPrefilter() const;
        VkDescriptorImageInfo getBRDF() const;

        float getMaxReflectionLOD() const {
            return float(prefilter.levels - 1);
        }

    private:
        struct Map {
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkImageView view = VK_NULL_HANDLE;
            std::vector<VkImageView> storageViews;
            uint32_t size = 0;
            uint32_t levels = 0;
            uint32_t layers = 0;
        };

        void CreateMap(Map &map, uint32_t size, uint32_t levels, uint32_t layers);
        void DestroyMap(Map &map);
        void Bake(VkQueue queue, uint32_t queueFamily, VkDescriptorImageInfo source,
                  const std::vector<char>& conversion, const std::vector<char>& shader, uint32_t samples,
                  const AtmosphereParameters* sky);

        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSampler sampler = VK_NULL_HANDLE;
        Map environment;
        Map irradiance;
        Map prefilter;
        Map brdf;
        uint32_t lutSamples;
    };
}

#endif