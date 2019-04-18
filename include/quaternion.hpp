#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include <lxgui/utils.hpp>
#include "vector3.hpp"

class quaternion
{
public :

    quaternion();
    quaternion(const vector3f& mAxis, float fAngle);
    quaternion(float fW, float fX, float fY, float fZ);
    ~quaternion();

    void       invert();
    quaternion get_inverse() const;

    float get_yaw() const;
    float get_pitch() const;
    float get_roll() const;

    quaternion operator + (const quaternion& q) const;
    quaternion operator - (const quaternion& q) const;
    quaternion operator * (const quaternion& q) const;
    quaternion operator * (float f) const;
    quaternion operator - () const;

    vector3f operator * (const vector3f& v) const;

    bool operator == (const quaternion& q) const;
    bool operator != (const quaternion& q) const;

    float w, x, y, z;

    static const quaternion UNIT;
};

quaternion operator * (float f, const quaternion& q);


#endif
