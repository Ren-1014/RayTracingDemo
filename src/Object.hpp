#pragma once
#include "Intersection.hpp"
#include "Bounds3.hpp"
#include "material.hpp"

// struct Object {
// 	Bounds3 b;
// 	Material* m;
// 	float area;
// 	Object(Material *m): m(m){}
// 	virtual Intersection intersect(const Ray& r) { return Intersection(); }
// 	virtual Bounds3 bounds() { return b; }
// 	virtual void sample(Intersection &pos, float &pdf){};
// 	virtual ~Object(){}
// };

class Object
{
public:
    Bounds3 b;
    Material* m;
    float area;

    Object(Material *m): m(m) {}
    virtual ~Object() {}

    virtual Intersection intersect(const Ray& r) { return Intersection(); };
    virtual Bounds3 bounds() { return b; }
    virtual void Sample(Intersection &pos, float &pdf) {};
};