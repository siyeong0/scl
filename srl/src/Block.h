#pragma once
#include "Common.h"
#include "Fragment.h"

namespace srl
{
	class Block
	{
	public:
		Block();
		~Block();

		void AddPoint(const Vector3& center, PointData& data, uint8_t label);
		Fragment** GetFrags();

	private:
		Fragment** mFrags;
	};
}