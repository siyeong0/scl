#include "Fragment.h"
#include <memory>
#include <fstream>
#include "Common.h"
#include "MemPool.hpp"

namespace srl
{
	static MemPool<POINTS_PER_FRAG* BYTES_PER_POINT, 65535> gPcdPool;

	Fragment::Fragment()
		: mPcd(nullptr)
	{
		mPcd = gPcdPool.Alloc();
		// Initialize to empty val
		float* fPcd = reinterpret_cast<float*>(mPcd);
		std::fill(fPcd, fPcd + POINTS_PER_FRAG * 3, PCD_EMPTY_VAL);
	}

	Fragment::~Fragment()
	{
		if (mPcd == nullptr)
		{
			return;
		}
		gPcdPool.Free(mPcd);
		mPcd = nullptr;

		mLabelCountList.Free();
	}

	bool Fragment::AddPoint(PointData& data, uint8_t label)
	{
		float* fPcd = GetPointPtr(mPcd);
		uint8_t* uPcd = GetColorPtr(mPcd);
		// Find empty memory
		for (int i = 0; i < POINTS_PER_FRAG; i++)
		{
			float* pointPtr = &fPcd[i * 3];
			uint8_t* colPtr = &uPcd[i * 3];

			if (pointPtr[0] == PCD_EMPTY_VAL)
			{
				memcpy(pointPtr, &data, sizeof(float) * 3);
				memcpy(colPtr, &(data.Color), sizeof(uint8_t) * 3);
				addLabel(label, i);

				if (i == POINTS_PER_FRAG - 1)
				{
					// Compute normals of points
				}

				return true;
			}
			else
			{
				Vector3 currPoint(pointPtr);
				Vector3 inputPoint(reinterpret_cast<float*>(&data));
				float dist = (currPoint - inputPoint).norm();
				if (dist < PCD_MIN_DIST)
				{
					// Blend colors
					colPtr[0] = uint8_t(colPtr[0] * 0.7f + data.Color.R * 0.3f);
					colPtr[1] = uint8_t(colPtr[1] * 0.7f + data.Color.G * 0.3f);
					colPtr[2] = uint8_t(colPtr[2] * 0.7f + data.Color.B * 0.3f);

					return false;
				}
			}
		}
		// Full
		return false;
	}

	uint8_t Fragment::GetLabel()
	{
		uint32_t maxCount = 0;
		uint8_t maxLabel = 0;

		size_t idx = 0;
		while (mLabelCountList[idx] != nullptr)
		{
			uint8_t label = mLabelCountList[idx]->Data.Label;
			uint32_t countBitMask = mLabelCountList[idx]->Data.Count;
			uint32_t count;
			for (count = 0; countBitMask != 0; count++)
			{
				countBitMask &= (countBitMask - 1);
			}
			if (count >= maxCount)
			{
				maxCount = count;
				maxLabel = label;
			}
			idx++;
		}

		return maxLabel;
	}

	const LinkedList<Fragment::LabelCount>& Fragment::GetLabelCountList() const
	{
		return mLabelCountList;
	}

	void* Fragment::GetPcd() const
	{
		return mPcd;
	}

	void Fragment::addLabel(uint8_t label, size_t pointIdx)
	{
		size_t idx = 0;
		while (mLabelCountList[idx] != nullptr)
		{
			if (mLabelCountList[idx]->Data.Label == label)
			{
				mLabelCountList[idx]->Data.Count |= 1 << pointIdx;
				return;
			}
			idx++;
		}
		LabelCount lc;
		lc.Label = label;
		lc.Count = 1 << pointIdx;
		mLabelCountList.Append(lc);
	}

	float* Fragment::GetPointPtr(void* pcdPtr)
	{
		return reinterpret_cast<float*>(pcdPtr);
	}

	int8_t* Fragment::GetNormalPtr(void* pcdPtr)
	{
		return reinterpret_cast<int8_t*>(pcdPtr) + NORMAL_OFFSET_IN_BYTE;
	}

	uint8_t* Fragment::GetColorPtr(void* pcdPtr)
	{
		return reinterpret_cast<uint8_t*>(pcdPtr) + COLOR_OFFSET_IN_BYTE;
	}
}