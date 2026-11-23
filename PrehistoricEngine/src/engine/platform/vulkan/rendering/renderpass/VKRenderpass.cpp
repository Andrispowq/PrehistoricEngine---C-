#include "Includes.hpp"
#include "VKRenderpass.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "platform/vulkan/rendering/shaders/VKColourSpace.h"

#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric
{
	static void Check(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Vulkan fullscreen pass creation failed (%d)!\n", result);
		}
	}

	VkPipeline VKRenderpass::CreateFullscreenPipeline(const char* vertexPath, const char* fragmentPath,
		VkPipelineLayout layout, uint32_t subpass)
	{
		VkShaderModule modules[2]{};
		VkPipelineShaderStageCreateInfo shaderStages[2]{};
		const char* paths[] = {vertexPath, fragmentPath};

		for (size_t i = 0; i < 2; ++i)
		{
			auto code = ResourceLoader::LoadShaderVK(paths[i]);
			VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
			moduleInfo.codeSize = code.size();
			moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
			Check(vkCreateShaderModule(device->getDevice(), &moduleInfo, nullptr, &modules[i]));

			shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStages[i].module = modules[i];
			shaderStages[i].pName = "main";
		}

		VkPipelineVertexInputStateCreateInfo vertex{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		viewport.viewportCount = viewport.scissorCount = 1;

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamicStates;

		VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = VK_CULL_MODE_NONE;
		raster.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		multisample.rasterizationSamples = subpass == 0 ? swapchain->getSceneSamples() : VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
		depth.depthTestEnable = subpass == 0;
		depth.depthWriteEnable = VK_FALSE;
		depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		VkPipelineColorBlendAttachmentState attachment{};
		attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
									VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		blend.attachmentCount = 1;
		blend.pAttachments = &attachment;

		VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		info.stageCount = 2;
		info.pStages = shaderStages;
		info.pVertexInputState = &vertex;
		info.pInputAssemblyState = &assembly;
		info.pViewportState = &viewport;
		info.pDynamicState = &dynamic;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisample;
		info.pDepthStencilState = &depth;
		info.pColorBlendState = &blend;
		info.layout = layout;
		info.subpass = subpass;
		info.renderPass = renderpass;
		VkPipeline pipeline{};
		const VkResult result = vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &info, nullptr,
														&pipeline);

		for (auto module: modules)
		{
			vkDestroyShaderModule(device->getDevice(), module, nullptr);
		}
		Check(result);
		return pipeline;
	}

	VKRenderpass::VKRenderpass(VKDevice* device, VKSwapchain* swapchain)
		: device(device), swapchain(swapchain)
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapchain->getColourImage()->getImageFormat();
		colorAttachment.samples = swapchain->getSceneSamples();
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = swapchain->FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		depthAttachment.samples = swapchain->getSceneSamples();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = swapchain->getColourImage()->getImageFormat();
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 3;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		bool multisampled = swapchain->getSceneSamples() != VK_SAMPLE_COUNT_1_BIT;
		if (multisampled) subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkAttachmentDescription presentation = colorAttachmentResolve;
		presentation.format = swapchain->getSwapchainImageFormat();
		presentation.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference output{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
		VkAttachmentReference input{multisampled ? 3u : 0u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		if (!multisampled) colorAttachment.finalLayout = input.layout;
		VkSubpassDescription post{};
		post.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		post.inputAttachmentCount = post.colorAttachmentCount = 1;
		post.pInputAttachments = &input;
		post.pColorAttachments = &output;
		VkSubpassDescription subpasses[] = { subpass, post };
		VkSubpassDependency dependencies[] =
		{
			{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0 },
			{ 0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT },
			{ VK_SUBPASS_EXTERNAL, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			  0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0 }
		};
		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment, presentation, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = multisampled ? 4 : 3;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 2;
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = 3;
		renderPassInfo.pDependencies = dependencies;

		if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create render pass!\n");
		}
		VkDevice handle = device->getDevice();
		VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
		VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		setInfo.bindingCount = 1;
		setInfo.pBindings = &binding;
		Check(vkCreateDescriptorSetLayout(handle, &setInfo, nullptr, &hdrSetLayout));
		VkDescriptorPoolSize size{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1};
		VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		poolInfo.maxSets = poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &size;
		Check(vkCreateDescriptorPool(handle, &poolInfo, nullptr, &hdrPool));
		VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		alloc.descriptorPool = hdrPool;
		alloc.descriptorSetCount = 1;
		alloc.pSetLayouts = &hdrSetLayout;
		Check(vkAllocateDescriptorSets(handle, &alloc, &hdrSet));
		VkDescriptorImageInfo image{VK_NULL_HANDLE, swapchain->getHDRImage()->getImageView(), input.layout};
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstSet = hdrSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		write.pImageInfo = &image;
		vkUpdateDescriptorSets(handle, 1, &write, 0, nullptr);
		VkPushConstantRange range{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(float)};
		VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layout.setLayoutCount = layout.pushConstantRangeCount = 1;
		layout.pSetLayouts = &hdrSetLayout;
		layout.pPushConstantRanges = &range;
		Check(vkCreatePipelineLayout(handle, &layout, nullptr, &hdrLayout));
		hdrPipeline = CreateFullscreenPipeline("vulkan/postProcessing/hdr_VS.spv", "vulkan/postProcessing/hdr_FS.spv", hdrLayout, 1);

	}

	VKRenderpass::~VKRenderpass()
	{
		VkDevice handle = device->getDevice();
		vkDestroyPipeline(handle, hdrPipeline, nullptr);
		vkDestroyPipelineLayout(handle, hdrLayout, nullptr);
		vkDestroyDescriptorPool(handle, hdrPool, nullptr);
		vkDestroyDescriptorSetLayout(handle, hdrSetLayout, nullptr);
		vkDestroyRenderPass(device->getDevice(), renderpass, nullptr);
	}

	std::unique_ptr<VKFramebuffer> VKRenderpass::CreateFramebuffer(uint32_t imageIndex) const
	{
		VkImageView attachments[] =
		{
			swapchain->getColourImage()->getImageView(),
			swapchain->getDepthImage()->getImageView(),
			swapchain->getSwapchainImages()[imageIndex]->getImageView(),
			swapchain->getHDRImage()->getImageView()
		};
		VkExtent2D extent = swapchain->getSwapchainExtent();
		uint32_t count = swapchain->getSceneSamples() == VK_SAMPLE_COUNT_1_BIT ? 3 : 4;
		return std::make_unique<VKFramebuffer>(device->getDevice(), renderpass, extent.width, extent.height, count, attachments);
	}

	void VKRenderpass::BeginRenderpass(VKCommandBuffer* commandBuffer, VKFramebuffer* framebuffer, VkExtent2D swapchainExtent, Vector4f clearColour)
	{
		VkClearValue clearValues[2] = {};
		clearValues[0].color = { clearColour.x, clearColour.y, clearColour.z, clearColour.w };
		clearValues[1].depthStencil = { 1, 0 };

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffer->getFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchainExtent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffer->getCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VKRenderpass::RenderHDR(VKCommandBuffer* commandBuffer)
	{
		VkCommandBuffer command = commandBuffer->getCommandBuffer();
		vkCmdNextSubpass(command, VK_SUBPASS_CONTENTS_INLINE);
		VkExtent2D extent = swapchain->getSwapchainExtent();
		VkViewport viewport{0, 0, float(extent.width), float(extent.height), 0, 1};
		VkRect2D scissor{{0, 0}, extent};
		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, hdrPipeline);
		vkCmdSetViewport(command, 0, 1, &viewport);
		vkCmdSetScissor(command, 0, 1, &scissor);
		vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, hdrLayout, 0, 1, &hdrSet, 0, nullptr);
		float settings[] = { __EngineConfig.rendererExposure,
			VKPBRShaderGamma(swapchain->getSwapchainImageFormat(), __EngineConfig.rendererGamma) };
		vkCmdPushConstants(command, hdrLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(settings), settings);
		vkCmdDraw(command, 3, 1, 0, 0);
	}
};