#pragma once
#include <cstdint>
#include <iostream>
#include <numeric>
#include "Common.h"
#include "LinkedList.hpp"

namespace srl
{
	static_assert(sizeof(float) == 4);
	static_assert(sizeof(int8_t) == 1);

	enum
	{
		POINTS_PER_FRAG = 24,
		NORMAL_OFFSET_IN_BYTE = POINTS_PER_FRAG * sizeof(float) * 3,
		COLOR_OFFSET_IN_BYTE = NORMAL_OFFSET_IN_BYTE + POINTS_PER_FRAG * sizeof(int8_t) * 3,
		BYTES_PER_POINT = 18
	};
	const float PCD_EMPTY_VAL = 9e+20f;
	const float PCD_MIN_DIST = FRAGMENT_SIZE / powf(POINTS_PER_FRAG, 1.f / 3.f) * 0.7f;

	class Fragment
	{
	public:
		struct LabelCount
		{
			uint32_t Label : 8;
			uint32_t Count : 24;
		};
	public:
		Fragment();
		~Fragment();

		bool AddPoint(PointData& data, uint8_t label);

		uint8_t GetLabel();
		const LinkedList<LabelCount>& GetLabelCountList() const;
		void* GetPcd() const;

	public:
		static float* GetPointPtr(void* pcdPtr);
		static int8_t* GetNormalPtr(void* pcdPtr);
		static uint8_t* GetColorPtr(void* pcdPtr);

	private:
		void addLabel(uint8_t label, size_t pointIdx);

	private:
		LinkedList<LabelCount> mLabelCountList;
		void* mPcd;
	};
}