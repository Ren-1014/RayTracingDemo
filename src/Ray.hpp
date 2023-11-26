#pragma once
#include "utils.hpp"

struct Ray
{
    Vec3d origin;
    Vec3d direction;
    Vec3d direction_inv;
    
    Ray(Vec3d o_, Vec3d d_): origin(o_), direction(d_) { direction_inv = Vec3d{1/d_.x, 1/d_.y, 1/d_.z}; }
};