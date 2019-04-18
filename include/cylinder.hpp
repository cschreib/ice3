#ifndef CYLINDER_H
#define CYLINDER_H

#include <lxgui/utils.hpp>
#include "vector3.hpp"

class axis_aligned_box;

class cylinder
{
public :

    cylinder();
    cylinder(float fRadius, float fHeight);
    cylinder(const vector3f& mPosition, float fRadius, float fHeight);
    ~cylinder();

    axis_aligned_box make_bounding_box() const;

    cylinder operator + (const vector3f& mAdd) const;
    cylinder operator - (const vector3f& mAdd) const;

    void operator += (const vector3f& mAdd);
    void operator -= (const vector3f& mAdd);

    vector3f pos;
    float    radius = 0.0;
    float    height = 0.0;
};

#endif
