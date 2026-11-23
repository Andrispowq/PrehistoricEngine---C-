#include "platform/vulkan/rendering/VKEnvironmentLighting.h"
#include "platform/vulkan/texture/VKImageMipmaps.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <memory>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>

using namespace Prehistoric;

static uint32_t validationErrors = 0;

static void Check(VkResult result) {
    if (result != VK_SUCCESS)
        throw std::runtime_error("Vulkan call failed: " + std::to_string(result));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT type,
                                                   const VkDebugUtilsMessengerCallbackDataEXT *data, void *user) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        validationErrors++;
        fprintf(stderr, "%s\n", data->pMessage);
    }
    return VK_FALSE;
}


static std::vector<char> ReadShader(const char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file || file.tellg() <= 0) throw std::runtime_error("Missing test shader");
    std::vector<char> result(size_t(file.tellg()));
    file.seekg(0);
    file.read(result.data(), result.size());
    return result;
}

static uint32_t MemoryType(VkPhysicalDevice gpu, uint32_t bits, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &properties);
    for (uint32_t i = 0; i < properties.memoryTypeCount; ++i)
        if ((bits & (1u << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags) return i;
    throw std::runtime_error("No suitable test memory");
}

struct Resources {
    VkDevice device = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkShaderModule module = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    ~Resources() {
        if (!device) return;
        vkDeviceWaitIdle(device);
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyShaderModule(device, module, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        vkDestroyDescriptorPool(device, pool, nullptr);
        vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroySampler(device, sampler, nullptr);
        vkDestroyImageView(device, view, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, imageMemory, nullptr);
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
        vkDestroyDevice(device, nullptr);
    }
};

static void RunIBL(VkPhysicalDevice gpu, uint32_t family, const std::vector<char>& conversion, const std::vector<char>& code,
                   const std::vector<char> &probe, const char *hdrPath) {
    Resources resources;
    uint32_t count;
    Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, nullptr));
    std::vector<VkExtensionProperties> extensions(count);
    Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, extensions.data()));
    std::vector<const char *> enabled;
    for (const auto &extension: extensions)
        if (!strcmp(extension.extensionName, "VK_KHR_portability_subset")) enabled.push_back(
            "VK_KHR_portability_subset");
    float priority = 1;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueInfo.queueFamilyIndex = family;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;
    VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = uint32_t(enabled.size());
    deviceInfo.ppEnabledExtensionNames = enabled.data();
    Check(vkCreateDevice(gpu, &deviceInfo, nullptr, &resources.device));
    VkDevice device = resources.device;
    VkQueue queue;
    vkGetDeviceQueue(device, family, 0, &queue);

    int hdrWidth, hdrHeight, channels;
    std::unique_ptr<float, decltype(&stbi_image_free)> hdr(
        stbi_loadf(hdrPath, &hdrWidth, &hdrHeight, &channels, 4), stbi_image_free);
    if (!hdr) throw std::runtime_error("Cannot load the real HDR seam regression fixture");
    const uint32_t width = uint32_t(hdrWidth), height = uint32_t(hdrHeight);
    const uint32_t mipLevels = uint32_t(std::floor(std::log2(width > height ? width : height))) + 1;
    const size_t bytes = size_t(width) * height * 4 * sizeof(float);
    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = bytes;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    Check(vkCreateBuffer(device, &bufferInfo, nullptr, &resources.buffer));
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, resources.buffer, &requirements);
    VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = MemoryType(gpu, requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Check(vkAllocateMemory(device, &allocation, nullptr, &resources.memory));
    Check(vkBindBufferMemory(device, resources.buffer, resources.memory, 0));

    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    Check(vkCreateImage(device, &imageInfo, nullptr, &resources.image));
    vkGetImageMemoryRequirements(device, resources.image, &requirements);
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = MemoryType(gpu, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    Check(vkAllocateMemory(device, &allocation, nullptr, &resources.imageMemory));
    Check(vkBindImageMemory(device, resources.image, resources.imageMemory, 0));
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = resources.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1};
    Check(vkCreateImageView(device, &viewInfo, nullptr, &resources.view));
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = float(mipLevels - 1);
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    Check(vkCreateSampler(device, &samplerInfo, nullptr, &resources.sampler));

    VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.queueFamilyIndex = family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    Check(vkCreateCommandPool(device, &poolInfo, nullptr, &resources.commandPool));
    VkCommandBufferAllocateInfo commandInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandInfo.commandPool = resources.commandPool;
    commandInfo.commandBufferCount = 1;
    commandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer command;
    Check(vkAllocateCommandBuffers(device, &commandInfo, &command));
    auto begin = [&]() {
        Check(vkResetCommandBuffer(command, 0));
        VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        Check(vkBeginCommandBuffer(command, &info));
    };
    auto submit = [&]() {
        Check(vkEndCommandBuffer(command));
        VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        info.commandBufferCount = 1;
        info.pCommandBuffers = &command;
        Check(vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE));
        Check(vkQueueWaitIdle(queue));
    };

    VkDescriptorSetLayoutBinding bindings[5]{};
    for (uint32_t i = 0; i < 5; ++i)
        bindings[i] = {
            i, i != 3 ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr
        };
    VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setInfo.bindingCount = 5;
    setInfo.pBindings = bindings;
    Check(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &resources.setLayout));
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.maxSets = 1;
    descriptorPoolInfo.poolSizeCount = 2;
    descriptorPoolInfo.pPoolSizes = poolSizes;
    Check(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &resources.pool));
    VkDescriptorSetAllocateInfo setAllocation{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    setAllocation.descriptorPool = resources.pool;
    setAllocation.descriptorSetCount = 1;
    setAllocation.pSetLayouts = &resources.setLayout;
    VkDescriptorSet set;
    Check(vkAllocateDescriptorSets(device, &setAllocation, &set));
    VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &resources.setLayout;
    Check(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &resources.layout));
    VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    moduleInfo.codeSize = probe.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t *>(probe.data());
    Check(vkCreateShaderModule(device, &moduleInfo, nullptr, &resources.module));
    VkComputePipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = resources.module;
    pipelineInfo.stage.pName = "main";
    pipelineInfo.layout = resources.layout;
    Check(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resources.pipeline));

    for (int pass = 0; pass < 4; ++pass) {
        // Black, constant HDR, analytic directional environment, then the actual
        // sample HDR with its small, bright sun. Exercise every source mip.
        float *mapped;
        Check(vkMapMemory(device, resources.memory, 0, bytes, 0, (void **) &mapped));
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                float longitude = ((float(x) + 0.5f) / width - 0.5f) * 6.2831853f;
                float latitude = (0.5f - (float(y) + 0.5f) / height) * 3.14159265f;
                float direction[] = {
                    cosf(latitude) * cosf(longitude), sinf(latitude), cosf(latitude) * sinf(longitude)
                };
                for (int c = 0; c < 3; ++c)
                    mapped[(y * width + x) * 4 + c] = pass == 0 ? 0 : float(2 + c) + (pass == 2 ? direction[c] : 0);
                mapped[(y * width + x) * 4 + 3] = 1;
            }
        }
        if (pass == 3) memcpy(mapped, hdr.get(), bytes);
        vkUnmapMemory(device, resources.memory);
        begin();
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.image = resources.image;
        barrier.subresourceRange = viewInfo.subresourceRange;
        barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.oldLayout = pass == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = pass == 0 ? 0 : VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        VkBufferImageCopy copy{};
        copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy.imageExtent = imageInfo.extent;
        vkCmdCopyBufferToImage(command, resources.buffer, resources.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &copy);
        VKRecordImageMipmaps(command, resources.image, width, height, mipLevels);
        submit();

        VKEnvironmentLighting lighting(gpu, device, queue, family,
                                       {resources.sampler, resources.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                                       16, 32, 4, 128, conversion, code, 32, 512);
        if (lighting.getMaxReflectionLOD() != 3) throw std::runtime_error("Incorrect reflection mip range");
        VkDescriptorImageInfo images[] = {lighting.getIrradiance(), lighting.getPrefilter(), lighting.getBRDF()};
        VkDescriptorImageInfo source = lighting.getEnvironment();
        VkDescriptorBufferInfo buffer{resources.buffer, 0, bytes};
        VkWriteDescriptorSet writes[5]{};
        for (uint32_t i = 0; i < 5; ++i) {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = set;
            writes[i].dstBinding = i;
            writes[i].descriptorType = bindings[i].descriptorType;
            writes[i].descriptorCount = 1;
            if (i < 3) writes[i].pImageInfo = &images[i];
            else if (i == 3) writes[i].pBufferInfo = &buffer;
            else writes[i].pImageInfo = &source;
        }
        vkUpdateDescriptorSets(device, 5, writes, 0, nullptr);
        begin();
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, resources.pipeline);
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, resources.layout, 0, 1, &set, 0, nullptr);
        vkCmdDispatch(command, 240, 1, 1);
        VkMemoryBarrier readback{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        readback.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        readback.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                             0, 1, &readback, 0, nullptr, 0, nullptr);
        submit();
        Check(vkMapMemory(device, resources.memory, 0, bytes, 0, (void **) &mapped));
        for (int i = 0; i < 36; ++i) {
            for (int c = 0; c < 3; ++c) {
                float value = mapped[i * 4 + c];
                if (!std::isfinite(value) || value < 0) throw std::runtime_error("Non-finite/negative IBL result");
                if (i >= 30) {
                    if (value > 1.05f) throw std::runtime_error("BRDF LUT outside expected range");
                } else if (pass < 2) {
                    if (fabsf(value - (pass == 0 ? 0 : float(2 + c) * (i < 6 ? 3.14159265f : 1.0f))) > 0.015f)
                        throw std::runtime_error("Constant HDR energy not preserved");
                } else if (pass == 2 && i < 12) {
                    int face = i % 6;
                    float direction = face / 2 == c ? (face % 2 ? -1.0f : 1.0f) : 0.0f;
                    if (c == 2) direction = -direction;
                    float expected = float(2 + c) + direction * (i < 6 ? 0.75f : 1.0f);
                    if (i < 6) value /= 3.14159265f;
                    if (fabsf(value - expected) > 0.09f)
                        throw std::runtime_error("Directional environment orientation/convolution mismatch");
                }
            }
        }
        if (mapped[30 * 4] < 0.1f) throw std::runtime_error("BRDF integration returned zero");
        if (pass == 2 && mapped[24 * 4] >= mapped[6 * 4] - 0.15f)
            throw std::runtime_error("Roughness did not broaden reflections");
        float worstBasis = 0.0f, worstCube = 0.0f;
        for (int pair = 0; pair < 288; ++pair) {
            float *a = mapped + (64 + pair * 2) * 4;
            float *b = a + 4;
            float difference = 0.0f, energy = 0.0f;
            for (int c = 0; c < 3; ++c) {
                if (!std::isfinite(a[c]) || !std::isfinite(b[c])) throw std::runtime_error("Non-finite seam sample");
                difference += (a[c] - b[c]) * (a[c] - b[c]);
                energy += a[c] * a[c] + b[c] * b[c];
            }
            float relative = sqrtf(difference / (energy > 0.000001f ? energy : 0.000001f));
            if (pair < 240) worstBasis = relative > worstBasis ? relative : worstBasis;
            else worstCube = relative > worstCube ? relative : worstCube;
        }
        printf("Environment %d: basis boundary %.6f%%, cube edge %.6f%%\n", pass, worstBasis * 100, worstCube * 100);
        if (worstBasis > 0.01f || worstCube > 0.01f) throw std::runtime_error("IBL sampling seam exceeds 1% tolerance");
        if (mapped[640 * 4] != 8.0f || mapped[641 * 4] < 1.0f)
            throw std::runtime_error("Filtered integration is not using the source mip chain");
        for (int c = 0; c < 3; ++c)
            if (fabsf(mapped[642 * 4 + c] - mapped[643 * 4 + c]) > 0.00001f)
                throw std::runtime_error("Mirror reflection no longer samples the sharp source");
        vkUnmapMemory(device, resources.memory);
    }
    printf("PASS: black/HDR energy, six cube faces, diffuse convolution, roughness mips, and BRDF LUT\n");
}

