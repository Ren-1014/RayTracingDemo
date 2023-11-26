#pragma once
#include "Bounds3.hpp"
#include "Object.hpp"


struct BVHBuildNode;

class BVHAccel
{
public:
    BVHBuildNode* root;

    BVHAccel(vector<Object*> objs); // root = build(objs)
    ~BVHAccel();

    BVHBuildNode* build(vector<Object*> objs);
    Intersection getIntersection(BVHBuildNode* node, const Ray& ray) const;
    void destruct(BVHBuildNode* u);
};



struct BVHBuildNode {
	Bounds3 bounds;
	BVHBuildNode *left = nullptr;
	BVHBuildNode *right = nullptr;
	Object* object = nullptr;
};

BVHAccel::BVHAccel(vector<Object*> objs) {
    root = build(objs);
}

BVHAccel::~BVHAccel() 
{
	destruct(root); 
}


BVHBuildNode* BVHAccel::build(vector<Object*>objs) {
	BVHBuildNode* node = new BVHBuildNode();
	Bounds3 bounds;
	auto left = objs.begin(), middle = left + (objs.size() / 2), right = objs.end();
	for (int i = 0; i < objs.size(); ++i) {
        bounds = bounds.Union(objs[i]->bounds());
    }
	if (objs.size() == 1) {
		node->bounds = objs[0]->bounds();
		node->object = objs[0];
		node->left = nullptr;
		node->right = nullptr;
		return node;
	} else if (objs.size() == 2) {
		node->left = build({ objs[0] });
		node->right = build({ objs[1] });
		node->bounds = node->left->bounds.Union(node->right->bounds);
		return node;
	} else {
		Bounds3 centroidBounds;
		for (int i = 0; i < objs.size(); ++i) {
            centroidBounds = centroidBounds.Union(objs[i]->bounds().Centroid());
        }
		int dim = (centroidBounds.pMax - centroidBounds.pMin).max_dim();
		switch (dim)
        {
        case 0:
            std::sort(left, right, [](auto f1, auto f2) {
                return f1->bounds().Centroid().x < f2->bounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(left, right, [](auto f1, auto f2) {
                return f1->bounds().Centroid().y < f2->bounds().Centroid().y;
            });
            break;
        default:
            std::sort(left, right, [](auto f1, auto f2) {
                return f1->bounds().Centroid().z < f2->bounds().Centroid().z;
            });
            break;
        }
		auto leftshapes= std::vector<Object*>(left, middle);
		auto rightshapes= std::vector<Object*>(middle, right);
		node->left = build(leftshapes);
		node->right = build(rightshapes);
		node->bounds = node->left->bounds.Union(node->right->bounds);
	}
	return node;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const {
    Intersection isect;
    if (!node->bounds.IntersectP(ray)) return isect;
    if (!node->left && !node->right) return node->object->intersect(ray);
    Intersection hit1 = getIntersection(node->left, ray);
    Intersection hit2 = getIntersection(node->right, ray);
    if (hit1.t && hit2.t) {
        return hit1.t < hit2.t ? hit1 : hit2;
    } else if(hit1.t) {
        return hit1;
    } else {
        return hit2;
    }
    return isect;
}

void BVHAccel::destruct(BVHBuildNode* u){
	if(u == nullptr) return ;
	destruct(u->left);
	destruct(u->right);
	delete u;
}
