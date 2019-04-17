#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "vector3.hpp"

struct uv_coordinates
{
    uv_coordinates() {}
    uv_coordinates(float mu, float mv) : u(mu), v(mv) {}

    float u, v;
};

struct vertex
{
    vertex() {}
    vertex(const vector3f& v, const uv_coordinates& u) : pos(v), uv(u), occlusion(0u) {}
    vertex(const vector3f& v, const uv_coordinates& u, uchar o) : pos(v), uv(u), occlusion(o) {}

    vector3f        pos;
    uv_coordinates uv;
    uchar          occlusion;
    uchar          sunlight;
    uchar          light;
};

#endif