int main(int argc, char **argv) {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    int result = 0;
    try {
        if (argc != 5)
            throw std::runtime_error("Usage: VulkanIBLSmoke ibl.spv probe.spv convert.spv environment.hdr");
        auto code = ReadShader(argv[1]);
        auto probe = ReadShader(argv[2]);

        uint32_t count = 0;
        Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        std::vector<VkExtensionProperties> extensions(count);
        Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
        std::vector<const char *> enabled;
        VkInstanceCreateInfo info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        for (const auto &extension: extensions) {
            if (!strcmp(extension.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
                enabled.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
            if (!strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
                enabled.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        Check(vkEnumerateInstanceLayerProperties(&count, nullptr));
        std::vector<VkLayerProperties> layers(count);
        Check(vkEnumerateInstanceLayerProperties(&count, layers.data()));
        const char *validation = "VK_LAYER_KHRONOS_validation";
        for (const auto &layer: layers) {
            if (!strcmp(layer.layerName, validation)) {
                info.enabledLayerCount = 1;
                info.ppEnabledLayerNames = &validation;
            }
        }
        if (!info.enabledLayerCount)
            throw std::runtime_error("Install the Vulkan SDK validation layer to run this check!");
        VkApplicationInfo application = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
        application.apiVersion = VK_API_VERSION_1_1;
        info.pApplicationInfo = &application;
        info.enabledExtensionCount = (uint32_t) enabled.size();
        info.ppEnabledExtensionNames = enabled.data();
        Check(vkCreateInstance(&info, nullptr, &instance));
        auto createMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (!createMessenger)
            throw std::runtime_error("Vulkan debug utils are required for this check!");
        VkDebugUtilsMessengerCreateInfoEXT debug = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug.pfnUserCallback = DebugMessage;
        Check(createMessenger(instance, &debug, nullptr, &messenger));

        Check(vkEnumeratePhysicalDevices(instance, &count, nullptr));
        std::vector<VkPhysicalDevice> devices(count);
        Check(vkEnumeratePhysicalDevices(instance, &count, devices.data()));
        uint32_t tested = 0;
        for (VkPhysicalDevice gpu: devices) {
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, families.data());
            for (uint32_t i = 0; i < count; i++) {
                if (families[i].queueCount && ((families[i].queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT))
                                               == (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT))) {
                    RunIBL(gpu, i, ReadShader(argv[3]), code, probe, argv[4]);
                    tested++;
                    break;
                }
            }
        }
        if (!tested)
            throw std::runtime_error("No Vulkan compute device was found!");
    } catch (const std::exception &exception) {
        fprintf(stderr, "%s\n", exception.what());
        result = 1;
    }
    if (messenger)
        ((PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))(
            instance, messenger, nullptr);
    if (instance)
        vkDestroyInstance(instance, nullptr);
    if (validationErrors)
        result = 1;
    printf("Vulkan validation errors: %u\n", validationErrors);
    return result;
}