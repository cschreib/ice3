#ifndef VECTOR2_HPP
#define VECTOR2_HPP

#include <utils.hpp>

class vector2f
{
public :

    vector2f() {}

    vector2f(float mX, float mY) : x(mX), y(mY) {}

    float get_norm() const {
        return sqrt(x*x + y*y);
    }

    float get_norm_squared() const {
        return x*x + y*y;
    }

    void normalize() {
        float fNorm = sqrt(x*x + y*y);
        x /= fNorm; y /= fNorm;
    }

    vector2f get_unit() const {
        float fNorm = sqrt(x*x + y*y);
        return vector2f(x/fNorm, y/fNorm);
    }

    void rotate(float fAngle) {
        vector2f p;
        p.x = x*cos(fAngle) - y*sin(fAngle);
        p.y = x*sin(fAngle) + y*cos(fAngle);
        *this = p;
    }

    vector2f GetRotation(float fAngle) const {
        return vector2f(x*cos(fAngle) - y*sin(fAngle), x*sin(fAngle) + y*cos(fAngle));
    }

    vector2f operator + () const {
        return *this;
    }

    vector2f operator - () const {
        return vector2f(-x, -y);
    }

    vector2f operator + (const vector2f& mVec) const {
        return vector2f(x + mVec.x, y + mVec.y);
    }

    vector2f operator - (const vector2f& mVec) const {
        return vector2f(x - mVec.x, y - mVec.y);
    }

    void operator += (const vector2f& mVec) {
        x += mVec.x; y += mVec.y;
    }

    void operator -= (const vector2f& mVec) {
        x -= mVec.x; y -= mVec.y;
    }

    vector2f operator * (float mValue) const {
        return vector2f(x*mValue, y*mValue);
    }

    vector2f operator / (float mValue) const {
        return vector2f(x/mValue, y/mValue);
    }

    void operator *= (float mValue) {
        x *= mValue; y *= mValue;
    }

    void operator /= (float mValue) {
        x /= mValue; y /= mValue;
    }

    float operator * (const vector2f& mVec) const {
        return x*mVec.x + y*mVec.y;
    }

    bool operator == (const vector2f& mVec) const {
        return x == mVec.x && y == mVec.y;
    }

    bool operator != (const vector2f& mVec) const {
        return x != mVec.x || y != mVec.y;
    }

    float x, y;
};

inline vector2f operator * (float fValue, const vector2f& mVec) {
    return vector2f(mVec.x*fValue, mVec.y*fValue);
}

class vector2i
{
public :

    vector2i() {}

    vector2i(int mX, int mY) : x(mX), y(mY) {}

    vector2i operator + () const {
        return *this;
    }

    vector2i operator - () const {
        return vector2i(-x, -y);
    }

    vector2i operator + (const vector2i& mVec)  const {
        return vector2i(x + mVec.x, y + mVec.y);
    }

    vector2i operator - (const vector2i& mVec) const {
        return vector2i(x - mVec.x, y - mVec.y);
    }

    void operator += (const vector2i& mVec) {
        x += mVec.x; y += mVec.y;
    }

    void operator -= (const vector2i& mVec) {
        x -= mVec.x; y -= mVec.y;
    }

    bool operator == (const vector2i& mVec) const {
        return x == mVec.x && y == mVec.y;
    }

    bool operator != (const vector2i& mVec) const {
        return x != mVec.x || y != mVec.y;
    }

    int operator * (const vector2i& mVec) const {
        return x*mVec.x + y*mVec.y;
    }

    int x, y;
};

#endif
