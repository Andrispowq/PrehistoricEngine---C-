#ifndef VK_SWAP_CHAIN_H
#define VK_SWAP_CHAIN_H

#include "prehistoric/common/framework/swapchain/Swapchain.h"
#include "Includes.hpp"

#include <GLFW/glfw3.h>
#define PR_INCLUDE_VULKAN
#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include "platform/vulkan/rendering/synchronisation/VKSemaphore.h"
#include "platform/vulkan/rendering/synchronisation/VKFence.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "platform/vulkan/texture/VKImage.h"

namespace Prehistoric
{
	const static uint32_t MAX_FRAMES_IN_FLIGHT = 3;

	class VKSwapchain : public Swapchain
	{
	public:
		VKSwapchain(Window* window);
		virtual ~VKSwapchain();

		virtual void SwapBuffers(CommandBuffer* buffer) override;
		virtual void ClearScreen() override;

		virtual void SetVSync(bool vSync) const override {}
		virtual void SetWindowSize(uint32_t width, uint32_t height) override;

		virtual uint32_t getAquiredImageIndex() const override { return aquiredImageIndex; }

		bool GetNextImageIndex(); //If return false, we should recreate the command buffers, and the renderpass along with the framebuffers

		VkSwapchainKHR getSwapchain() const { return swapchain; }
		std::vector<VKImage*> getSwapchainImages() const { return swapchainImages; }

		VKImage* getColourImage() { return multisampleColourImage; }
		VKImage* getDepthImage() { return depthImage; }
		VkFormat getSwapchainImageFormat() const { return swapchainImageFormat; }
		VkExtent2D getSwapchainExtent() const { return swapchainExtent; }
		VKRenderpass* getRenderpass() { return renderpass; }

		void setSurface(VKSurface& surface) { this->surface = &surface; }
		void setRenderpass(VKRenderpass* renderpass) { this->renderpass = renderpass; }

	public:
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	private:
		//External
		VKDevice* device;
		VKSurface* surface;

		VKRenderpass* renderpass;

		//Swapchain imageviews and images
		VkSwapchainKHR swapchain;

		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;

		std::vector<VKImage*> swapchainImages;
		VKImage* multisampleColourImage;
		VKImage* depthImage;

		//Synchronization
		std::vector<VKSemaphore*> imageAvailableSemaphores;
		std::vector<VKSemaphore*> renderFinishedSemaphores;
		std::vector<VKFence*> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		uint32_t aquiredImageIndex;
		uint32_t NumImages;
	};
};

#endif
