#include "ScanRec.h"
#include "MemPool.hpp"
#include <opencv2/opencv.hpp>
#include <io.h>

namespace srl
{
	ScanRec::ScanRec(size_t width, size_t height, float farDepth)
		: mStepCount(0)
		, mWidth(width)
		, mHeight(height)
		, mFarDepth(farDepth)
		, mChunkDatas()
		, mSize(0)
		, mUVBuffer(nullptr)
	{
		ChunkData baseChunkData;
		baseChunkData.Chunk = (Chunk*)Alloc(sizeof(Chunk));
		new (baseChunkData.Chunk) Chunk();
		baseChunkData.Center = Vector3(0, 0, 0);
		baseChunkData.RecentStep = mStepCount;
		mChunkDatas.push_back(baseChunkData);

		mUVBuffer = (Vector2*)Alloc(sizeof(Vector2) * width * height);

		mRosPcdBuf.reserve(mWidth * mHeight);
	}

	ScanRec::~ScanRec()
	{
		std::cout << "Number of Chunks : " << mChunkDatas.size() << std::endl;
		for (auto& chunkData : mChunkDatas)
		{
			if (chunkData.Chunk == nullptr)
			{
				continue;
			}
			chunkData.Chunk->~Chunk();
			Free(chunkData.Chunk);
			chunkData.Chunk = nullptr;
		}
		Free(mUVBuffer);
		mUVBuffer = nullptr;
	}

	void ScanRec::SetCameraIntrinsics(const CameraInstrinsic& camIntrinsic)
	{
		mCamIntrnsic = camIntrinsic;

		float cx = mCamIntrnsic.Center[0];
		float cy = mCamIntrnsic.Center[1];
		float fx = mCamIntrnsic.FocalLength[0];
		float fy = mCamIntrnsic.FocalLength[1];
		float invFx = 1.f / fx;
		float invFy = 1.f / fy;

		// Create view frustum
		float aspectRatio = float(mWidth) / mHeight;
		float fovY = 2.f * atanf(mHeight / 2.f / fy); // fovY / 2 == tan(h/2 / fy)
		mScannerFrustum.Initialize(fovY, aspectRatio, 0.01f, mFarDepth);

		// Fill UV buffer
		// world coord x = (w - cx) * depth / focalLength
		for (size_t h = 0; h < mHeight; h++)
		{
			for (size_t w = 0; w < mWidth; w++)
			{
				Vector2& uv = mUVBuffer[h * mWidth + w];
				uv[0] = (w - cx) * invFx;
				uv[1] = (h - cy) * invFy;
			}
		}
	}

