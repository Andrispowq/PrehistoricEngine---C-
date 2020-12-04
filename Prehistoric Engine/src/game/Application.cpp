#pragma once

#include "engine/prehistoric/core/util/Includes.hpp"
#include "engine/CoreEngine.h"

#include "engine/prehistoric/resources/Factory.h"

struct SomeStruct
{
	SomeStruct() : index(0), pData(nullptr), prev(nullptr), next(nullptr) {}
	~SomeStruct() {}

	void* operator new(size_t size, Factory<SomeStruct>& factory)
	{
		void* ptr = factory.allocate();

		if (!ptr)
			throw std::exception_ptr();

		return ptr;
	}

	void operator delete(void* ptr, Factory<SomeStruct>& factory)
	{
		SomeStruct* _ptr = (SomeStruct*)ptr;
		_ptr->~SomeStruct();
		factory.deallocate(_ptr);
	}

	void foo()
	{
		std::cout << "hello there\n";
	}

private:
	size_t index;
	void* pData;
	SomeStruct* prev;
	SomeStruct* next;
};

namespace PrehistoricEngine
{
	void StartEngine()
	{
		{
			Factory<SomeStruct> factory(1024);

			SomeStruct* ptr = new(factory) SomeStruct();
			ptr->foo();
			SomeStruct* ptr2 = new(factory) SomeStruct();
			SomeStruct::operator delete(ptr, factory);
			SomeStruct::operator delete(ptr2, factory);
		}

		CoreEngine engine;
		engine.Start();
	}
};