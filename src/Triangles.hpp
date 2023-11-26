#pragma once

#include "Object.hpp"
#include "Bounds3.hpp"
#include "BVH.hpp"
#include "OBJ_Loader.hpp"

#if 1
class Triangle: public Object {
public:
    Vec3d v0, v1, v2, n;

    Triangle(Vec3d v0_, Vec3d v1_, Vec3d v2_,Material* m):
			v0(v0_), v1(v1_), v2(v2_), Object(m){
		Vec3d e2 = v2 - v0, e1 = v1-v0;
		n = (e1 % e2).normalized();
		b = Bounds3(v0, v1).Union(Bounds3(v2));
		area = (e2 % e1).norm() * 0.5;
		// cout << "0: " << b.pMax.x << endl;
	}

    ~Triangle() {}

    Intersection intersect(const Ray &r) override {
		Vec3d o = r.origin, d  = r.direction;
		Vec3d e1 = v1 - v0, e2 = v2 - v0;
		Vec3d s0 = o - v0, s1 = d % e2, s2 = s0 % e1;
		Vec3d s = Vec3d(s2.dot(e2), s1.dot(s0), s2.dot(r.direction)) / s1.dot(e1);
		double t = s.x, u = s.y, v = s.z;
		if (t >= 0 && u >= 0 && v >= 0 && (u + v) <= 1)  return {t , r.origin + r.direction * t , n, this};
		return Intersection();
	}
    // 这个函数是随机采样一个Intersection，并将概率密度函数pdf设为 1 / area
    void Sample(Intersection &pos, float &pdf) override {
		float x = sqrt(uniform_rand()), y = uniform_rand();
        pos.x= v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.n= this->n;
		pos.obj = this;
        pdf = 1.0f / area;
	}
};


class MeshTriangle: public Object {
public: 
    std::vector<Triangle> triangles;
    BVHAccel* bvh = nullptr;

    MeshTriangle(string filename,Vec3d trans, float scale, Material* m): Object(m) {
        objl::Loader loader;
        loader.LoadFile(filename);
        auto mesh = loader.LoadedMeshes[0];
		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			std::array<Vec3d, 3> face_vertices;
			for (int j = 0; j < 3; j++) {
				auto vert = Vec3d(mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z);
				face_vertices[j] =vert * scale + trans ;
			}
			triangles.emplace_back(face_vertices[0], face_vertices[1], face_vertices[2],m);
		}

		std::vector<Object*> objs;

		for (auto& tri : triangles) {
            objs.push_back(&tri);  
        }
		for (auto obj : objs) {
            b = b.Union(obj->bounds());
        }
		bvh = new BVHAccel(objs);
    }

    ~MeshTriangle() { delete bvh; }

    Intersection intersect(const Ray& r) { return bvh->getIntersection(bvh->root, r); }
};

#if 0
class Triangle :public Object {
public:
	Vec3d v0, v1, v2, n;

	Triangle(Vec3d v0_, Vec3d v1_, Vec3d v2_,Material* m):
			v0(v0_), v1(v1_), v2(v2_), Object(m){
		Vec3d e2 = v2 - v0, e1 = v1-v0;
		n = (e1 % e2).normalized();
		b = Bounds3(v0, v1).Union(Bounds3(v2));
		area = (e2 % e1).norm() * 0.5;
	}

	// ~Triangle(){}

	Intersection intersect(const Ray &r) override {
		Vec3d o = r.origin, d  = r.direction;
		Vec3d e1 = v1 - v0, e2 = v2 - v0;
		Vec3d s0 = o- v0, s1 = d%e2, s2 = s0%e1;
		Vec3d s = Vec3d(s2.dot(e2), s1.dot(s0), s2.dot(r.direction)) / s1.dot(e1);
		double t = s.x, u = s.y, v = s.z;
		if (t >=0 && u >= 0 && v >=0 && (u+v) <= 1)  return { t , r.origin+r.direction*t , n, this };
		return Intersection();
	}
	void Sample(Intersection &pos, float &pdf) override {
		float x = sqrt(uniform_rand()), y = uniform_rand();
        pos.x= v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.n= this->n;
		pos.obj = this;
        pdf = 1.0f / area;
	}
} ;

class MeshTriangle :public Object {
public:
	std::vector<Triangle> triangles;
	BVHAccel* bvh = nullptr;
	MeshTriangle(string filename,Vec3d trans, float scale, Material* m)
	: Object(m) {
		objl::Loader loader;
		loader.LoadFile(filename);
		auto mesh = loader.LoadedMeshes[0];
		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			std::array<Vec3d,3> face_vertices;
			for (int j = 0; j < 3; j++) {
				auto vert = Vec3d(mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z);
				face_vertices[j] =vert * scale + trans ;
			}
			triangles.emplace_back(face_vertices[0], face_vertices[1], face_vertices[2],m);
		}
		std::vector<Object*> objs;
		for (auto& tri : triangles) objs.push_back(&tri);  
		for (auto obj : objs) b = b.Union(obj->bounds());
		bvh = new BVHAccel(objs);
	}
	Intersection intersect(const Ray & r) { return bvh->getIntersection(bvh->root,r); }
	~MeshTriangle(){ delete bvh; }
};
#endif

#endif