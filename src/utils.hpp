#pragma once
#define _USE_MATH_DEFINES
#include <random>
#include <iostream>
#include <algorithm>
#include <math.h>   
#include <vector> 
#include <array>
#include <cfloat>
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <functional>
using namespace std;
std:: default_random_engine generator;
std::uniform_real_distribution<double> distr(0.0,1.0);
double uniform_rand() { return distr(generator); }
inline float clamp(float lo, float hi, float v) { return max(lo, min(hi, v)); }
inline double clamp(double x) { return x<0 ? 0 : x>1 ? 1 : x; }
inline int toInt(double x) { return int(pow(clamp(x),1/2.2)*255+.5); }

class Vec3d
{
public:
    double x = 0, y = 0, z = 0;
	Vec3d(double x_=0, double y_=0, double z_=0) { x=x_; y=y_; z=z_; }
    // Vec3d(): x(0), y(0), z(0) {}
    // Vec3d(double xx): x(xx), y(xx), z(xx){}
    // Vec3d(double xx, double yy, double zz): x(xx), y(yy), z(zz){}
    Vec3d operator * (const double &r) const {return Vec3d(x * r, y * r, z * r);}
    Vec3d operator / (const double &r) const {return Vec3d(x / r, y / r, z / r);}

    double norm() {return std::sqrt(x * x + y * y + z * z);}
    Vec3d normalized() {
        double n = std::sqrt(x * x + y * y + z * z);
        return Vec3d(x / n, y / n, z / n);
    }

    double operator[] (int idx) const { return idx == 0 ? x : idx == 1 ? y : z; }
    Vec3d operator * (const Vec3d &v) const { return Vec3d(x * v.x, y * v.y, z * v.z); }
    Vec3d operator - (const Vec3d &v) const { return Vec3d(x - v.x, y - v.y, z - v.z); }
    Vec3d operator + (const Vec3d &v) const { return Vec3d(x + v.x, y + v.y, z + v.z); }
    Vec3d operator - () const { return Vec3d(-x, -y, -z); }
    Vec3d& operator += (const Vec3d &v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vec3d operator % (const Vec3d& b) const { return Vec3d(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    double dot(const Vec3d& b) const { return x * b.x + y * b.y + z * b.z; }

    bool operator ==(const Vec3d &b) const { return x == b.x && y == b.y && z == b.z; }
    bool operator !=(const Vec3d &b) const { return !(*this == b); }

    friend Vec3d operator * (const double &r, const Vec3d &v) { return Vec3d(v.x * r, v.y * r, v.z * r); }
    friend std::ostream & operator << (std::ostream &os, const Vec3d &v) { return os << v.x << ", " << v.y << ", " << v.z; }

    static Vec3d Min(const Vec3d &p1, const Vec3d &p2) {
        return Vec3d(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                       std::min(p1.z, p2.z));
    }

    static Vec3d Max(const Vec3d &p1, const Vec3d &p2) {
        return Vec3d(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                       std::max(p1.z, p2.z));
    }

    int max_dim() const { return x > y && x > z ? x : y > z ? y : z; }
};

template<typename T>
class threadsafe_queue {
private:
	mutable std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
public:
	threadsafe_queue() {}
	void push(T new_value) {
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(std::move(new_value));
		data_cond.notify_one();
	}
	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk,[this]{return !data_queue.empty();});
		value=std::move(data_queue.front());
		data_queue.pop();
	}
	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk,[this]{return !data_queue.empty();});
		std::shared_ptr<T> res( std::make_shared<T>(std::move(data_queue.front())));
		data_queue.pop();
		return res;
	}
	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(mut);
		if(data_queue.empty())
			return false;
		value=std::move(data_queue.front());
		data_queue.pop();
	return true;
	}
	std::shared_ptr<T> try_pop() {
		std::lock_guard<std::mutex> lk(mut);
		if(data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res(
		std::make_shared<T>(std::(data_queue.front())));
		data_queue.pop();
		return res;
	}

	int size() const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.size();
	}
};

class join_threads {
	std::vector<std::thread>& threads;
public:
	explicit join_threads(std::vector<std::thread>& threads_): threads(threads_) {}	// 防止隐式类型转换
	~join_threads() {
		for(unsigned long i=0;i<threads.size();++i) {
			if(threads[i].joinable())
				threads[i].join();
		}
	}
};

class thread_pool {
	std::atomic_bool done;
	threadsafe_queue<std::function<void()> > work_queue;
	std::vector<std::thread> threads;
	join_threads joiner;
	void worker_thread() {
		while(!done) {
			std::function<void()> task;
			if(work_queue.try_pop(task)) {
				task();
			} else {
				std::this_thread::yield();
			}
		}
	}
	public:
	thread_pool(): done(false),joiner(threads) {
		unsigned const thread_count=std::thread::hardware_concurrency();
		try {
			for(unsigned i=0;i<thread_count;++i) {
				threads.push_back( std::thread(&thread_pool::worker_thread,this));
			}
		} catch(...) {
			done=true;
			throw;
		}
	}
	~thread_pool() {
		done=true;
	}
	template<typename FunctionType>
	void submit(FunctionType f) {
		work_queue.push(std::function<void()>(f));
	}
	int uncompleted_task_size(){
		return work_queue.size();
	}
};