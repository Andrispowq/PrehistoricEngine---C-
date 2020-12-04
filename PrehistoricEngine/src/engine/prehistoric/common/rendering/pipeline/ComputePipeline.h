#ifndef COMPUTE_PIPELINE_H
#define COMPUTE_PIPELINE_H

#include "Pipeline.h"

#include "engine/prehistoric/core/util/math/Math.h"

#include "engine/prehistoric/common/buffer/ShaderStorageBuffer.h"

static uint32_t READ_ONLY = 0x1;
static uint32_t WRITE_ONLY = 0x2;
static uint32_t READ_WRITE = 0x3;

typedef uint32_t AccessMask;

class ComputePipeline
{
public:
	ComputePipeline(Vector3u invocationSize) : invocationSize(invocationSize) {}
	ComputePipeline() {}

	virtual ~ComputePipeline() {}

	std::unordered_map<uint32_t, std::pair<Texture*, AccessMask>>& getTextureBindingTable() { return textureBindingTable; }
	std::unordered_map<uint32_t, std::pair<ShaderStorageBuffer*, AccessMask>>& getSSBOBindingTable() { return ssboBindingTable; }
	Vector3u getInvocationSize() const { return invocationSize; }

	void addTextureBinding(uint32_t binding, Texture* texture, AccessMask accessMask) { textureBindingTable.insert(std::make_pair(binding, std::make_pair(texture, accessMask))); }
	void addSSBOBinding(uint32_t binding, ShaderStorageBuffer* ssbo, AccessMask accessMask) { ssboBindingTable.insert(std::make_pair(binding, std::make_pair(ssbo, accessMask))); }
	void setInvocationSize(const Vector3u& size) { this->invocationSize = size; }
protected:
	Vector3u invocationSize;

	std::unordered_map<uint32_t, std::pair<Texture*, AccessMask>> textureBindingTable;
	std::unordered_map<uint32_t, std::pair<ShaderStorageBuffer*, AccessMask>> ssboBindingTable;
};

#endif