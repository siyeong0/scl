#pragma once
#include "Block.h"

namespace srl
{
	class Chunk
	{
	public:
		Chunk();
		~Chunk();

		void AddPoint(const Vector3& center, PointData& data, uint8_t label);

		Block** GetBlocks() const;

	public:
		static Vector3 GetCenter(const Vector3& pos);
		static bool Include(const Vector3& center, const Vector3& point);
		static BoundingBox GetBoundingBox(const Vector3& center);
		static std::ofstream& Write(const Chunk* chunk, std::ofstream& out);
		static std::ifstream& Read(Chunk* chunk, const Vector3 center, std::ifstream& in);

	private:
		Block** mBlocks;
	};
}