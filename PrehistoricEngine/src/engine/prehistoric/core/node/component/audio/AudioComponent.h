#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"
#include "prehistoric/core/node/GameObject.h"

#include <AL/al.h>

namespace Prehistoric
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(size_t audioBuffer, float startOffset = 0.0f, bool is3D = true, bool isStereo = false, bool isLoop = false);
		AudioComponent(const std::string& audio, float startOffset = 0.0f, bool is3D = true, bool isStereo = false, bool isLoop = false);
		~AudioComponent();

		virtual void PreUpdate(CoreEngine* engine);

		void setGain(float gain);
		void setPitch(float pitch);
		void setVelocity(const Vector3f& velocity);

		uint32_t getSourceID() const { return sourceID; }
		size_t getBufferIndex() const { return bufferIndex; }

		float getStartOffset() const { return startOffset; }

		bool is3D() const { return is_3D; }
		bool isStereo() const { return is_Stereo; }
		bool isLoop() const { return is_Loop; }
		bool isSpatial() const { return is_Spatial; }

		void setBufferIndex(size_t bufferIndex);

		void set3D(bool is_3D);
		void setStereo(bool is_Stereo);
		void setLoop(bool is_Loop);
		void setSpatial(bool is_Spatial) { this->is_Spatial = is_Spatial; }

		static ComponentType getStaticComponentType() { return ComponentType::AudioComponent; }

	private:
		uint32_t sourceID;
		uint32_t bufferID;
		size_t bufferIndex;

		float startOffset;

		bool is_3D;
		bool is_Stereo;
		bool is_Loop;
		bool is_Spatial;
	};
};

#endif