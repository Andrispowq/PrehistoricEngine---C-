#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

namespace Prehistoric
{
	class PR_API CommandBuffer
	{
	public:
		CommandBuffer() {}
		virtual ~CommandBuffer();

		virtual void BindBuffer() const {}
		virtual void UnbindBuffer() const {}

		virtual void DeleteBuffer() {}
	};
};

#endif