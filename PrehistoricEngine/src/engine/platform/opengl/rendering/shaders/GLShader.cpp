#include "Includes.hpp"
#include "GLShader.h"

namespace Prehistoric
{
	std::vector<char> ResourceLoader::LoadShaderGL(const std::string& filename)
	{
		const std::string SHADER_LOC = "res/shaders/";

		std::ifstream file;
		file.open((SHADER_LOC + filename).c_str());

		std::string output;
		std::string line;

		if (file.is_open())
		{
			while (file.good())
			{
				std::getline(file, line);

				if (line.find("#include") == std::string::npos)
				{
					output.append(line + "\n");
				}
				else
				{
					std::string includeFile = Util::Split(line, ' ')[1];
					includeFile = includeFile.substr(1, includeFile.size() - 2);

					std::vector<char> toAppend = LoadShaderGL(includeFile);
					output.append(toAppend.data());
					output.append("\n");
				}
			}
		}
		else
		{
			PR_LOG_ERROR("Unable to load shader: %s\n", filename.c_str());
		}

		std::vector<char> out(output.begin(), output.end());
		return out;
	}

	GLShader::GLShader(const std::vector<char>* files, uint32_t length)
		: Shader()
	{
		program = glCreateProgram();

		shaders = new GLuint[5]{ 0 };
		counter = 0;

		if (program == 0)
		{
			PR_LOG_RUNTIME_ERROR("Shader program creation has failed!\n");
		}

		if (length == 1) //Compute shader
		{
			if (!AddShader(files[0], COMPUTE_SHADER))
				PR_LOG_RUNTIME_ERROR("Compute shader couldn't be added\n");
		}
		else if (length == 2) // Vertex + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}
		else if (length == 3) //Vertex + geometry + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], GEOMETRY_SHADER))
				PR_LOG_RUNTIME_ERROR("Geometry shader couldn't be added!");
			if (!AddShader(files[2], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}
		else if (length == 5) //Vertex + tess control + tess evaluation + geomtry + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], TESSELLATION_CONTROL_SHADER))
				PR_LOG_RUNTIME_ERROR("Tessellation control shader couldn't be added!");
			if (!AddShader(files[2], TESSELLATION_EVALUATION_SHADER))
				PR_LOG_RUNTIME_ERROR("Tessellation evaluation shader couldn't be added!");
			if (!AddShader(files[3], GEOMETRY_SHADER))
				PR_LOG_RUNTIME_ERROR("Geometry shader couldn't be added!");
			if (!AddShader(files[4], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}
	}

	GLShader::GLShader() : Shader()
	{
		program = glCreateProgram();

		shaders = new GLuint[5]{ 0 }; //Maximum of 5 shaders can be used - compute shader or the 5 graphics pipeline shaders
		counter = 0;

		if (program == 0)
		{
			PR_LOG_RUNTIME_ERROR("Shader program creation has failed!");
		}
	}

	GLShader::~GLShader()
	{
		for (uint32_t i = 0; i < counter; i++)
		{
			glDetachShader(program, shaders[i]);
			glDeleteShader(shaders[i]);
		}

		glDeleteProgram(program);
	}

	void GLShader::Bind(CommandBuffer* commandBuffer) const
	{
		glUseProgram(program);
	}

	void GLShader::Unbind() const
	{
		glUseProgram(0);
	}

	bool GLShader::AddUniform(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, size_t size, Texture* texture)
	{
		GLuint location = glGetUniformLocation(program, name.c_str());

		if (location == 0xFFFFFFFF)
		{
			PR_LOG_ERROR("Uniform %s was not found in a shader code\n", name.c_str());
			uniforms.emplace(name, 0xFFFFFFFF);
			return false;
		}

		uniforms.emplace(name, location);
		return true;
	}

	bool GLShader::AddUniformBlock(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, size_t size, Texture* texture)
	{
		GLuint location = glGetUniformBlockIndex(program, name.c_str());

		if (location == 0xFFFFFFFF)
		{
			PR_LOG_ERROR("Uniform block %s was not found in a shader code\n", name.c_str());
			uniforms.emplace(name, 0xFFFFFFFF);
			return false;
		}

		uniforms.emplace(name, location);
		return true;
	}

	void GLShader::BindAttribute(const std::string& name, GLuint location) const
	{
		glBindAttribLocation(program, location, name.c_str());
	}

	bool GLShader::CompileShader() const
	{
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLchar log[1024];
			glGetProgramInfoLog(program, 1024, NULL, log);

			PR_LOG_RUNTIME_ERROR("Shader program linking has failed!");

			return false;
		}

		glValidateProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLchar log[1024];
			glGetProgramInfoLog(program, 1024, NULL, log);

			PR_LOG_RUNTIME_ERROR("Shader program validation has failed!");

			return false;
		}

		return true;
	}

	bool GLShader::AddShader(const std::vector<char>& code, ShaderType type)
	{
		switch (type)
		{
		case VERTEX_SHADER:
			return AddProgram(code, GL_VERTEX_SHADER);
		case TESSELLATION_CONTROL_SHADER:
			return AddProgram(code, GL_TESS_CONTROL_SHADER);
		case TESSELLATION_EVALUATION_SHADER:
			return AddProgram(code, GL_TESS_EVALUATION_SHADER);
		case GEOMETRY_SHADER:
			return AddProgram(code, GL_GEOMETRY_SHADER);
		case FRAGMENT_SHADER:
			return AddProgram(code, GL_FRAGMENT_SHADER);
		case COMPUTE_SHADER:
			return AddProgram(code, GL_COMPUTE_SHADER);
		default:
			PR_LOG_ERROR("Invalid shader type given: %i\n", type);
			return false;
		}
	}

	bool GLShader::AddProgram(const std::vector<char>& code, GLenum type) const
	{
		GLuint shader = glCreateShader(type);

		shaders[counter++] = shader;

		if (shader == 0)
		{
			PR_LOG_ERROR("Shader creation failed!\n");

			return false;
		}

		const GLchar* charCode[1];
		charCode[0] = code.data();
		GLint lengths[1];
		lengths[0] = static_cast<GLint>(code.size());

		glShaderSource(shader, 1, charCode, lengths);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLchar log[1024];
			glGetShaderInfoLog(shader, 1024, NULL, log);

			PR_LOG_RUNTIME_ERROR("Shader stage addition has failed! Error: %s", log);

			return false;
		}

		glAttachShader(program, shader);

		return true;
	}
};