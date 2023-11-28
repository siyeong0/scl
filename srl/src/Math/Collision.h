#pragma once
#include <Eigen/Dense>
#include "Vector.h"
#include "Matrix.h"
#include "Plane.h"

namespace srl
{
	struct BoundingBox
	{
		Vector3 Center;            // Center of the box.
		Vector3 Extents;           // Distance from the center to each side.

		BoundingBox() : Center(0, 0, 0), Extents(1.f, 1.f, 1.f) {}
		BoundingBox(const Vector3& center, const Vector3& extents) : Center(center), Extents(extents) {}

		BoundingBox(const BoundingBox&) = default;
		BoundingBox& operator=(const BoundingBox&) = default;
		BoundingBox(BoundingBox&&) = default;
		BoundingBox& operator=(BoundingBox&&) = default;
	};

	struct BoundingFrustum
	{
		static const size_t CORNER_COUNT = 8;

		Vector3 Origin;            // Origin of the frustum (and projection)
		Matrix Transform;		   // Transform matrix.

		float RightSlope;           // Positive X (X/Z)
		float LeftSlope;            // Negative X
		float TopSlope;             // Positive Y (Y/Z)
		float BottomSlope;          // Negative Y
		float Near, Far;            // Z of the near plane and far plane

		BoundingFrustum() :
			Origin(0, 0, 0), Transform(Matrix::Identity()), RightSlope(1.f), LeftSlope(-1.f),
			TopSlope(1.f), BottomSlope(-1.f), Near(0), Far(1.f) {}
		BoundingFrustum(const Matrix& projection) { CreateFromProjection(projection); }

		BoundingFrustum(const BoundingFrustum&) = default;
		BoundingFrustum& operator=(const BoundingFrustum&) = default;
		BoundingFrustum(BoundingFrustum&&) = default;
		BoundingFrustum& operator=(BoundingFrustum&&) = default;

