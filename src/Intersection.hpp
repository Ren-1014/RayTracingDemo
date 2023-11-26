#pragma once
#include "utils.hpp"

class Object;

struct Intersection {
	double  t = 0.0f;
	Vec3d x, n;
	Object* obj = nullptr;
};