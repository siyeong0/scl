#pragma once
#include <cstdlib>

namespace srl
{
	inline void* Alloc(size_t size)
	{
		return malloc(size);
	}

	inline void Free(void* ptr)
	{
		free(ptr);
	}
}