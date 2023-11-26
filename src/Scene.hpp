#pragma once

#include <vector>
#include "Object.hpp"
#include "BVH.hpp"
#include "Material.hpp"
#include "Lights.hpp"
#include "Triangles.hpp"
#include "utils.hpp"

class Scene {
public:
	BVHAccel* bvh = nullptr;
	vector<Object*> objs;
	float RussianRoulette = 0.8;

	Scene() {}
	Intersection intersect(const Ray &r) { 
		return bvh->getIntersection(bvh->root,r); 
	}
	
	~Scene(){
		for(int i = 0; i < objs.size(); i++) delete objs[i];
		delete bvh;
	}

	void buildBVH();

	void initScene();

	Vec3d radiance(const Ray &r,Intersection inter = Intersection());
};

Vec3d Scene::radiance(const Ray &r,Intersection inter) {
	if (!inter.t) inter = intersect(r);
	if (!inter.t) return Vec3d();
	Material* m= inter.obj->m;
	Vec3d x=inter.x, n = inter.n, wo =(r.direction * -1).normalized();
	float pdf_light = 0.0f;
	Intersection inter_;
	Lights::sample_on_lights( inter_,pdf_light); // 光源采样
	Material* m_= inter_.obj->m;
	Vec3d x_ = inter_.x, ws = (x_-x).normalized(), n_= inter_.n, L;
	// if there is no occlusion between shading point and sample point,
	// calculate radiance caused by direction light 
	if((intersect(Ray(x,ws)).x - x_).norm() < 1e-3) 
		L = m_->e * m->eval(wo,ws,n) * ws.dot(n)*(ws*-1.0).dot(n_) / (((x_-x).norm() * (x_-x).norm()) * pdf_light); 
	// when calculate indrectional radiance, use RussianRoulette algorithm to decide whether to stop tracing
	if(uniform_rand() < Scene::RussianRoulette) { 
		Vec3d wi = m->sample(wo,n);
		Intersection i = intersect(Ray(x,wi));
		if(Lights::light_objs.count(i.obj) == 0) // 如果交点信息中的物体不是光源物体   
			L = L + radiance(Ray(x,wi),i) * m->eval(wi,wo,n)*wi.dot(n) / (m->pdf(wi,wo,n)*Scene::RussianRoulette);
	}
	// calculate randiance caused by refraction
	Vec3d refra_wi = m->refract(wo,n);
	if(refra_wi != Vec3d()){
		if(uniform_rand() < Scene::RussianRoulette) { 
			L = L + radiance(Ray(x,refra_wi)) / Scene::RussianRoulette;
		}
	} 
	return m->e + L;
}

void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objs);
}

void Scene::initScene()
{
	Material* red = new Diffuse(Vec3d(0.75,0.25,0.25));
	Material* green = new Diffuse(Vec3d(0.25, 0.75, 0.25));
	Material* white = new Diffuse(Vec3d(0.75, 0.75, 0.75));
	Material* yellow= new Diffuse(Vec3d(0.75f, 0.75f, 0.0f));
	Material* mirr= new Specular();
	Material* glass= new Refraction();
	Material* light = new Diffuse(Vec3d(), Vec3d(25,25,25));
	float ul = 50;
	Vec3d llu(-ul,ul,ul),lru(-ul,ul,-ul),lrd(-ul,-ul,-ul),lld(-ul,-ul,ul);
	Vec3d rlu(ul,ul,ul), rru(ul,ul,-ul), rrd(ul,-ul,-ul), rld(ul,-ul,ul);
	objs = {
		new Triangle(lru,llu,lrd, red), new Triangle(lld,lrd,llu, red),//left wall
		new Triangle(rru,rrd,rlu, green), new Triangle(rld,rlu,rrd, green),//right wall
		new Triangle(llu,lru,rlu, white), new Triangle(rru,rlu,lru, white),//top wall
		new Triangle(lld,rld,lrd, white), new Triangle(rrd,lrd,rld, white),//bottom wall
		new Triangle(lrd,rrd,lru, white), new Triangle(rru,lru,rrd, white),//back wall
		new MeshTriangle("./assets/bunny.obj",Vec3d(0,-60,0), 300.0, glass),  
	};
	llu.x *= 0.3, llu.z *= 0.3, rlu.x *= 0.3, rlu.z *= 0.3;
	lru.x *= 0.3, lru.z *= 0.3, rru.x *= 0.3, rru.z *= 0.3;
	objs.push_back(new Triangle(llu,lru,rlu, light)); Lights:: regist_lights(objs.back()); // area light
	objs.push_back(new Triangle(rru,rlu,lru, light)); Lights:: regist_lights(objs.back());
	cout << "size: " << objs.size() << endl;
	buildBVH();
	cout << "BVH done! " << endl;
}