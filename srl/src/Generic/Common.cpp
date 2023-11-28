#include "Common.h"
#include <sstream>

namespace srl
{
	std::string FRAGMENT_CACHE_PATH = std::string("../cache/fragment/");
	std::string BLOCK_CACHE_PATH = std::string("../cache/block/");
	std::string CHUNK_CACHE_PATH = std::string("../cache/chunk/");

	std::string centerToString(const Vector3& center)
	{
		std::ostringstream ss;
		ss.precision(2);
		ss << center[0] << ","
			<< center[1] << ","
			<< center[2];
		return ss.str();
	}

	void centerFromIdx(Vector3* outCenter, size_t indices[], float numInSide, float size)
	{
		(*outCenter)[0] += (float(indices[0]) - numInSide * 0.5f + float(int(numInSide + 1) % 2) * 0.5f) * size;
		(*outCenter)[1] += (float(indices[1]) - numInSide * 0.5f + float(int(numInSide + 1) % 2) * 0.5f) * size;
		(*outCenter)[2] += (float(indices[2]) - numInSide * 0.5f + float(int(numInSide + 1) % 2) * 0.5f) * size;
	}

	void idxFromCenter(size_t outIndices[], const Vector3& center, float numInSide, float size)
	{
		outIndices[0] = size_t(center[0] / size + numInSide * 0.5f - float(int(numInSide + 1) % 2) * 0.5f);
		outIndices[1] = size_t(center[1] / size + numInSide * 0.5f - float(int(numInSide + 1) % 2) * 0.5f);
		outIndices[2] = size_t(center[2] / size + numInSide * 0.5f - float(int(numInSide + 1) % 2) * 0.5f);
	}
}