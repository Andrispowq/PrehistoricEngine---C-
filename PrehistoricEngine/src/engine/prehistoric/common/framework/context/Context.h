#ifndef CONTEXT_H
#define CONTEXT_H

namespace Prehistoric
{
	class Window;

	class PR_API Context
	{
	public:
		Context(Window* window) : window(window) {}
		virtual ~Context() = 0;

		virtual void* getNativePhysicalDevice() { return nullptr; };
		virtual void* getNativeDevice() { return nullptr; };

		virtual void* getPhysicalDevice() { return nullptr; };
		virtual void* getDevice() { return nullptr; };

	protected:
		Window* window;
	};
};

#endif