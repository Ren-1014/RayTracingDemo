#pragma once
#include "utils.hpp"
#define M_PI 3.1415926535

class Material{
public:
    Vec3d e; 

	Material(Vec3d e = Vec3d(0,0,0)) : e(e) {}
	
	virtual float pdf(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) {return 1;}
	virtual Vec3d sample(const Vec3d &wo, const Vec3d &n)  {return Vec3d();}
	virtual Vec3d eval(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) {return Vec3d();}
	virtual Vec3d refract(const Vec3d &wi, const Vec3d &n) { return Vec3d(); }

	Vec3d toWorld(const Vec3d &a, const Vec3d &n){
       Vec3d B, C;
       if (fabs(n.x) > fabs(n.y)){
           float invLen = 1.0f / sqrt(n.x * n.x + n.z * n.z);
           C =Vec3d(n.z * invLen, 0.0f, -n.x *invLen);
       } else {
           float invLen = 1.0f / std::sqrt(n.y * n.y + n.z * n.z);
           C =Vec3d(0.0f, n.z * invLen, -n.y *invLen);
       }
       B =C % n;
       return B * a.x   + C * a.y  + n * a.z ;
    }

	Vec3d reflect(const Vec3d &wi, const Vec3d &n) const { return wi * -1 + n* wi.dot(n) * 2; }

	Vec3d calc_refract(const Vec3d &wi, const Vec3d &n, float ior) const {
        float cosi = clamp(-1, 1, wi.dot(n)), etai = 1, etat = ior;
        Vec3d n_ = n, i = wi * -1;
		if(cosi < 0 ){ swap(etai,etat); n_ = n * -1; i = i* -1;}
        float eta = etai / etat, k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : i * eta  + n_ * (eta * cosi - sqrtf(k));
    }

	float fresnel(const Vec3d &wi, const Vec3d &n, const float &ior) const {
        float kr, cosi = clamp(-1, 1,wi.dot(n)), etai = 1, etat = ior;
        if (cosi < 0) {  std::swap(etai, etat); }
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        if (sint >= 1) kr = 1;
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            float Rp = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            kr = (Rs * Rs + Rp * Rp) / 2; // 折射比例
        }
		return kr;
    }
};

class Diffuse :public Material{
public:
	Vec3d Kd;

	Diffuse(Vec3d Kd, Vec3d e = Vec3d(0,0,0)) :Kd(Kd), Material(e){}

	float pdf(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) override { return 0.5f / M_PI; }

	Vec3d sample(const Vec3d &wo, const Vec3d &n) override {
		float x_1 = uniform_rand(), x_2 = uniform_rand();
		float z = std::fabs(1.0f - 2.0f * x_1);
		float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
		Vec3d localRay(r*std::cos(phi), r*std::sin(phi), z);
		return toWorld(localRay, n);
	}

	Vec3d eval(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) override {
		if(wo.dot(n) < 0) return Vec3d();
		return Kd / M_PI;
	}
};

class Specular :public  Material {
public:
	Vec3d Ks;

	Specular(Vec3d Ks = Vec3d(1,1,1)) : Ks(Ks), Material() {}

	float pdf(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) override { return 1; }

	Vec3d sample(const Vec3d &wo, const Vec3d &n) override { 
		return reflect(wo,n).normalized(); 
	}

	Vec3d eval(const Vec3d &wi, const Vec3d &wo, const Vec3d &n) override {
		if(wo.dot(n) < 0) return Vec3d();
		if((reflect(wi,n).normalized()- wo).norm() < 1e-6){ return Ks; } 
		return Vec3d();
	}
};

class Refraction :public Material {
public:
	float ior;

	Vec3d Ks;

	Refraction(float ior = 1.5,Vec3d Ks = Vec3d(1,1,1)) : Ks(Ks),ior(ior),Material(){}

	Vec3d refract(const Vec3d &wi, const Vec3d &n) override { return  calc_refract(wi,n,ior); }
};
