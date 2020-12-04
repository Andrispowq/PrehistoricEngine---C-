#ifndef SSBO_H
#define SSBO_H

#include "engine/prehistoric/core/math/Math.h"

#include "engine/prehistoric/common/rendering/command/CommandBuffer.h"

enum class LayoutType
{
	UNSIGNED_INT,
	SIGNED_INT,
	FLOAT,
	DOUBLE,
	VECTOR2F,
	VECTOR3F,
	VECTOR4F
};

enum class LayoutTypeInfo
{
	SINGLE_ELEMENT,
	SIZED_ARRAY,
	UNSIZED_ARRAY
};

struct LayoutMember
{
	LayoutType type;
	LayoutTypeInfo typeInfo;
	size_t count;
};

struct Layout
{
	constexpr static size_t MAX_MEMBERS = 10;

	LayoutMember members[MAX_MEMBERS];
	size_t memberCount = 0;

	void addLayoutMember(LayoutType type, LayoutTypeInfo typeInfo, size_t count)
	{
		if (memberCount == MAX_MEMBERS)
		{
			PR_LOG_RUNTIME_ERROR("Tried to add a new member to a Layout descriptor for ShaderStorageBuffer, but already reached the maximum!\n");
		}

		members[memberCount].count = count;
		members[memberCount].type = type;
		members[memberCount].typeInfo = typeInfo;

		memberCount++;
	}

	size_t getSize() const
	{
		size_t size = 0;

		for (size_t i = 0; i < memberCount; i++)
		{
			LayoutMember member = members[i];

			switch (member.type)
			{
			case LayoutType::UNSIGNED_INT:
				size += sizeof(uint32_t) * member.count;
				break;
			case LayoutType::SIGNED_INT:
				size += sizeof(int32_t) * member.count;
				break;
			case LayoutType::FLOAT:
				size += sizeof(float) * member.count;
				break;
			case LayoutType::DOUBLE:
				size += sizeof(double) * member.count;
				break;
			case LayoutType::VECTOR2F:
				size += Vector2f::size() * member.count;
				break;
			case LayoutType::VECTOR3F:
				size += Vector3f::size() * member.count;
				break;
			case LayoutType::VECTOR4F:
				size += Vector4f::size() * member.count;
				break;
			}
		}

		return size;
	}
};

class ShaderStorageBuffer
{
public:
	ShaderStorageBuffer(void* data, const Layout& layout) : data(data), layout(layout) {}
	virtual ~ShaderStorageBuffer() = 0;

	virtual void Bind(CommandBuffer* commandBuffer, uint32_t binding) const = 0;
	virtual void Unbind() const = 0;

	virtual void MapBuffer() = 0;
	virtual void UnmapBuffer() = 0;

	void* getMappedData() const { return data; }
protected:
	void* data;
	Layout layout;
};

#endif