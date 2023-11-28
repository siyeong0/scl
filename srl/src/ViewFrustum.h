#pragma once
#include "MathLib.h"

namespace srl
{
	class ViewFrustum
	{
	public:
		ViewFrustum();
		ViewFrustum(float fovAngleY, float aspectRatio, float nearZ, float farZ);
		~ViewFrustum();

		void Initialize(float fovAngleY, float aspectRatio, float nearZ, float farZ);

		void Update(const Matrix& transform);
		bool Intersects(const BoundingBox& box) const;

	private:
		BoundingFrustum mBaseFrustum;
		BoundingFrustum mViewFrustum;
	};
}
