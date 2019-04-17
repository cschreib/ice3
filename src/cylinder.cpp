#include "cylinder.hpp"
#include "axisalignedbox.hpp"

cylinder::cylinder()
{
}

cylinder::cylinder(float fRadius, float fHeight) :
    radius(fRadius), height(fHeight)
{

}

cylinder::cylinder(const vector3f& mPosition, float fRadius, float fHeight) :
    pos(mPosition), radius(fRadius), height(fHeight)
{

}

cylinder::~cylinder()
{
}

axis_aligned_box cylinder::make_bounding_box() const
{
    vector3f mSize(radius, height/2.0f, radius);
    return axis_aligned_box(pos - mSize, pos + mSize);
}

cylinder cylinder::operator + (const vector3f& mAdd) const
{
    return cylinder(pos + mAdd, radius, height);
}

cylinder cylinder::operator - (const vector3f& mAdd) const
{
    return cylinder(pos - mAdd, radius, height);
}

void cylinder::operator += (const vector3f& mAdd)
{
    pos += mAdd;
}

void cylinder::operator -= (const vector3f& mAdd)
{
    pos -= mAdd;
}
