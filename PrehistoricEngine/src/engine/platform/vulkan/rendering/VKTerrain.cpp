#include "Includes.hpp"
#include "VKTerrain.h"
#include "command/VKCommandPool.h"
#include "pipeline/VKComputePipeline.h"
#include "prehistoric/core/modules/terrain/Terrain.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/terrain/TerrainMaps.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	namespace
	{
		void CheckBake(VkResult result)
		{
			if (result != VK_SUCCESS)
				throw std::runtime_error("Vulkan terrain map bake failed: " + std::to_string(result));
		}

		struct BakeResources
		{
			VkDevice device;
			VkDescriptorSetLayout setLayout{};
			VkDescriptorPool pool{};
			VkPipelineLayout layout{};
			VkShaderModule module{};
			~BakeResources()
			{
				vkDestroyShaderModule(device, module, nullptr);
				vkDestroyPipelineLayout(device, layout, nullptr);
				vkDestroyDescriptorPool(device, pool, nullptr);
				vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
			}
		};
	}

	void BakeVulkanTerrainMaps(Window* window, AssembledAssetManager* manager, TextureHandle height,
		TextureHandle& normal, TextureHandle& splat, std::vector<float>& heights)
	{
		auto* gpu = static_cast<VKDevice*>(window->getContext()->getDevices());
		const VkDevice device = gpu->getDevice();
		const uint32_t size = height->getWidth();
		if (size == 0 || size != height->getHeight())
			throw std::runtime_error("Terrain requires a square heightmap");

		auto normals = VKTexture::CreateStorageMap(gpu, size, size);
		auto weights = VKTexture::CreateStorageMap(gpu, size, size);
		VKBuffer readback(gpu, size_t(size) * size * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		BakeResources resources{device};
		const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
		VkDescriptorSetLayoutBinding bindings[4]{};
		for (uint32_t i = 0; i < 4; ++i)
			bindings[i] = {i, types[i], 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
		VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		setInfo.bindingCount = 4;
		setInfo.pBindings = bindings;
		CheckBake(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &resources.setLayout));

		const VkDescriptorPoolSize counts[] = {{types[0], 1}, {types[1], 2}, {types[3], 1}};
		VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = 3;
		poolInfo.pPoolSizes = counts;
		CheckBake(vkCreateDescriptorPool(device, &poolInfo, nullptr, &resources.pool));
		VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		alloc.descriptorPool = resources.pool;
		alloc.descriptorSetCount = 1;
		alloc.pSetLayouts = &resources.setLayout;
		VkDescriptorSet set;
		CheckBake(vkAllocateDescriptorSets(device, &alloc, &set));

		auto* source = static_cast<VKTexture*>(height.pointer);
		VkDescriptorImageInfo images[] = {
			{source->getSampler(), source->getImage()->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
			{VK_NULL_HANDLE, normals->getImage()->getImageView(), VK_IMAGE_LAYOUT_GENERAL},
			{VK_NULL_HANDLE, weights->getImage()->getImageView(), VK_IMAGE_LAYOUT_GENERAL}};
		VkDescriptorBufferInfo buffer{readback.getBuffer(), 0, readback.getSize()};
		VkWriteDescriptorSet writes[4]{};
		for (uint32_t i = 0; i < 4; ++i)
		{
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = set;
			writes[i].dstBinding = i;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = types[i];
			if (i < 3) writes[i].pImageInfo = &images[i];
			else writes[i].pBufferInfo = &buffer;
		}
		vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);

		struct BakeParams { int32_t pass; float strength; } params{0, TerrainMaps::normalStrength};
		VkPushConstantRange range{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params)};
		VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &resources.setLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &range;
		CheckBake(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &resources.layout));
		const auto code = ResourceLoader::LoadShaderVK("vulkan/terrain/maps.comp.spv");
		VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		moduleInfo.codeSize = code.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		CheckBake(vkCreateShaderModule(device, &moduleInfo, nullptr, &resources.module));
		VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stage.module = resources.module;
		stage.pName = "main";
		VKComputePipeline pipeline(device, stage, resources.layout);
		VKCommandPool commands(gpu);
		commands.AddCommandBuffer();
		auto* recording = commands.getCommandBuffer(0);
		recording->BindBuffer();
		VkCommandBuffer cmd = recording->getCommandBuffer();
		VkImageMemoryBarrier transitions[2]{};
		for (unsigned i = 0; i < 2; ++i)
		{
			auto& barrier = transitions[i];
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = (i == 0 ? normals : weights)->getImage()->getImage();
			barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		}
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 2, transitions);
		pipeline.BindPipeline(cmd);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, resources.layout, 0, 1, &set, 0, nullptr);
		uint32_t groups = (size + 15) / 16;
		vkCmdPushConstants(cmd, resources.layout, range.stageFlags, 0, sizeof(params), &params);
		vkCmdDispatch(cmd, groups, groups, 1);
		VkMemoryBarrier dependency{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		dependency.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
			1, &dependency, 0, nullptr, 0, nullptr);
		params.pass = 1;
		vkCmdPushConstants(cmd, resources.layout, range.stageFlags, 0, sizeof(params), &params);
		vkCmdDispatch(cmd, groups, groups, 1);
		for (auto& barrier : transitions)
		{
			barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		dependency.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &dependency, 0, nullptr, 2, transitions);
		recording->UnbindBuffer();

		VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		VkQueue queue = gpu->getGraphicsQueue().getQueue();
		CheckBake(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE));
		CheckBake(vkQueueWaitIdle(queue));
		heights.resize(size_t(size) * size);
		void* data;
		CheckBake(vkMapMemory(device, readback.getMemory(), 0, readback.getSize(), 0, &data));
		std::memcpy(heights.data(), data, readback.getSize());
		vkUnmapMemory(device, readback.getMemory());

		auto* assets = manager->getAssetManager();
		normal = assets->storeTexture(normals.release());
		splat = assets->storeTexture(weights.release());
		assets->addReference<Texture>(normal.handle);
		assets->addReference<Texture>(splat.handle);
	}

	void CreateVulkanTerrainSurface(Terrain& terrain, Window* window, TerrainMaps& maps, AssembledAssetManager* manager)
	{
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(*static_cast<VkPhysicalDevice*>(window->getContext()->getNativePhysicalDevice()), &features);
		if (!features.tessellationShader)
		{
			PR_LOG_ERROR("Vulkan terrain requires tessellationShader.\n");
			return;
		}

		Mesh mesh;
		mesh.setMaterialIndex(-1);
		const auto patch = terrain.getQuadtree()->generatePatch();
		for (uint32_t i = 0; i < patch.size(); ++i)
		{
			const auto& point = patch[i];
			mesh.AddIndex(i);
			mesh.AddVertex(Vertex(Vector3f(point.x, 0, point.y), point));
		}

		Model model;
		model.AddMesh(std::move(mesh));
		auto* assets = manager->getAssetManager();
		auto vertices = assets->storeVertexBuffer(new VKMeshVertexBuffer(window, model));
		auto shader = assets->loadShader(ShaderName::Terrain).value();
		auto pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vertices);
		auto layers = maps.getMaterials();
		if (layers.empty())
		{
			PR_LOG_ERROR("Vulkan terrain requires at least one surface material.\n");
			return;
		}

		auto* material = new Material(assets, "Terrain");
		material->addTexture("heightMap", maps.getHeightmapHandle());
		material->addTexture("terrainNormal", maps.getNormalmapHandle());
		material->addTexture("terrainSplat", maps.getSplatmapHandle());
		for (size_t i = 0; i < 3; ++i)
		{
			auto layer = layers[(std::min)(i, layers.size() - 1)];
			const auto& textures = layer->getTextures();
			const std::string suffix = std::to_string(i);
			for (const char* name : {"albedoMap", "normalMap", "mrotMap", "emissionMap"})
			{
				auto found = textures.find(name);
				material->addTexture(name + suffix, found == textures.end() ? textures.at("DEFAULT_TEX") : found->second);
			}

			material->addFloat("heightScale" + suffix, layer->getFloat("heightScale"));
			material->addFloat("hasEmission" + suffix, textures.count("emissionMap") ? 1.0f : 0.0f);
			material->addFloat("tiling" + suffix, layer->getFloat("horizontalScale"));
			material->addVector4f("mrot" + suffix, layer->getVector4f("mrot"));
		}

		terrain.SetPosition(terrain.getQuadtree()->getWorldTransform().getPosition());
		terrain.SetScale({__TerrainConfig.scaleXZ, __TerrainConfig.scaleY, __TerrainConfig.scaleXZ});
		terrain.AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, manager->storeMaterial(material)));
	}
}
