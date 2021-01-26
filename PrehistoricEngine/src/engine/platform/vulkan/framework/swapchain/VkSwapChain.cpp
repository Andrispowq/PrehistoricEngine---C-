 #include "Includes.hpp"
#include "VKSwapchain.h"

#include "platform/vulkan/framework/context/VKContext.h"

namespace Prehistoric
{
    VKSwapchain::VKSwapchain(Window* window)
        : Swapchain(window)
    {
        VKContext* cont = (VKContext*)window->getContext();

        this->device = (VKDevice*)cont->getDevices();
        this->surface = cont->getSurface();

        SwapchainSupportDetails swapchainSupport = surface->QuerySwapChainSupport(device->getPhysicalDevice());

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

        NumImages = swapchainSupport.capabilities.minImageCount + 1;

        if (swapchainSupport.capabilities.maxImageCount > 0 && NumImages > swapchainSupport.capabilities.maxImageCount)
        {
            NumImages = swapchainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface->getSurface();
        createInfo.minImageCount = NumImages;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device->getQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        {
            PR_LOG_RUNTIME_ERROR("Failed to create swap chain!\n");
        }

        //Create images and image views
        std::vector<VkImage> swapImages;

        vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &NumImages, nullptr);
        swapImages.resize(NumImages);
        vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &NumImages, swapImages.data());

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;

        swapchainImages.resize(NumImages);
        for (size_t i = 0; i < NumImages; i++)
        {
            swapchainImages[i] = new VKImage(device, swapImages[i], 1, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        //Create multisampled image
        multisampleColourImage = new VKImage(device, swapchainExtent.width, swapchainExtent.height, 1, swapchainImageFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device->getSampleCount(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        //Create depth buffer
        VkFormat depthFormat = FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        depthImage = new VKImage(device, swapchainExtent.width, swapchainExtent.height, 1, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device->getSampleCount(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

        VKUtil::TransitionImageLayout(device, depthImage->getImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

        //Create synchronisation primitives
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(NumImages, VK_NULL_HANDLE);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            imageAvailableSemaphores[i] = new VKSemaphore(device->getDevice());
            renderFinishedSemaphores[i] = new VKSemaphore(device->getDevice());
            inFlightFences[i] = new VKFence(device->getDevice());
        }
    }

    VKSwapchain::~VKSwapchain()
    {
        VkDevice dev = device->getDevice();

        vkDeviceWaitIdle(dev);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            delete renderFinishedSemaphores[i];
            delete imageAvailableSemaphores[i];
            delete inFlightFences[i];
        }

        delete depthImage;
        delete multisampleColourImage;

        for (auto image : swapchainImages)
        {
            vkDestroyImageView(dev, image->getImageView(), nullptr);
        }

        vkDestroySwapchainKHR(dev, swapchain, nullptr);
    }

    //TODO: for VKRenderer
    bool VKSwapchain::GetNextImageIndex()
    {
        //get the next available image to render to
        vkWaitForFences(device->getDevice(), 1, &(inFlightFences[currentFrame]->getFence()), VK_TRUE, UINT64_MAX);

        VkResult res = vkAcquireNextImageKHR(device->getDevice(), swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame]->getSemaphore(), VK_NULL_HANDLE, &aquiredImageIndex);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            SetWindowSize(window->getWidth(), window->getHeight());
            return false;
        }

        if (imagesInFlight[aquiredImageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(device->getDevice(), 1, &imagesInFlight[aquiredImageIndex], VK_TRUE, UINT64_MAX);
        }

        imagesInFlight[aquiredImageIndex] = inFlightFences[currentFrame]->getFence();

        return true;
    }

    void VKSwapchain::SwapBuffers(CommandBuffer* buffer)
    {
        //Submit the graphics queue for rendering
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]->getSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &((VKCommandBuffer*)buffer)->getCommandBuffer();

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]->getSemaphore() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        inFlightFences[currentFrame]->ResetFence();

        if (vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, inFlightFences[currentFrame]->getFence()) != VK_SUCCESS)
        {
            PR_LOG_RUNTIME_ERROR("Failed to submit draw command buffer!\n");
        }

        //Present the results on the present queue
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &aquiredImageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(device->getPresentQueue().getQueue(), &presentInfo);
        vkQueueWaitIdle(device->getPresentQueue().getQueue());

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VKSwapchain::ClearScreen()
    {
        /*VkClearColorValue value = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };

        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.levelCount = 1;
        range.layerCount = 1;

        commandPool->getCommandBuffer(aquiredImageIndex)->BindBuffer();
        vkCmdClearColorImage(commandPool->getCommandBuffer(aquiredImageIndex)->getCommandBuffer(), swapchainImages[aquiredImageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &value, 1, &range);
        commandPool->getCommandBuffer(aquiredImageIndex)->UnbindBuffer();*/
    }

    void VKSwapchain::SetWindowSize(uint32_t width, uint32_t height)
    {
        VkDevice dev = device->getDevice();

        vkDeviceWaitIdle(dev);

        //We destroy some objects that should be destroyed
        delete depthImage;
        delete multisampleColourImage;

        for (auto image : swapchainImages)
        {
            vkDestroyImageView(dev, image->getImageView(), nullptr);
        }
        swapchainImages.clear();

        vkDestroySwapchainKHR(device->getDevice(), swapchain, nullptr);

        //Then we create new of them
        SwapchainSupportDetails swapchainSupport = surface->QuerySwapChainSupport(device->getPhysicalDevice());

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

        NumImages = swapchainSupport.capabilities.minImageCount + 1;

        if (swapchainSupport.capabilities.maxImageCount > 0 && NumImages > swapchainSupport.capabilities.maxImageCount)
        {
            NumImages = swapchainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface->getSurface();
        createInfo.minImageCount = NumImages;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device->getQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        {
            PR_LOG_RUNTIME_ERROR("Failed to create swap chain!\n");
        }

        //Create images and image views
        std::vector<VkImage> swapImages;

        vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &NumImages, nullptr);
        swapImages.resize(NumImages);
        vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &NumImages, swapImages.data());

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;

        swapchainImages.resize(NumImages);
        for (size_t i = 0; i < NumImages; i++)
        {
            swapchainImages[i] = new VKImage(device, swapImages[i], 1, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        //Create multisampled image
        multisampleColourImage = new VKImage(device, swapchainExtent.width, swapchainExtent.height, 1, swapchainImageFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device->getSampleCount(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        //Create depth buffer
        VkFormat depthFormat = FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        depthImage = new VKImage(device, swapchainExtent.width, swapchainExtent.height, 1, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device->getSampleCount(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

        VKUtil::TransitionImageLayout(device, depthImage->getImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }
    
    VkSurfaceFormatKHR VKSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VKSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VKSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = { FrameworkConfig::windowWidth, FrameworkConfig::windowHeight };

            actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat VKSwapchain::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        PR_LOG_RUNTIME_ERROR("Couldn't find suitable format!\n");
        return VK_FORMAT_R8G8B8A8_SRGB;
    }
};