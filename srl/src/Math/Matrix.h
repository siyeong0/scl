#pragma once
#include <Eigen/Dense>

namespace srl
{
	using Matrix = Eigen::Matrix4f;
	using Matrix3 = Eigen::Matrix3f;

	inline Matrix CreatePerspectiveFovRH(float fovY, float aspectRatio, float nearZ, float farZ)
	{
		float sinFov = sinf(0.5f * fovY);
		float cosFov = cosf(0.5f * fovY);
		float fRange = farZ / (nearZ - farZ);
		float height = cosFov / sinFov;

		Matrix mat = Matrix::Zero();
		mat.coeffRef(0, 0) = height / aspectRatio;
		mat.coeffRef(1, 1) = height;
		mat.coeffRef(2, 2) = fRange;
		mat.coeffRef(3, 2) = -1.0f;
		mat.coeffRef(2, 3) = fRange * nearZ;

		return mat;
	}

	inline Matrix CreatePerspectiveFovLH(float fovY, float aspectRatio, float nearZ, float farZ)
	{
		float sinFov = sinf(0.5f * fovY);
		float cosFov = cosf(0.5f * fovY);
		float fRange = farZ / (nearZ - farZ);
		float height = cosFov / sinFov;

		Matrix mat = Matrix::Zero();
		mat.coeffRef(0, 0) = height / aspectRatio;
		mat.coeffRef(1, 1) = height;
		mat.coeffRef(2, 2) = fRange;
		mat.coeffRef(2, 3) = fRange * nearZ;
		mat.coeffRef(3, 3) = 1.0f;

		return mat;
	}
}