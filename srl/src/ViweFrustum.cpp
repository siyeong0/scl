#include "ViewFrustum.h"

namespace srl
{
	ViewFrustum::ViewFrustum()
	{

	}

	ViewFrustum::ViewFrustum(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		Initialize(fovAngleY, aspectRatio, nearZ, farZ);
	}

	ViewFrustum::~ViewFrustum()
	{

	}

	void ViewFrustum::Initialize(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		mBaseFrustum = BoundingFrustum(CreatePerspectiveFovRH(fovAngleY, aspectRatio, nearZ, farZ));
	}

	void ViewFrustum::Update(const Matrix& transform)
	{
		mViewFrustum = mBaseFrustum;
		mViewFrustum.Origin = Vector3(transform.coeff(0, 3), transform.coeff(1, 3), transform.coeff(2, 3));
		mViewFrustum.Transform = transform;
	}

	bool ViewFrustum::Intersects(const BoundingBox& box) const
	{
		return mViewFrustum.Intersects(box);
	}
}