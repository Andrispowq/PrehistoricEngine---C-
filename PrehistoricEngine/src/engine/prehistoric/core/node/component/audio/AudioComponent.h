#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"
#include "prehistoric/core/node/GameObject.h"

#include <al.h>

namespace Prehistoric
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(size_t audioBuffer, bool is3D = true, bool isStereo = false, bool isLoop = false);
		AudioComponent(const std::string& audio, bool is3D = true, bool isStereo = false, bool isLoop = false);
		~AudioComponent();

		virtual void PreUpdate(Engine* engine);

		void setGain(float gain);
		void setPitch(float pitch);
		void setVelocity(const Vector3f& velocity);

		uint32_t getSourceID() const { return sourceID; }
		size_t getBufferIndex() const { return bufferIndex; }

		bool is3D() const { return is_3D; }
		bool isStereo() const { return is_Stereo; }
		bool isLoop() const { return is_Loop; }

		void setBufferIndex(size_t bufferIndex);

		void set3D(bool is_3D);
		void setStereo(bool is_Stereo);
		void setLoop(bool is_Loop);

	private:
		uint32_t sourceID;
		uint32_t bufferID;
		size_t bufferIndex;

		bool is_3D;
		bool is_Stereo;
		bool is_Loop;
	};
};

#endif