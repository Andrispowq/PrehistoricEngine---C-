#ifndef VK_SWAP_CHAIN_H
#define VK_SWAP_CHAIN_H

#include "engine/prehistoric/common/framework/swapchain/Swapchain.h"
#include "engine/prehistoric/core/util/Includes.hpp"

#include <glfw3.h>
#define PR_INCLUDE_VULKAN
#include <vulkan/vulkan.h>

#include "engine/platform/vulkan/framework/device/VKDevice.h"
#include "engine/platform/vulkan/framework/surface/VKSurface.h"
#include "engine/platform/vulkan/rendering/command/VKCommandPool.h"
#include "engine/platform/vulkan/rendering/synchronisation/VKSemaphore.h"
#include "engine/platform/vulkan/rendering/synchronisation/VKFence.h"
#include "engine/platform/vulkan/rendering/renderpass/VKRenderpass.h"

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
	std::vector<VkImage> getSwapchainImages() const { return swapchainImages; }
	std::vector<VkImageView> getSwapchainImageViews() const { return swapchainImageViews; }

	VkImageView getColourImageView() { return colourImageView; }
	VkImageView getDepthImageView() { return depthImageView; }
	VkFormat getSwapchainImageFormat() const { return swapchainImageFormat; }
	VkExtent2D getSwapchainExtent() const { return swapchainExtent; }
	VKRenderpass* getRenderpass() { return renderpass; }

	void setSurface(VKSurface& surface) { this->surface = &surface; }
	void setRenderpass(VKRenderpass* renderpass) { this->renderpass = renderpass; }
private:
	//External
	VKPhysicalDevice* physicalDevice;
	VKDevice* device;
	VKSurface* surface;

	VKRenderpass* renderpass;

	//Swapchain imageviews and images
	VkSwapchainKHR swapchain;

	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	//For multisampling
	VkImage colourImage;
	VkDeviceMemory colourImageMemory;
	VkImageView colourImageView;
	
	//Depth buffer
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	//Synchronization
	std::vector<VKSemaphore*> imageAvailableSemaphores;
	std::vector<VKSemaphore*> renderFinishedSemaphores;
	std::vector<VKFence*> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;

	uint32_t aquiredImageIndex;
	uint32_t NumImages;
};

#endif
