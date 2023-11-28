#include "Block.h"
#include <cmath>
#include <string>
#include "Common.h"
#include "MemPool.hpp"

namespace srl
{
	static MemPool<NUM_FRAGS_IN_BLOCK * sizeof(Fragment*)> gFragPtrArrPool;
	static MemPool<sizeof(Fragment), 65535> gFragPool;

	Block::Block()
		: mFrags(nullptr)
	{
		mFrags = (Fragment**)gFragPtrArrPool.Alloc();
		memset(mFrags, 0, sizeof(Fragment*) * NUM_FRAGS_IN_BLOCK);
	}
	Block::~Block()
	{
		for (size_t i = 0; i < NUM_FRAGS_IN_BLOCK; i++)
		{
			Fragment** frag = &mFrags[i];
			if (*frag != nullptr)
			{
				(*frag)->~Fragment();
				gFragPool.Free(*frag);
				*frag = nullptr;
			}
		}
		gFragPtrArrPool.Free(mFrags);
	}

	void Block::AddPoint(const Vector3& center, PointData& data, uint8_t label)
	{
		float cx = center[0];
		float cy = center[1];
		float cz = center[2];
		float x = data.Position[0];
		float y = data.Position[1];
		float z = data.Position[2];
		Assert(fabs(cx - x) <= HALF_BLOCK_SIZE + 1e-4f && fabs(cy - y) <= HALF_BLOCK_SIZE + 1e-4f && fabs(cz - z) <= HALF_BLOCK_SIZE + 1e-4f);

		size_t idxX = size_t((x - cx + HALF_BLOCK_SIZE) / FRAGMENT_SIZE);
		size_t idxY = size_t((y - cy + HALF_BLOCK_SIZE) / FRAGMENT_SIZE);
		size_t idxZ = size_t((z - cz + HALF_BLOCK_SIZE) / FRAGMENT_SIZE);

		size_t size = size_t(NUM_FRAGS_IN_SIDE);
		Fragment** frag = &mFrags[idxX * size * size + idxY * size + idxZ];
		if (*frag == nullptr)
		{
			*frag = (Fragment*)gFragPool.Alloc();
			new (*frag) Fragment();
		}
		(*frag)->AddPoint(data, label);
	}

	Fragment** Block::GetFrags()
	{
		return mFrags;
	}
}