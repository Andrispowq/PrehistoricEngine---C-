#ifndef SHADER_H
#define SHADER_H

#include "engine/prehistoric/core/util/Includes.hpp"

#include "engine/prehistoric/common/texture/Texture.h"

#include "engine/prehistoric/core/math/Math.h"

#include "engine/prehistoric/common/rendering/command/CommandBuffer.h"

#include <vulkan/vulkan.h>

enum ShaderType
{
	VERTEX_SHADER = 0x00000001,
	TESSELLATION_CONTROL_SHADER = 0x00000002,
	TESSELLATION_EVALUATION_SHADER = 0x00000004,
	GEOMETRY_SHADER = 0x00000008,
	FRAGMENT_SHADER = 0x00000010,
	COMPUTE_SHADER = 0x00000020,
	GRAPHICS_PIPELINE = 0x0000001F,
	TASK_SHADER_NV = 0x00000040,
	MESH_SHADER_NV = 0x00000080,
	MESH_SHADING_PIPELINE_NV = 0x000000c0,
	RAY_GENERATION_SHADER_KHR = 0x00000100,
	RAY_HIT_SHADER_KHR = 0x00000200,
	RAY_CLOSEST_HIT_SHADER_KHR = 0x00000400,
	RAY_MISS_SHADER_KHR = 0x00000800,
	RAY_INTERSECTION_SHADER_KHR = 0x00001000,
	RAY_CALLABLE_SHADER_KHR = 0x00002000,
	UNKNOWN = 0xFFFFFFFF
};

enum ShaderCodeType
{
	GLSL,
	HLSL,
	SPIR_V_BINARY,
	SPIR_V_ASSEMBLY
};

enum UniformType
{
	Sampler = 0,
	CombinedImageSampler = 1,
	SampledImage = 2,
	StorageImage = 3,
	UniformTexelBuffer = 4,
	StorageTexelBuffer = 5,
	UniformBuffer = 6,
	StorageBuffer = 7,
	UniformBufferDynamic = 8,
	StorageBufferDynamic = 9,
	InputAttachment = 10,
	InlineUniformBlock_EXT = 1000138000,
	AccelerationStructure_KHR = 1000165000,
};


namespace ResourceLoader
{
	std::vector<char> LoadShaderGL(const std::string& filename);
	std::vector<char> LoadShaderVK(const std::string& filename);
};

class GameObject;
class Camera;
class Light;

class Shader
{
public:
	Shader() : counter(0) {}
	virtual ~Shader() = 0;

	virtual void Bind(CommandBuffer* commandBuffer) const = 0;
	virtual void Unbind() const = 0;

	virtual bool AddUniform(const std::string& name, uint32_t stages = GRAPHICS_PIPELINE, UniformType type = UniformBuffer, uint32_t set = 0, uint32_t binding = 0, size_t size = 0, Texture* texture = nullptr) = 0;
	virtual bool AddUniformBlock(const std::string& name, uint32_t stages = GRAPHICS_PIPELINE, UniformType type = UniformBuffer, uint32_t set = 0, uint32_t binding = 0, size_t size = 0, Texture* texture = nullptr) = 0;

	virtual bool AddShader(const std::vector<char>& code, ShaderType type) = 0;
	virtual bool CompileShader() const = 0;

	//Uniform handling
	virtual void SetUniformi(const std::string& name, int value, size_t offset = 0, uint32_t instance_index = 0) const = 0;
	virtual void SetUniformf(const std::string& name, float value, size_t offset = 0, uint32_t instance_index = 0) const = 0;

	virtual void SetUniform(const std::string& name, const Vector2f& value, size_t offset = 0, uint32_t instance_index = 0) const = 0;
	virtual void SetUniform(const std::string& name, const Vector3f& value, size_t offset = 0, uint32_t instance_index = 0) const = 0;
	virtual void SetUniform(const std::string& name, const Vector4f& value, size_t offset = 0, uint32_t instance_index = 0) const = 0;
	virtual void SetUniform(const std::string& name, const Matrix4f& value, size_t offset = 0, uint32_t instance_index = 0) const = 0;

	virtual void SetTexture(const std::string& name, Texture* value, uint32_t instance_index = 0) const = 0;

	//General SetUniform method for Uniform Buffer Objects, like uploading view and projection matrix to the same uniform binding
	virtual void SetUniform(const std::string& name, const void* value, size_t size, size_t offset = 0, uint32_t instance_index = 0) const = 0;

	virtual void BindUniformBlock(const std::string& name, uint32_t binding, uint32_t instance_index = 0) const = 0;

	//Constant uniforms -> uniforms that change very infrequently
	//Shader uniforms -> per-shader uniforms, like view and projection matrices
	//Shared uniforms -> uniforms that are shared between objects, and can be fetched from the first object
	//Object uniforms -> unique, per-object values
	virtual void UpdateConstantUniforms(Camera* camera, const std::vector<Light*>& lights) const {}
	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const {}
	virtual void UpdateSharedUniforms(GameObject* object, uint32_t instance_index = 0) const {}
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const {}

	Shader(const Shader&) = delete;
	Shader(const Shader&&) = delete;
	Shader& operator=(Shader) = delete;
protected:
	mutable uint32_t counter;
};

#endif