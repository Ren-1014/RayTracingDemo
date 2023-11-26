#include "OBJ_Loader.hpp"
#include "utils.hpp"
#include "material.hpp"
#include <set>
#include <ctime>
using namespace std;
#include "Lights.hpp"
#include "Triangles.hpp"
#include "Scene.hpp"

int main(int argc, char *argv[]) {
	Scene scene;
	scene.initScene();
	int w= 1024, h= 700, samps = 4;
	Ray cam(Vec3d(0,0,250), Vec3d(0,0.0,-1).normalized());
	Vec3d cx=Vec3d(w*.5135/h), cy=(cx%cam.direction).normalized()*.5135, *c=new Vec3d[w*h];
	thread_pool tpool; // thread pool
	
	time_t start, stop;
    time(&start);
	cout << "begin!" << endl;
	#if 1
	for (int y=0; y<h; y++) {
		for (unsigned short x=0; x<w; x++) {
			auto task =  [=,&scene](){
				for (int sy=0, i=(h-y-1)*w+x; sy<2; sy++){
					Vec3d r;
					for (int sx=0; sx<2; sx++) {
						for (int s=0; s<samps; s++) {
							double r1=2*uniform_rand(), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
							double r2=2*uniform_rand(), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);
							Vec3d d = cx*( ( (sx+.5 + dx)/2 + x)/w - .5) + cy*( ( (sy+.5 + dy)/2 + y)/h - .5) + cam.direction;
							r = r + scene.radiance(Ray(cam.origin+d*140,d.normalized())) * (1./samps);
						}
						c[i] = c[i] + Vec3d(clamp(r.x),clamp(r.y),clamp(r.z))*.25;
					}
				}
			};
			tpool.submit(task);
		}
	}
	cout << "test1: " << tpool.uncompleted_task_size() << endl;
	cout << "test2: " << thread::hardware_concurrency() << endl;
	while(int s = tpool.uncompleted_task_size()) {	// 将当前线程挂起（休眠）500毫秒。这是为了避免在循环中频繁检查任务状态，从而减轻 CPU 的负担
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		fprintf(stderr,"\rRendering (%d spp) %5.2f%%",samps*4,100 - 100.* s/(w*h));
	}
	#endif

	#if 0	// 单线程 3min27s
	for (int y = 0; y < h; ++y) {
        for (unsigned short x = 0; x < w; ++x) {
            for (int sy=0, i=(h-y-1)*w+x; sy<2; sy++){
				Vec3d r;
				for (int sx=0; sx<2; sx++) {
					for (int s=0; s<samps; s++) {
						double r1=2*uniform_rand(), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
						double r2=2*uniform_rand(), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);
						Vec3d d = cx*( ( (sx+.5 + dx)/2 + x)/w - .5) + cy*( ( (sy+.5 + dy)/2 + y)/h - .5) + cam.direction;
						r = r + scene.radiance(Ray(cam.origin+d*140,d.normalized())) * (1./samps);
					}
					c[i] = c[i] + Vec3d(clamp(r.x),clamp(r.y),clamp(r.z))*.25;
				}
			}
        }
    }
	#endif
	FILE *f = fopen("./outputs/bunny.ppm", "w");
	fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
	for (int i=0; i<w*h; i++)
	    fprintf(f,"%d %d %d ", toInt(c[i].x), toInt(c[i].y), toInt(c[i].z));

	fprintf(stderr,"\rRendering (%d spp) %5.2f%%",samps*4,100.);
	time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rRay Tracing  complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}
