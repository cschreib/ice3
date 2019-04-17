#include "axisalignedbox.hpp"
#include <limits>
#include <iostream>

axis_aligned_box::axis_aligned_box() :
    min(-std::numeric_limits<float>::infinity()),
    max(+std::numeric_limits<float>::infinity())
{

}

axis_aligned_box::axis_aligned_box(const vector3f& mMin, const vector3f& mMax) :
    min(mMin), max(mMax)
{
}

axis_aligned_box::~axis_aligned_box()
{
}

vector3f axis_aligned_box::operator[] (size_t uiIndex) const
{
    if (uiIndex == 0)
        return min;
    else if (uiIndex == 1)
        return vector3f(max.x, min.y, min.z);
    else if (uiIndex == 2)
        return vector3f(max.x, min.y, max.z);
    else if (uiIndex == 3)
        return vector3f(min.x, min.y, max.z);
    else if (uiIndex == 4)
        return vector3f(min.x, max.y, max.z);
    else if (uiIndex == 5)
        return vector3f(min.x, max.y, min.z);
    else if (uiIndex == 6)
        return vector3f(max.x, max.y, min.z);
    else
        return max;
}

axis_aligned_box axis_aligned_box::operator + (const vector3f& mAdd) const
{
    return axis_aligned_box(min + mAdd, max + mAdd);
}

axis_aligned_box axis_aligned_box::operator - (const vector3f& mAdd) const
{
    return axis_aligned_box(min - mAdd, max - mAdd);
}

void axis_aligned_box::operator += (const vector3f& mAdd)
{
    min += mAdd;
    max += mAdd;
}

void axis_aligned_box::operator -= (const vector3f& mAdd)
{
    min -= mAdd;
    max -= mAdd;
}

bool axis_aligned_box::contains(const axis_aligned_box& mBox) const
{
    if (is_infinite() || mBox.is_infinite())
        return true;

    if (max.x < mBox.min.x)
        return false;
    if (max.y < mBox.min.y)
        return false;
    if (max.z < mBox.min.z)
        return false;

    if (min.x > mBox.max.x)
        return false;
    if (min.y > mBox.max.y)
        return false;
    if (min.z > mBox.max.z)
        return false;

    return true;
}

bool axis_aligned_box::contains(const vector3f& mPoint) const
{
    return (min.x <= mPoint.x && mPoint.x <= max.x) &&
           (min.y <= mPoint.y && mPoint.y <= max.y) &&
           (min.z <= mPoint.z && mPoint.z <= max.z);
}

void axis_aligned_box::include(const vector3f& mPoint)
{
    max.x = std::max(max.x, mPoint.x);
    max.y = std::max(max.y, mPoint.y);
    max.z = std::max(max.z, mPoint.z);

    min.x = std::min(min.x, mPoint.x);
    min.y = std::min(min.y, mPoint.y);
    min.z = std::min(min.z, mPoint.z);
}

bool axis_aligned_box::is_infinite() const
{
    return  min.x == -std::numeric_limits<float>::infinity() &&
            min.y == -std::numeric_limits<float>::infinity() &&
            min.z == -std::numeric_limits<float>::infinity() &&
            max.x == +std::numeric_limits<float>::infinity() &&
            max.y == +std::numeric_limits<float>::infinity() &&
            max.z == +std::numeric_limits<float>::infinity();
}

bool axis_aligned_box::find_ray_intersection(const ray& mRay, vector3f& mIntersection) const
{
    if (contains(mRay.mOrigin))
    {
        mIntersection = mRay.mOrigin;
        return true;
    }

    // Check collision with each face of the bounding box (the 3 closest)
    float fLowestT = +std::numeric_limits<float>::infinity();

    // X
    if (mRay.mOrigin.x <= min.x && mRay.mDirection.x > 0.0f)
    {
        float fT = (min.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }
    if (mRay.mOrigin.x >= max.x && mRay.mDirection.x < 0.0f)
    {
        float fT = (max.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }

    // Y
    if (mRay.mOrigin.y <= min.y && mRay.mDirection.y > 0.0f)
    {
        float fT = (min.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x<= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }
    if (mRay.mOrigin.y >= max.y && mRay.mDirection.y < 0.0f)
    {
        float fT = (max.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }

    // Z
    if (mRay.mOrigin.z <= min.z && mRay.mDirection.z > 0.0f)
    {
        float fT = (min.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }
    if (mRay.mOrigin.z >= max.z && mRay.mDirection.z < 0.0f)
    {
        float fT = (max.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
            }
        }
    }

    return fLowestT != +std::numeric_limits<float>::infinity();
}

bool axis_aligned_box::find_ray_intersection(const ray& mRay, vector3f& mIntersection, face& mFace) const
{
    if (contains(mRay.mOrigin))
    {
        mIntersection = mRay.mOrigin;
        return true;
    }

    // Check collision with each face of the bounding box (the 3 closest)
    float fLowestT = +std::numeric_limits<float>::infinity();

    // X
    if (mRay.mOrigin.x <= min.x && mRay.mDirection.x > 0.0f)
    {
        float fT = (min.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = LEFT;
            }
        }
    }
    if (mRay.mOrigin.x >= max.x && mRay.mDirection.x < 0.0f)
    {
        float fT = (max.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = RIGHT;
            }
        }
    }

    // Y
    if (mRay.mOrigin.y <= min.y && mRay.mDirection.y > 0.0f)
    {
        float fT = (min.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x<= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = BOTTOM;
            }
        }
    }
    if (mRay.mOrigin.y >= max.y && mRay.mDirection.y < 0.0f)
    {
        float fT = (max.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = TOP;
            }
        }
    }

    // Z
    if (mRay.mOrigin.z <= min.z && mRay.mDirection.z > 0.0f)
    {
        float fT = (min.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = FRONT;
            }
        }
    }
    if (mRay.mOrigin.z >= max.z && mRay.mDirection.z < 0.0f)
    {
        float fT = (max.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = BACK;
            }
        }
    }

    return fLowestT != +std::numeric_limits<float>::infinity();
}

