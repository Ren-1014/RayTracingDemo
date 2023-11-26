#pragma once
#include "Ray.hpp"

class Bounds3
{
public:
	Vec3d pMin = { DBL_MAX,DBL_MAX,DBL_MAX } ;
	Vec3d pMax = { DBL_MIN,DBL_MIN,DBL_MIN } ;
	// Bounds3() {}
	Bounds3() = default;
	Bounds3(Vec3d p) : pMin(p), pMax(p) { }
	Bounds3(Vec3d p1, Vec3d p2) { pMin = Vec3d::Min(p1,p2); pMax = Vec3d::Max(p1,p2); }

	Bounds3 Union(const Bounds3& b2) {
		Bounds3 ret;
		ret.pMin = Vec3d::Min(this->pMin, b2.pMin);
		ret.pMax = Vec3d::Max(this->pMax, b2.pMax);
		return ret;
	}
	
	Vec3d Centroid() { return 0.5 * pMin + 0.5 * pMax; }

    bool IntersectP(const Ray& ray) {
        const Vec3d& ori = ray.origin, invDir = ray.direction_inv;
        bool dirIsNeg[3] = { ray.direction.x >= 0, ray.direction.y >= 0, ray.direction.z >= 0 } ;
		float tEnter = FLT_MIN, tExit = FLT_MAX;
        for (int i = 0; i < 3; i++) {
			float min = (pMin[i] - ori[i]) * invDir[i];
			float max = (pMax[i] - ori[i]) * invDir[i];
			if (!dirIsNeg[i])  swap(min, max);
			tEnter = std::max(min, tEnter);
			tExit = std::min(max, tExit);
		}
		return tEnter <= tExit&& tExit >= 0;
    }
};