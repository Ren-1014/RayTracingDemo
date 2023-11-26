#pragma once
#include <set>
#include "Object.hpp"

namespace Lights{
	set<Object*> light_objs;
	void regist_lights(Object* obj){ light_objs.insert(obj);}
	void unregist_lights(Object* obj){ light_objs.erase(obj); }
	void sample_on_lights(Intersection &pos, float &pdf){ 
		float emit_area_sum = 0;
		for (Object* obj : light_objs){
				emit_area_sum += obj->area;
		}
		float p = uniform_rand() * emit_area_sum;
		emit_area_sum = 0;
		for (Object* obj : light_objs){
			emit_area_sum += obj->area;
			if (p <= emit_area_sum){
				obj->Sample(pos, pdf);
				break;
			}
		}
	}
};

#if 0
class Light
{
public:
	Light(){}
};
#endif
