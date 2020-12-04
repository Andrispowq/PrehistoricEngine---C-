#ifndef FACTORY_H
#define FACTORY_H

#include <memory>
#include <stdint.h>

template<typename T>
class Factory
{
public:
	Factory(size_t max_allocations)
		: max_allocations(max_allocations)
	{
		_ASSERT(sizeof(uint32_t) == 4); //We're relying on the fact that a uint32_t is 4 bytes/32 bits to mark used bits, so it's necessary we check it

		num_of_uint32_ts = ((max_allocations / 32) % 1) ? uint32_t(max_allocations / 32) + 1 : max_allocations / 32;

		used_bits = new uint32_t[num_of_uint32_ts];
		array = new T[max_allocations];

		memset(used_bits, 0, num_of_uint32_ts * 4); //A uint32_t is always 4 bytes
	}

	Factory(const Factory&& factory)
	{
		max_allocations = factory.max_allocations;
		num_of_uint32_ts = factory.num_of_uint32_ts;

		used_bits = factory.used_bits;
		array = factory.array;

		first_free = factory.first_free;

		factory.used_bits = nullptr;
		factory.array = nullptr;
	}

	~Factory()
	{
		delete[] used_bits;
		delete[] array;
	}

	//Allocates a pointer
	T* allocate()
	{
		size_t index = find_next_free_index();

		if (index == max_allocations)
			return nullptr;

		//Mark the used block as free
		size_t j = index % 32;
		size_t i = size_t(index / 32);
		used_bits[i] |= 0x1 << j;

		return &array[index];
	}

	//Deallocates a pointer
	void deallocate(T* ptr)
	{
		size_t index = index_of(ptr);

		//We have to set the first_free variable to the newly freed block
		first_free = index;

		//Mark the used block as free
		size_t j = index % 32;
		size_t i = size_t(index / 32);
		used_bits[i] &= ~(0x1 << j);
	}

	Factory& operator=(Factory&& factory)
	{
		max_allocations = factory.max_allocations;
		num_of_uint32_ts = factory.num_of_uint32_ts;

		used_bits = factory.used_bits;
		array = factory.array;

		first_free = factory.first_free;

		factory.used_bits = nullptr;
		factory.array = nullptr;

		return *this;
	}

	//Factories can't be copied because they're huge structures, only moved via std::move()
	Factory(const Factory&) = delete;
	Factory operator=(const Factory&) = delete;

private:
	//This function returns the next free address. If it returns max_allocations that means there is no space left
	size_t find_next_free_index()
	{
		if (first_free == max_allocations)
			return max_allocations;

		size_t ret = first_free;

		//check the next bit
		size_t j = (first_free + 1) % 32;
		size_t i = size_t((first_free + 1) / 32);

		if ((used_bits[i] >> j) & 0x1)
		{
			//if the next bit isn't available, search till we find one
			for (; i < num_of_uint32_ts; i++)
			{
				uint32_t& bitset = used_bits[i];

				//Every bit is set -> no free space
				if (bitset & 0xFFFFFFFF)
					continue;

				for (j = 0; j < 32; j++)
				{
					if (!((bitset >> j) & 0x1))
					{
						first_free = i * 32 + j;
						return ret;
					}
				}
			}
		}
		else
		{
			++first_free;
			return ret;
		}

		return max_allocations;
	}

	//Gets the index of an allocated pointer, and returns it's address
	size_t index_of(T* ptr)
	{
		size_t diff = (size_t)ptr - (size_t)array;
		return diff / sizeof(T);
	}

private:
	//The limits, given at runtime
	size_t max_allocations;
	size_t num_of_uint32_ts;

	//to speed up allocation when a large portion is allocated
	size_t first_free = 0;

	//Allocated on the heap as it can be a huge allocation and could blow up the stack
	//1 -> used, 0 -> free
	uint32_t* used_bits;
	T* array;
};

#endif