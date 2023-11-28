#include "Chunk.h"
#include "MemPool.hpp"
#include <sstream>

namespace srl
{
	static MemPool<NUM_BLOCKS_IN_CHUNK * sizeof(Block*), 2048> gBlockPtrArrPool;
	static MemPool<sizeof(Block), 65535> gBlockPool;

	Chunk::Chunk()
		: mBlocks(nullptr)
	{
		mBlocks = (Block**)gBlockPtrArrPool.Alloc();
		memset(mBlocks, 0, sizeof(Block*) * NUM_BLOCKS_IN_CHUNK);
	}

	Chunk::~Chunk()
	{
		for (size_t i = 0; i < NUM_BLOCKS_IN_CHUNK; i++)
		{
			Block** block = &mBlocks[i];
			if (*block != nullptr)
			{
				(*block)->~Block();
				gBlockPool.Free(*block);
				*block = nullptr;
			}
		}
		gBlockPtrArrPool.Free(mBlocks);
	}

	void Chunk::AddPoint(const Vector3& center, PointData& data, uint8_t label)
	{
		float cx = center[0];
		float cy = center[1];
		float cz = center[2];
		float x = data.Position[0];
		float y = data.Position[1];
		float z = data.Position[2];
		Assert(fabs(cx - x) <= HALF_CHUNK_SIZE + 1e-4f && fabs(cy - y) <= HALF_CHUNK_SIZE + 1e-4f && fabs(cz - z) <= HALF_CHUNK_SIZE + 1e-4f);

		size_t idxX = size_t((x - cx + HALF_CHUNK_SIZE) / BLOCK_SIZE);
		size_t idxY = size_t((y - cy + HALF_CHUNK_SIZE) / BLOCK_SIZE);
		size_t idxZ = size_t((z - cz + HALF_CHUNK_SIZE) / BLOCK_SIZE);

		Vector3 blockCenter;
		memcpy(&blockCenter, &center, sizeof(float) * 3);
		size_t indices[3] = { idxX, idxY, idxZ };
		centerFromIdx(&blockCenter, indices, NUM_BLOCKS_IN_SIDE, BLOCK_SIZE);

		size_t size = size_t(NUM_BLOCKS_IN_SIDE);
		Block** block = &mBlocks[idxX * size * size + idxY * size + idxZ];
		if (*block == nullptr)
		{
			*block = (Block*)gBlockPool.Alloc();
			new (*block) Block();
		}
		(*block)->AddPoint(blockCenter, data, label);
	}

	Block** Chunk::GetBlocks() const
	{
		return mBlocks;
	}

	Vector3 Chunk::GetCenter(const Vector3& pos)
	{
		return Vector3(roundf(pos[0] / CHUNK_SIZE) * CHUNK_SIZE,
			roundf(pos[1] / CHUNK_SIZE) * CHUNK_SIZE,
			roundf(pos[2] / CHUNK_SIZE) * CHUNK_SIZE);
	}

	bool Chunk::Include(const Vector3& center, const Vector3& point)
	{
		float dx = fabs(center[0] - point[0]);
		float dy = fabs(center[1] - point[1]);
		float dz = fabs(center[2] - point[2]);

		return (dx <= HALF_CHUNK_SIZE && dy <= HALF_CHUNK_SIZE && dz <= HALF_CHUNK_SIZE);
	}

	BoundingBox Chunk::GetBoundingBox(const Vector3& center)
	{
		return BoundingBox(center, Vector3(HALF_CHUNK_SIZE, HALF_CHUNK_SIZE, HALF_CHUNK_SIZE));
	}

	std::ofstream& Chunk::Write(const Chunk* chunk, std::ofstream& out)
	{
		for (size_t bIdx = 0; bIdx < NUM_BLOCKS_IN_CHUNK; bIdx++)
		{
			Block* currBlock = chunk->GetBlocks()[bIdx];
			if (currBlock == nullptr)
			{
				continue;
			}
			for (size_t fIdx = 0; fIdx < NUM_FRAGS_IN_BLOCK; fIdx++)
			{
				Fragment* currFrag = currBlock->GetFrags()[fIdx];
				if (currFrag == nullptr)
				{
					continue;
				}
				float* points = Fragment::GetPointPtr(currFrag->GetPcd());
				int8_t* normals = Fragment::GetNormalPtr(currFrag->GetPcd());
				uint8_t* colors = Fragment::GetColorPtr(currFrag->GetPcd());
				for (size_t pIdx = 0; pIdx < POINTS_PER_FRAG; pIdx++)
				{
					PointData pointData;
					memcpy(&pointData.Position, &points[pIdx * 3], sizeof(float) * 3);
					if (pointData.Position[0] == PCD_EMPTY_VAL)
					{
						break;
					}
					memcpy(&pointData.Normal, &normals[pIdx * 3], sizeof(int8_t) * 3);
					memcpy(&pointData.Color, &colors[pIdx * 3], sizeof(uint8_t) * 3);

					out.precision(4);
					out << pointData.Position[0] << ", "
						<< pointData.Position[1] << ", "
						<< pointData.Position[2] << ", "
						<< pointData.Normal.x / 127.f << ", "
						<< pointData.Normal.y / 127.f << ", "
						<< pointData.Normal.z / 127.f << ", "
						<< (int)pointData.Color.R << ", "
						<< (int)pointData.Color.G << ", "
						<< (int)pointData.Color.B << "\n";

					out.flush();

				}
			}
		}
		return out;
	}

	std::ifstream& Chunk::Read(Chunk* chunk, const Vector3 center, std::ifstream& in)
	{
		PointData pointData;
		std::string lineBuf;
		auto getFloat = [](std::stringstream& ss) -> float
			{
				std::string strBuf;
				std::getline(ss, strBuf, ',');
				return std::stof(strBuf);
			};

		while (std::getline(in, lineBuf))
		{
			std::stringstream ss(lineBuf);
			pointData.Position[0] = getFloat(ss);
			pointData.Position[1] = getFloat(ss);
			pointData.Position[2] = getFloat(ss);
			pointData.Normal.x = int8_t(getFloat(ss) * 127.f);
			pointData.Normal.y = int8_t(getFloat(ss) * 127.f);
			pointData.Normal.z = int8_t(getFloat(ss) * 127.f);
			pointData.Color.R = uint8_t(getFloat(ss));
			pointData.Color.G = uint8_t(getFloat(ss));
			pointData.Color.B = uint8_t(getFloat(ss));
			chunk->AddPoint(center, pointData, 3);
		}

		return in;
	}
}