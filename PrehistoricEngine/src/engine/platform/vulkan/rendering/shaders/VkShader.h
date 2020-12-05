#ifndef VK_SHADER_H
#define VK_SHADER_H

#include "prehistoric/common/rendering/shaders/Shader.h"

#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/framework/device/VKDevice.h"

#include "platform/vulkan/rendering/descriptors/VKDescriptorPool.h"

#include "prehistoric/core/node/GameObject.h"
#include "Includes.hpp"

namespace Prehistoric
{
	struct UniformBufferObject
	{
		Matrix4f transformation;
		Matrix4f view;
		Matrix4f projection;
	};

	class VKShader : public Shader
	{
	public:
		VKShader(Context* context, Swapchain* swapchain, const std::vector<char>* files, uint32_t length);
		VKShader(Context* context, Swapchain* swapchain);

		virtual ~VKShader();

		virtual void Bind(CommandBuffer* commandBuffer) const override { this->commandBuffer = commandBuffer; }
		virtual void Unbind() const override {}

		void BindSets(CommandBuffer* commandBuffer, uint32_t first, uint32_t set_count, uint32_t instance_index = 0) const;

		virtual bool AddUniform(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, size_t size, Texture* texture = nullptr) override;
		virtual bool AddUniformBlock(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, size_t size, Texture* texture = nullptr) override { return true; }

		virtual bool AddShader(const std::vector<char>& code, ShaderType type) override;
		virtual bool CompileShader() const override { return true; }

		//Uniform handling
		virtual void SetUniformi(const std::string& name, int value, size_t offset = 0, uint32_t instance_index = 0) const override;
		virtual void SetUniformf(const std::string& name, float value, size_t offset = 0, uint32_t instance_index = 0) const override;

		virtual void SetUniform(const std::string& name, const Vector2f& value, size_t offset = 0, uint32_t instance_index = 0) const override;
		virtual void SetUniform(const std::string& name, const Vector3f& value, size_t offset = 0, uint32_t instance_index = 0) const override;
		virtual void SetUniform(const std::string& name, const Vector4f& value, size_t offset = 0, uint32_t instance_index = 0) const override;
		virtual void SetUniform(const std::string& name, const Matrix4f& value, size_t offset = 0, uint32_t instance_index = 0) const override;

		virtual void SetTexture(const std::string& name, Texture* value, uint32_t instance_index = 0) const override;

		virtual void SetUniform(const std::string& name, const void* value, size_t size, size_t offset = 0, uint32_t instance_index = 0) const override;

		virtual void BindUniformBlock(const std::string& name, uint32_t binding, uint32_t instance_index = 0) const override {}

		virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override = 0;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override = 0;

		VkPipelineShaderStageCreateInfo* GetShaderStages() { return shaderStages; }
		VkPipelineLayout& GetPipelineLayout() { return pipelineLayout; }
		uint32_t getModuleCount() { return counter; }

		void RegisterInstance();
	private:
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
		void CreateStageInfo(VkPipelineShaderStageCreateInfo& info, VkShaderModule module, const char* main, VkShaderStageFlagBits type);

	protected:
		//Outside data
		VKPhysicalDevice* physicalDevice;
		VKDevice* device;
		VKSwapchain* swapchain;

		mutable CommandBuffer* commandBuffer; //The command buffer to use

		//Module data
		VkShaderModule* modules;
		VkPipelineShaderStageCreateInfo* shaderStages;

		//Pipeline data
		VkPipelineLayout pipelineLayout;
		VKDescriptorPool* descriptorPool;
	};
};

#endif