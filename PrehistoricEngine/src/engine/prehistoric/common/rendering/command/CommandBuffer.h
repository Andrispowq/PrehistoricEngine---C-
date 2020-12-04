#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

class CommandBuffer
{
public:
	CommandBuffer() {}
	virtual ~CommandBuffer();

	virtual void BindBuffer() const {}
	virtual void UnbindBuffer() const {}

	virtual void DeleteBuffer() {}
};

#endif