void axis_aligned_box::get_inside_ray_intersection(const ray& mRay, vector3f& mIntersection, face& mFace) const
{
    // Check collision with each face of the bounding box (the 3 closest)
    float fLowestT = +std::numeric_limits<float>::infinity();

    // X
    if (mRay.mDirection.x > 0.0f)
    {
        float fT = (max.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = RIGHT;
            }
        }
    }
    if (mRay.mDirection.x < 0.0f)
    {
        float fT = (min.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = LEFT;
            }
        }
    }

    // Y
    if (mRay.mDirection.y > 0.0f)
    {
        float fT = (max.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x<= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = TOP;
            }
        }
    }
    if (mRay.mDirection.y < 0.0f)
    {
        float fT = (min.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = BOTTOM;
            }
        }
    }

    // Z
    if (mRay.mDirection.z > 0.0f)
    {
        float fT = (max.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                mIntersection = mPoint;
                mFace = BACK;
            }
        }
    }
    if (mRay.mDirection.z < 0.0f)
    {
        float fT = (min.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                mIntersection = mPoint;
                mFace = FRONT;
            }
        }
    }
}

void axis_aligned_box::get_inside_ray_intersection_ignore_face(
    const ray& mRay, vector3f& mIntersection, face& mFace, face mIgnore) const
{
    // Check collision with each face of the bounding box (the 3 closest)
    float fLowestT = +std::numeric_limits<float>::infinity();

    // X
    if (mRay.mDirection.x > 0.0f && mIgnore != RIGHT)
    {
        float fT = (max.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = RIGHT;
            }
        }
    }
    if (mRay.mDirection.x < 0.0f && mIgnore != LEFT)
    {
        float fT = (min.x - mRay.mOrigin.x)/mRay.mDirection.x;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.y <= mPoint.y && mPoint.y <= max.y &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = LEFT;
            }
        }
    }

    // Y
    if (mRay.mDirection.y > 0.0f && mIgnore != TOP)
    {
        float fT = (max.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x<= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = TOP;
            }
        }
    }
    if (mRay.mDirection.y < 0.0f && mIgnore != BOTTOM)
    {
        float fT = (min.y - mRay.mOrigin.y)/mRay.mDirection.y;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.z <= mPoint.z && mPoint.z <= max.z)
        {
            if (fT < fLowestT)
            {
                fLowestT = fT;
                mIntersection = mPoint;
                mFace = BOTTOM;
            }
        }
    }

    // Z
    if (mRay.mDirection.z > 0.0f && mIgnore != BACK)
    {
        float fT = (max.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                mIntersection = mPoint;
                mFace = BACK;
            }
        }
    }
    if (mRay.mDirection.z < 0.0f && mIgnore != FRONT)
    {
        float fT = (min.z - mRay.mOrigin.z)/mRay.mDirection.z;
        vector3f mPoint = mRay.mOrigin + mRay.mDirection*fT;
        if (min.x <= mPoint.x && mPoint.x <= max.x &&
            min.y <= mPoint.y && mPoint.y <= max.y)
        {
            if (fT < fLowestT)
            {
                mIntersection = mPoint;
                mFace = FRONT;
            }
        }
    }
}

std::ostream& operator << (std::ostream& o, const axis_aligned_box& mBox)
{
    return o << "(min : " << mBox.min << ", max : " << mBox.max << ")";
}
