#include "vector3.hpp"
#include <iostream>

#define ISQRT_2 0.7071067811865475244008443621048
#define ISQRT_3 0.5773502691896257645091487805019

const vector3f vector3f::ZERO     = vector3f(0.0f,    0.0f,    0.0f);
const vector3f vector3f::UNIT     = vector3f(1.0f,    1.0f,    1.0f);
const vector3f vector3f::UNIT_X   = vector3f(1.0f,    0.0f,    0.0f);
const vector3f vector3f::UNIT_Y   = vector3f(0.0f,    1.0f,    0.0f);
const vector3f vector3f::UNIT_Z   = vector3f(0.0f,    0.0f,    1.0f);
const vector3f vector3f::UNIT_XY  = vector3f(ISQRT_2, ISQRT_2, 0.0f);
const vector3f vector3f::UNIT_XZ  = vector3f(ISQRT_2, 0.0f,    ISQRT_2);
const vector3f vector3f::UNIT_YZ  = vector3f(0.0f,    ISQRT_2, ISQRT_2);
const vector3f vector3f::UNIT_XYZ = vector3f(ISQRT_3, ISQRT_3, ISQRT_3);

const vector3i vector3i::ZERO     = vector3i(0, 0, 0);
const vector3i vector3i::UNIT     = vector3i(1, 1, 1);
const vector3i vector3i::UNIT_X   = vector3i(1, 0, 0);
const vector3i vector3i::UNIT_Y   = vector3i(0, 1, 0);
const vector3i vector3i::UNIT_Z   = vector3i(0, 0, 1);
const vector3i vector3i::UNIT_XY  = vector3i(1, 1, 0);
const vector3i vector3i::UNIT_XZ  = vector3i(1, 0, 1);
const vector3i vector3i::UNIT_YZ  = vector3i(0, 1, 1);

void vector3f::rotate(float fAngle, const vector3f& mAxis)
{
    // Code extracted from Ogre for optimum performance
    float fHalfAngle = 0.5f*fAngle;
    float fSin = sin(fHalfAngle);
    float w = cos(fHalfAngle);
    vector3f mNAxis = fSin*mAxis;

    vector3f uv, uuv;
    uv = mNAxis ^ (*this);
    uuv = mNAxis ^ uv;
    uv *= 2.0f*w;
    uuv *= 2.0f;

    (*this) += uv + uuv;
}

std::ostream& operator << (std::ostream& s, const vector3f& v)
{
    return s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

std::ostream& operator << (std::ostream& s, const vector3i& v)
{
    return s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
