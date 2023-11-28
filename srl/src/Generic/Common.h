#pragma once
#include <string>
#include <fstream>
#include <cassert>

#include "MathLib.h"

namespace srl
{
	static constexpr float cxprPow3(float val)
	{
		return val * val * val;
	}

	static constexpr float cxprFloor(float val)
	{
		return float(int(val));
	}

	extern std::string FRAGMENT_CACHE_PATH;
	extern std::string BLOCK_CACHE_PATH;
	extern std::string CHUNK_CACHE_PATH;

	constexpr float FRAGMENT_SIZE = 0.05f;
	constexpr float BLOCK_SIZE = 1.0f;
	constexpr float CHUNK_SIZE = 8.0f;
	constexpr float NUM_FRAGS_IN_SIDE = BLOCK_SIZE / FRAGMENT_SIZE;
	constexpr float NUM_BLOCKS_IN_SIDE = CHUNK_SIZE / BLOCK_SIZE;
	constexpr size_t NUM_FRAGS_IN_BLOCK = size_t(cxprPow3(cxprFloor(NUM_FRAGS_IN_SIDE)));;
	constexpr size_t NUM_BLOCKS_IN_CHUNK = size_t(cxprPow3(cxprFloor(NUM_BLOCKS_IN_SIDE)));;
	constexpr float HALF_FRAG_SIZE = FRAGMENT_SIZE / 2.0f;
	constexpr float HALF_BLOCK_SIZE = BLOCK_SIZE / 2.0f;
	constexpr float HALF_CHUNK_SIZE = CHUNK_SIZE / 2.0f;

	struct RGB
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	};
	static_assert(sizeof(RGB) == 3);

	struct Normal
	{
		int8_t x;
		int8_t y;
		int8_t z;
	};
	static_assert(sizeof(Normal) == 3);

	struct PointData
	{
		Vector3 Position;
		Normal Normal;
		RGB Color;
	};
	static_assert(sizeof(PointData) == 20);

	struct RosPointData
	{
		float x;
		float y;
		float z;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t dummy;
	};
	static_assert(sizeof(RosPointData) == 16);

	inline void _Assert(bool e)
	{
		if (!e)
		{
			_CrtDbgBreak();
		}
	}

# ifdef DEBUG
# define Assert(E) _Assert(E)
# else
# define Assert(E) __assume(E)
#endif

	std::string centerToString(const Vector3& center);
	void centerFromIdx(Vector3* outCenter, size_t indices[], float numInSide, float size);
	void idxFromCenter(size_t outIndices[], const Vector3& center, float numInSide, float size);
}