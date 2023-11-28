#include <iostream>
#include "Chunk.h"
#include <random>
#include <opencv2/opencv.hpp>
#include "MathLib.h"
#include "ScanRec.h"
#include <io.h>
#include "MemPool.hpp"

#include <Eigen/Dense>

using namespace srl;

void TestPcdGeneration();
void TestMemPool();

int main(void)
{
	TestPcdGeneration();
	// TestMemPool();
	return 0;
}

void TestPcdGeneration()
{
	std::string basePath = "../resources/2023-11-20-12-09-41/";
	ScanRec scanRec(256, 144, 8);
	Matrix prevExtrinsic;
	size_t count = 0;

	cv::VideoWriter videoWriter;
	videoWriter.open("ac.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 2, cv::Size(1280, 720), true);

	for (int i = 0; i < 1700; i += 1)
	{
		std::string imagePath = basePath + "image/" + std::to_string(i) + ".jpg";
		std::string depthPath = basePath + "depth/" + std::to_string(i) + ".png";
		std::string cameraPath = basePath + "camera/" + std::to_string(i) + ".bin";
		if (_access(imagePath.c_str(), 0) == -1 || _access(depthPath.c_str(), 0) == -1 || _access(cameraPath.c_str(), 0) == -1)
		{
			continue;
		}
		// read rgb
		cv::Mat img = cv::imread(imagePath);
		cv::imshow("test", img);
		cv::waitKey(1);
		cv::Mat rgb;
		cv::resize(img, rgb, cv::Size(256, 144));
		uchar* rawRgb = rgb.ptr();
		RGB* rgbData = new RGB[256 * 144];
		for (int h = 0; h < 144; h++)
		{
			for (int w = 0; w < 256; w++)
			{
				int idx = (h * 256 + w) * 3;
				rgbData[h * 256 + w].B = static_cast<uint8_t>(rawRgb[idx + 0]);
				rgbData[h * 256 + w].G = static_cast<uint8_t>(rawRgb[idx + 1]);
				rgbData[h * 256 + w].R = static_cast<uint8_t>(rawRgb[idx + 2]);
			}
		}
		// read depth
		auto depth = cv::imread(depthPath, 2);
		uchar* rawDepth = depth.ptr();
		uint16_t* depthData = new uint16_t[256 * 144];
		for (int h = 0; h < 144; h++)
		{
			for (int w = 0; w < 256; w++)
			{
				int idx = (h * 256 + w) * 2;
				depthData[h * 256 + w] = *reinterpret_cast<uint16_t*>(&rawDepth[idx]);
			}
		}
		// read camera info
		std::ifstream camFile;
		camFile.open(cameraPath, std::ios_base::in | std::ios_base::binary);
		float camIntrinsic[12] = { 0 };
		camFile.read(reinterpret_cast<char*>(camIntrinsic), 12 * sizeof(float));
		Matrix camExtrinsic;
		camFile.read(reinterpret_cast<char*>(&camExtrinsic), 16 * sizeof(float));

		// calculate distance between prev extrinsic and curr extrinsic
		Vector3 prevTranslation(prevExtrinsic.coeff(0, 3), prevExtrinsic.coeff(1, 3), prevExtrinsic.coeff(2, 3));
		Vector3 currTranslation(camExtrinsic.coeff(0, 3), camExtrinsic.coeff(1, 3), camExtrinsic.coeff(2, 3));
		Matrix3 prevRotMat;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				prevRotMat.coeffRef(i, j) = prevExtrinsic.coeffRef(i, j);
			}
		}
		Matrix3 currRotMat;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				currRotMat.coeffRef(i, j) = camExtrinsic.coeffRef(i, j);
			}
		}

		float transDist = (currTranslation - prevTranslation).norm();
		auto vv = (currRotMat * prevRotMat.transpose()).eulerAngles(0, 1, 2);
		auto euler = (currRotMat * prevRotMat.transpose()).eulerAngles(0, 1, 2);
		for (int i = 0; i < 3; i++)
		{
			if (fabs(euler[i]) > 3.14f)
			{
				euler[i] = euler[i] - (euler[i] > 0.f ? 1.57f : -1.57f);
			}
		}
		float rotDist = euler.norm();
		if (transDist < 0.6f && (rotDist < 0.5f || rotDist > 3.14))
		{
			continue;
		}
		videoWriter << img;
		//std::cout << "=======================" << std::endl;
		//std::cout << count << " : frame number" << i << std::endl;
		//std::cout << "T : " << transDist << std::endl;
		//std::cout << "R : " << rotDist << std::endl;
		//std::cout << "=======================" << std::endl;

		ScanRec::CameraInstrinsic camIn;
		camIn.FocalLength[0] = camIntrinsic[0] / 5.f;
		camIn.FocalLength[1] = camIntrinsic[5] / 5.f;
		camIn.Center[0] = camIntrinsic[8] / 5.f;
		camIn.Center[1] = camIntrinsic[9] / 5.f;
		scanRec.SetCameraIntrinsics(camIn);

		scanRec.Step(camExtrinsic, rgbData, depthData);

		prevExtrinsic = camExtrinsic;
		count++;
	}
	scanRec.Save("pcd.txt");
}

void TestMemPool()
{
	struct Foo
	{
		float x;
		float y;

		uint16_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
	};
	MemPool<sizeof(Foo)> mp;
	std::vector<Foo*> foos;
	for (size_t i = 0; i < 25092; i++)
	{
		foos.push_back(reinterpret_cast<Foo*>(mp.Alloc()));
	}
	
	auto getAllocCount = [&]() -> size_t
	{
		size_t allocCount = 0;
		for (auto& bucket : mp.mBuckets)
		{
			allocCount += bucket.Idx;
		}
		return allocCount;
	};

	mp.Free(foos[12]);
	auto t = mp.Alloc();
	mp.Free(foos[177]);
	mp.Free(foos[1311]);
	auto t1 = mp.Alloc();
	auto t2 = mp.Alloc();
	auto t3 = mp.Alloc();
	mp.Free(foos[11]);
	mp.Free(foos[23333]);
	mp.Free(foos[133]);
	mp.Free(foos[3131]);
	mp.Free(foos[4]);
	mp.Free(foos[1111]);
	auto t4 = mp.Alloc();
	auto t5 = mp.Alloc();
	auto t6 = mp.Alloc();
	auto t7 = mp.Alloc();
	auto t8 = mp.Alloc();
	auto t9 = mp.Alloc();

	return;
}