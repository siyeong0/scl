#pragma once
#include <vector>
#include "Chunk.h"
#include "ViewFrustum.h"

namespace srl
{
	class ScanRec
	{
	public:
		struct CameraInstrinsic
		{
			Vector2 FocalLength;
			Vector2 Center;
		};
	public:
		ScanRec(size_t width, size_t height, float farDepth);
		~ScanRec();

		void SetCameraIntrinsics(const CameraInstrinsic& camIntrinsic);
		const std::vector<RosPointData>& Step(Matrix& camExtrinsic, RGB* rgb, uint16_t* depth);

		void Save(std::string filename);
		void Load(std::string filename);
	private:
		CameraInstrinsic mCamIntrnsic;
		struct ChunkData
		{
			Chunk* Chunk;
			Vector3 Center;
			size_t RecentStep;
		};
		std::vector<ChunkData> mChunkDatas;
		size_t mSize;

		size_t mStepCount;

		size_t mWidth;
		size_t mHeight;
		float mFarDepth;

		Vector2* mUVBuffer;

		ViewFrustum mScannerFrustum;

		std::vector<RosPointData> mRosPcdBuf;
	};
}