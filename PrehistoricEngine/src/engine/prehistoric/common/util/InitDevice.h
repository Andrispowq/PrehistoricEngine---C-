#ifndef INIT_DEVICE_H
#define INIT_DEVICE_H

#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	class InitDevice
	{
	public:
		static InitDevice& getInstance();
		static void DeleteInstance();

		virtual void InitiateDevice() const = 0;
	protected:
		InitDevice() {}
		virtual ~InitDevice() {}
	private:
		static InitDevice* instance;
	};
};

#endif