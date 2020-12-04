#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLContext.h"
#include "engine/platform/Platform.h"

static void OpenGLMessageCallback(
	unsigned source,
	unsigned type,
	unsigned id,
	unsigned severity,
	int length,
	const char* message,
	const void* userParam)
{
	std::string output;

	output.append("OpenGL Debug Callback:\n");
	output.append("\tID: %u\n");
	output.append("\tType: %s\n");
	output.append("\tSource: %s\n");
	output.append("\tSeverity: %s\n");
	output.append("\tMessage: %s\n\n");

	std::string sType;
	
	switch (type)
	{
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		sType = "DEPRECATED_BEHAVIOR or DEPRECATED_BEHAVIOR_ARB";
		break;
	case GL_DEBUG_TYPE_ERROR:
		sType = "ERROR or ERROR_ARB";
		break;
	case GL_DEBUG_TYPE_MARKER:
		sType = "MARKER";
		break;
	case GL_DEBUG_TYPE_OTHER:
		sType = "OTHER or OTHER_ARB";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		sType = "PERFORMANCE or PERFORMANCE_ARB";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		sType = "POP_GROUP";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		sType = "PORTABILITY or PORTABILITY_ARB";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		sType = "PUSH_GROUP";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		sType = "UNDEFINED_BEHAVIOR or UNDEFINED_BEHAVIOR_ARB";
		break;
	default:
		PR_LOG_RUNTIME_ERROR("Unknow OpenGL debug type: %u\n", type);
	}

	std::string sSource;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		sSource = "API or API_ARB";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		sSource = "APPLICATION or APPLICATION_ARB";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		sSource = "OTHER or OTHER_ARB";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sSource = "SHADER_COMPILER or SHADER_COMPILER_ARB";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sSource = "THIRD_PARTY or THIRD_PARTY_ARB";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sSource = "WINDOW_SYSTEM or WINDOW_SYSTEM_ARB";
		break;
	default:
		PR_LOG_RUNTIME_ERROR("Unknow OpenGL debug source: %u\n", type);
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:  
		PR_LOG_RUNTIME_ERROR(output, id, sType.c_str(), sSource.c_str(), "HIGH", message);
		return;
	case GL_DEBUG_SEVERITY_MEDIUM:
		PR_LOG_ERROR(output, id, sType.c_str(), sSource.c_str(), "MEDIUM", message);
		return;
	case GL_DEBUG_SEVERITY_LOW:
		PR_LOG_WARNING(output, id, sType.c_str(), sSource.c_str(), "LOW", message);
		return;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		PR_LOG(CYAN, output, id, sType.c_str(), sSource.c_str(), "NOTIFICATION", message);
		return;
	default:
		return;
	}
}

GLContext::GLContext(Window* window)
	: Context(window)
{
	glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(window->getWindowHandle()));

	GLenum error = glewInit();

	glewExperimental = GL_FALSE;

	if (error != GLEW_OK)
	{
		PR_LOG_ERROR("The initialisation of OpenGL Extension Wrangler Library (GLEW) has failed! The error: \n%s\n", glewGetErrorString(error));

		glfwTerminate();
	}

#if defined(PR_ENABLE_DEBUGGING)
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLMessageCallback, nullptr);

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif
}