		void CreateFromProjection(const Matrix& projection)
		{
			// Corners of the projection frustum in homogenous space
			static Vector4 homogenousPoints[6] =
			{
				Vector4{  1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
				Vector4{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
				Vector4{  0.0f,  1.0f, 1.0f, 1.0f },   // top
				Vector4{  0.0f, -1.0f, 1.0f, 1.0f },   // bottom

				Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
				Vector4{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
			};

			Matrix matInverse = projection.inverse();

			// Compute the frustum corners in world space
			Vector4 points[6];

			for (size_t i = 0; i < 6; ++i)
			{
				// Transform point.
				points[i] = matInverse * homogenousPoints[i];
			}

			Origin = Vector3(0.0f, 0.0f, 0.0f);
			Transform = Matrix::Identity();

			// Compute the slopes
			points[0] = points[0].cwiseProduct(Vector4(1.f / points[0][2], 1.f / points[0][2], 1.f / points[0][2], 1.f / points[0][2]));
			points[1] = points[1].cwiseProduct(Vector4(1.f / points[1][2], 1.f / points[1][2], 1.f / points[1][2], 1.f / points[1][2]));
			points[2] = points[2].cwiseProduct(Vector4(1.f / points[2][2], 1.f / points[2][2], 1.f / points[2][2], 1.f / points[2][2]));
			points[3] = points[3].cwiseProduct(Vector4(1.f / points[3][2], 1.f / points[3][2], 1.f / points[3][2], 1.f / points[3][2]));

			LeftSlope = points[0][0];
			RightSlope = points[1][0];
			BottomSlope = points[2][1];
			TopSlope = points[3][1];

			// Compute near and far.
			points[4] = points[4].cwiseProduct(Vector4(1.f / points[4][3], 1.f / points[4][3], 1.f / points[4][3], 1.f / points[4][3]));
			points[5] = points[5].cwiseProduct(Vector4(1.f / points[5][3], 1.f / points[5][3], 1.f / points[5][3], 1.f / points[5][3]));

			Near = points[4][2];
			Far = points[5][2];
		}

		bool Intersects(const BoundingBox& box) const
		{
			float absNear = fabs(Near);
			float absFar = fabs(Far);

			float nearLeft = LeftSlope * absNear;
			float nearRight = RightSlope * absNear;
			float nearTop = TopSlope * absNear;
			float nearBottom = BottomSlope * absNear;
			float farLeft = LeftSlope * absFar;
			float farRight = RightSlope * absFar;
			float farTop = TopSlope * absFar;
			float farBottom = BottomSlope * absFar;

			Vector4 points4[8] =
			{
				Vector4{nearLeft, nearTop, Near, 1.0f},
				Vector4{nearRight, nearTop, Near, 1.0f},
				Vector4{nearLeft, nearBottom, Near, 1.0f},
				Vector4{nearRight, nearBottom, Near, 1.0f},
				Vector4{farLeft, farTop, Far, 1.0f},
				Vector4{farRight, farTop, Far, 1.0f},
				Vector4{farLeft, farBottom, Far, 1.0f},
				Vector4{farRight, farBottom, Far, 1.0f}
			};

			Vector3 points[8];
			for (int i = 0; i < 8; i++)
			{
				Vector4 tp = Transform * points4[i];
				points[i][0] = tp[0];
				points[i][1] = tp[1];
				points[i][2] = tp[2];
			}

			auto createPlane = [](const Vector3& p0, const Vector3& p1, const Vector3& p2) -> Vector4
			{
				Vector3 v1 = p1 - p0;
				Vector3 v2 = p2 - p0;

				Vector3 normal = v1.cross(v2).normalized();
				float dist = normal.dot(p0);
				return Vector4(normal[0], normal[1], normal[2], dist);
			};
			Vector4 planes[6] =
			{
				createPlane(points[0], points[2], points[1]),
				createPlane(points[4], points[5], points[6]),
				createPlane(points[1], points[3], points[5]),
				createPlane(points[0], points[4], points[2]),
				createPlane(points[0], points[1], points[4]),
				createPlane(points[2], points[6], points[3]),
			};

			Vector3 boxMin = box.Center - box.Extents;
			Vector3 boxMax = box.Center + box.Extents;

			size_t outCount;
			bool bBoxInFrus = true;
			bool bFrusInBox = true;
			// Check box outside/inside of frustum
			for (size_t i = 0; i < 6; ++i)
			{
				outCount = 0;
				outCount += size_t(planes[i].dot(Vector4(boxMin[0], boxMin[1], boxMin[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMax[0], boxMin[1], boxMin[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMin[0], boxMax[1], boxMin[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMax[0], boxMax[1], boxMin[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMin[0], boxMin[1], boxMax[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMax[0], boxMin[1], boxMax[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMin[0], boxMax[1], boxMax[2], 1.0f)) < 0.0f);
				outCount += size_t(planes[i].dot(Vector4(boxMax[0], boxMax[1], boxMax[2], 1.0f)) < 0.0f);
				bBoxInFrus &= outCount != 8;
			}

			// Check frustum outside/inside box
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount += size_t(points[i][0] > boxMax[0]);
			}
			bFrusInBox &= outCount != 8;
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount += size_t(points[i][0] < boxMin[0]);
			}
			bFrusInBox &= outCount != 8;
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount += size_t(points[i][1] > boxMax[1]);
			}
			bFrusInBox &= outCount != 8;
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount == size_t(points[i][1] < boxMin[1]);
			}
			bFrusInBox &= outCount != 8;
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount += size_t(points[i][2] > boxMax[2]);
			}
			bFrusInBox &= outCount != 8;
			outCount = 0;
			for (int i = 0; i < 8; i++)
			{
				outCount += size_t(points[i][2] < boxMin[2]);
			}
			bFrusInBox &= outCount != 8;

			return bBoxInFrus || bFrusInBox;
		}
	};
}