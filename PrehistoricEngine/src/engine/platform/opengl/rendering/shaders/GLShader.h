#ifndef GL_SHADER_H
#define GL_SHADER_H

#include "engine/prehistoric/common/rendering/shaders/Shader.h"

#include <glew.h>

#include <fstream>

#include "engine/prehistoric/core/node/component/light/Light.h"
#include "engine/prehistoric/core/util/Util.h"

class GLShader : public Shader
{
public:
	GLShader(const std::vector<char>* files, uint32_t length);
	GLShader();

	virtual ~GLShader() override;

	void Bind(CommandBuffer* commandBuffer) const override;
	void Unbind() const override;

	bool AddUniform(const std::string& name, uint32_t stages = GRAPHICS_PIPELINE, UniformType type = UniformBuffer, uint32_t set = 0, uint32_t binding = 0, size_t size = 0, Texture* texture = nullptr) override;
	bool AddUniformBlock(const std::string& name, uint32_t stages = GRAPHICS_PIPELINE, UniformType type = UniformBuffer, uint32_t set = 0, uint32_t binding = 0, size_t size = 0, Texture* texture = nullptr) override;
	void BindAttribute(const std::string& name, GLuint location) const;

	bool AddShader(const std::vector<char>& code, ShaderType type) override;
	bool CompileShader() const override;

	virtual void UpdateConstantUniforms(Camera* camera, const std::vector<Light*>& lights) const {}
	virtual void UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index = 0) const override {};
	virtual void UpdateSharedUniforms(GameObject* object, uint32_t instance_index = 0) const override {}
	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override {};

	virtual void SetUniformi(const std::string& name, int value, size_t offset = 0, uint32_t instance_index = 0) const override { glUniform1i(getUniformLocation(name), value); }
	virtual void SetUniformf(const std::string& name, float value, size_t offset = 0, uint32_t instance_index = 0) const override { glUniform1f(getUniformLocation(name), value); }

	virtual void SetUniform(const std::string& name, const Vector2f& value, size_t offset = 0, uint32_t instance_index = 0) const override { glUniform2f(getUniformLocation(name), value.x, value.y); }
	virtual void SetUniform(const std::string& name, const Vector3f& value, size_t offset = 0, uint32_t instance_index = 0) const override { glUniform3f(getUniformLocation(name), value.x, value.y, value.z); }
	virtual void SetUniform(const std::string& name, const Vector4f& value, size_t offset = 0, uint32_t instance_index = 0) const override { glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w); }
	virtual void SetUniform(const std::string& name, const Matrix4f& matrix, size_t offset = 0, uint32_t instance_index = 0) const override { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix.m); }

	virtual void SetTexture(const std::string& name, Texture* value, uint32_t instance_index = 0) const {};
	virtual void SetUniform(const std::string& name, const void* value, size_t size, size_t offset = 0, uint32_t instance_index = 0) const override {}

	void SetUniformi(int location, int value) const { glUniform1i(location, value); }
	void SetUniformf(int location, float value) const { glUniform1f(location, value); }

	void SetUniform(int location, const Vector2f& value) const { glUniform2f(location, value.x, value.y); }
	void SetUniform(int location, const Vector3f& value) const { glUniform3f(location, value.x, value.y, value.z); }
	void SetUniform(int location, const Vector4f& value) const { glUniform4f(location, value.x, value.y, value.z, value.w); }
	void SetUniform(int location, const Matrix4f& value) const { glUniformMatrix4fv(location, 1, GL_FALSE, value.m); }

	virtual void BindUniformBlock(const std::string& name, uint32_t binding, uint32_t instance_index = 0) const override { glUniformBlockBinding(program, getUniformLocation(name), binding); }

	void BindFragDataLocation(const std::string& name, GLuint index)
	{
		glBindFragDataLocation(program, index, name.c_str());
	}
private:
	bool AddProgram(const std::vector<char>& code, GLenum type) const;

	uint32_t getUniformLocation(const std::string& name) const
	{
		return uniforms.at(name);
	}
protected:
	std::unordered_map<std::string, uint32_t> uniforms;

	GLuint program;
	GLuint* shaders;
};

#endif 