	const std::vector<RosPointData>& ScanRec::Step(Matrix& camExtrinsic, RGB* rgb, uint16_t* depth)
	{
		mRosPcdBuf.clear();
		Assert(mRosPcdBuf.size() == 0);

		Vector3 currPosition(camExtrinsic.coeff(0, 3), camExtrinsic.coeff(1, 3), camExtrinsic.coeff(2, 3));
		// Set scanner position and orientation from camera extrinsic
		mScannerFrustum.Update(camExtrinsic);

		// Chunk managing
		std::vector<size_t> visibleChunkIdxs;
		for (size_t i = 0; i < mChunkDatas.size(); i++)
		{
			ChunkData& chunkData = mChunkDatas[i];
			Chunk* chunk = chunkData.Chunk;
			// const Vector3& center = chunkData.Center;
			Vector3 center = chunkData.Center;

			size_t stepOffset = mStepCount - chunkData.RecentStep;
			float dist = (Chunk::GetCenter(currPosition) - center).norm();
			if (int(stepOffset) > 4 && dist > mFarDepth * 2.0f)
			{
				if (chunk != nullptr)
				{
					std::ofstream out;
					std::string path = CHUNK_CACHE_PATH;
					out.open(path + centerToString(center), std::ios_base::out);
					Assert(out.is_open());

					Chunk::Write(chunk, out);
					(chunk)->~Chunk();
					chunkData.Chunk = nullptr;

					out.close();
				}
			}

			if (mScannerFrustum.Intersects(Chunk::GetBoundingBox(center)))
			{
				if (chunkData.Chunk == nullptr)
				{
					std::ifstream in;
					std::string path = CHUNK_CACHE_PATH;
					in.open(path + centerToString(center), std::ios_base::in);
					Assert(in.is_open());

					chunkData.Chunk = (Chunk*)Alloc(sizeof(Chunk));
					new (chunkData.Chunk) Chunk();
					Chunk::Read(chunkData.Chunk, center, in);

					in.close();
				}
				chunkData.RecentStep = mStepCount;

				visibleChunkIdxs.push_back(i);
			}
		}

		//for (int i = 0; i < 7; i++)
		//{
		//	for (int j = 0; j < 7; j++)
		//	{
		//		bool bf = false;
		//		for (auto& idx : visibleChunkIdxs)
		//		{
		//			auto& chunkData = mChunkDatas[idx];
		//			Vector3 center = chunkData.Center;
		//			size_t x = size_t(roundf(center[0] / CHUNK_SIZE));
		//			size_t y = size_t(roundf(center[1] / CHUNK_SIZE));
		//			size_t z = size_t(roundf(center[2] / CHUNK_SIZE));

		//			if (x + 3 == i && z + 3 == j)
		//			{
		//				bf = true;
		//				break;
		//			}
		//		}
		//		if (bf)
		//		{
		//			std::cout << "#";
		//		}
		//		else
		//		{
		//			std::cout << " ";
		//		}
		//	}
		//	std::cout << std::endl;
		//}

		//std::cout << mChunkDatas.size() << " " << visibleChunkIdxs.size() << std::endl;

		// Generate point cloud data
		float depthScale = 1.f / ((powf(2.f, 16.f) - 1) / mFarDepth);
		for (size_t h = 0; h < mHeight; h++)
		{
			for (size_t w = 0; w < mWidth; w++)
			{
				if (depth[h * mWidth + w] == 0)
				{
					continue;
				}
				PointData data;
				RosPointData rosData;
				const Vector3& pos = data.Position;
				// point
				Vector4 hgPoint;
				float z = float(depth[h * mWidth + w]) * depthScale;
				hgPoint[0] = mUVBuffer[h * mWidth + w][0] * z;
				hgPoint[1] = -mUVBuffer[h * mWidth + w][1] * z;
				hgPoint[2] = -z;
				hgPoint[3] = 1.f;
				hgPoint = camExtrinsic * hgPoint;
				memcpy(&data, &hgPoint, sizeof(float) * 3);
				memcpy(&rosData, &hgPoint, sizeof(float) * 3);
				// color
				memcpy(&(data.Color), &rgb[h * mWidth + w], sizeof(uint8_t) * 3);
				memcpy(&(rosData.r), &rgb[h * mWidth + w], sizeof(uint8_t) * 3);

				// output ros point cloud data
				mRosPcdBuf.push_back(rosData);

				// add point to local memory
				bool bChunkExist = false;
				for (auto& chunkIdx : visibleChunkIdxs)
				{
					ChunkData& chunkData = mChunkDatas[chunkIdx];
					Chunk* chunk = chunkData.Chunk;
					const Vector3& center = chunkData.Center;
					if (Chunk::Include(center, Vector3(pos[0], pos[1], pos[2])))
					{
						chunk->AddPoint(center, data, 3);
						bChunkExist = true;
						break;
					}
				}
				if (!bChunkExist)
				{
					Vector3 chunkCenter = Chunk::GetCenter(pos);

					// Debug
#ifdef DEBUG
					for (auto& chunkData : mChunkDatas)
					{
						Assert(chunkData.Center != chunkCenter);
					}
#endif

					ChunkData newChunkData;
					newChunkData.Chunk = (Chunk*)Alloc(sizeof(Chunk));
					new (newChunkData.Chunk) Chunk();
					newChunkData.Center = chunkCenter;
					newChunkData.Chunk->AddPoint(chunkCenter, data, 3);
					newChunkData.RecentStep = mStepCount;
					mChunkDatas.push_back(newChunkData);

					visibleChunkIdxs.push_back(mChunkDatas.size() - 1);
				}
			}
		}
		mStepCount++;

		return mRosPcdBuf;
	}

	void ScanRec::Save(std::string filename)
	{
		std::ofstream out;
		out.open(std::string("../") + filename, std::ios_base::out);
		Assert(out.is_open());

		for (auto& chunkData : mChunkDatas)
		{
			if (chunkData.Chunk == nullptr)
			{
				std::ifstream in;
				std::string path = CHUNK_CACHE_PATH;
				in.open(path + centerToString(chunkData.Center), std::ios_base::in);
				Assert(in.is_open());

				chunkData.Chunk = (Chunk*)Alloc(sizeof(Chunk));
				new (chunkData.Chunk) Chunk();
				Chunk::Read(chunkData.Chunk, chunkData.Center, in);

				in.close();
			}
			const Chunk* chunk = chunkData.Chunk;
			const Vector3& center = chunkData.Center;
			Chunk::Write(chunk, out);
		}

		out.close();
	}

	void ScanRec::Load(std::string filename)
	{
		std::ifstream in;
		in.open(std::string("../../") + filename, std::ios_base::out);
		Assert(in.is_open());
	}
}