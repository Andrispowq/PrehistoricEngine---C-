#ifndef GL_SWAP_CHAIN_H
#define GL_SWAP_CHAIN_H

#include "prehistoric/common/framework/swapchain/Swapchain.h"

#include <glew.h>
#include <glfw3.h>

namespace Prehistoric
{
	class GLSwapchain : public Swapchain
	{
	public:
		GLSwapchain(Window* window);
		virtual ~GLSwapchain();

		virtual void SwapBuffers(CommandBuffer* buffer) override;
		virtual void ClearScreen() override;

		virtual void SetVSync(bool vSync) const override;
		virtual void SetWindowSize(uint32_t width, uint32_t height) override;
	};
};

#endif