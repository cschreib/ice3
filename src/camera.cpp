#include "camera.hpp"

#include <SFML/OpenGL.hpp>
#include <cmath>
#include <iostream>

// Undefine ugly macros from Windows if necessary
#ifdef NEAR
#undef NEAR
#endif
#ifdef FAR
#undef FAR
#endif

camera::camera(utils::wptr<world> pWorld) :
    movable(pWorld), fFOVy_(1.25f), fNear_(0.1f), fFar_(100.0f),
    fAspectRatio_(1.0f), bUpdateProjMatrix_(true), bUpdateViewMatrix_(true)
{
}

camera::camera(utils::wptr<world> pWorld, const vector3f& mPosition) :
    movable(pWorld, mPosition), fFOVy_(1.25f), fNear_(0.1f), fFar_(100.0f),
    fAspectRatio_(1.0f), bUpdateProjMatrix_(true), bUpdateViewMatrix_(true)
{
}

void camera::set_params(float fFOVy, float fAspectRatio, float fNear, float fFar)
{
    fFOVy_        = fFOVy;
    fAspectRatio_ = fAspectRatio;
    fNear_        = fNear;
    fFar_         = fFar;

    bUpdateProjMatrix_ = true;
}

void camera::set_aspect_ratio(float fAspectRatio)
{
    fAspectRatio_ = fAspectRatio;

    bUpdateProjMatrix_ = true;
}

float camera::get_aspect_ratio() const
{
    return fAspectRatio_;
}

float camera::get_fov_y() const
{
    return fFOVy_;
}

void camera::set_far_distance(float fFar)
{
    if (fFar != fFar_)
    {
        fFar_ = fFar;
        bUpdateProjMatrix_ = true;
    }
}

float camera::get_far_distance() const
{
    return fFar_;
}

void camera::yaw(float fYaw)
{
    if (fYaw == 0.0f)
        return;

    mOrientation_ = quaternion(vector3f::UNIT_Y, fYaw)*mOrientation_;
    on_moved_(MVT_ROT);
}

void camera::pitch(float fPitch)
{
    if (fPitch == 0.0f)
        return;

    mOrientation_ = quaternion(mOrientation_*vector3f::UNIT_X, fPitch)*mOrientation_;
    on_moved_(MVT_ROT);
}

void camera::roll(float fRoll)
{
    if (fRoll == 0.0f)
        return;

    mOrientation_ = quaternion(mOrientation_*vector3f::UNIT_Z, fRoll)*mOrientation_;
    on_moved_(MVT_ROT);
}

void camera::on_moved_(movable::movement_type mType)
{
    movable::on_moved_(mType);

    bUpdateViewMatrix_ = true;

    if (mOnMoved_)
        mOnMoved_(mType);
}

void camera::set_on_moved_listener(std::function<void (movable::movement_type mType)> mFunc)
{
    mOnMoved_ = mFunc;
}

ray camera::get_mouse_ray(float fMX, float fMY) const
{
    matrix mMat = matrix::inverse(mProjMatrix_*mViewMatrix_);

    float fX = (2.0f*fMX) - 1.0f;
    float fY = 1.0f - (2.0f*fMY);

    ray mRay;
    mRay.mOrigin    = mMat*vector3f(fX, fY, 0.0f);
    mRay.mDirection = mMat*vector3f(0.0f, 0.0f, 1.0f);
    mRay.mDirection.normalize();

    return mRay;
}

vector3f camera::project(const vector3f& mPos) const
{
    return mProjMatrix_*mViewMatrix_*mPos;
}

bool camera::is_visible(const axis_aligned_box& mBox) const
{
    if (mBox.is_infinite())
        return true;

    if (mBox.max == mBox.min)
        return false;

    make_culling_planes_();

    vector3f mCenter = (mBox.max + mBox.min)/2.0f;
    vector3f mDist   = (mBox.max - mBox.min)/2.0f;

    for (int i = 0; i < 6; ++i)
    {
        float fDistance = lCullingPlanes_[i].normal*mCenter + lCullingPlanes_[i].dist;
        float fMaxDist  = fabs(lCullingPlanes_[i].normal.x*mDist.x) +
                          fabs(lCullingPlanes_[i].normal.y*mDist.y) +
                          fabs(lCullingPlanes_[i].normal.z*mDist.z);

        if (fDistance < -fMaxDist)
            return false;
    }

    return true;
}

bool camera::is_visible(const vector3f& mPos) const
{
    make_culling_planes_();

    for (int i = 0; i < 6; ++i)
    {
        float fDistance = lCullingPlanes_[i].normal*mPos + lCullingPlanes_[i].dist;
        if (fDistance < 0.0f)
            return false;
    }

    return true;
}

bool camera::is_visible(const vector3f& mPos, float fRadius) const
{
    make_culling_planes_();

    for (int i = 0; i < 6; ++i)
    {
        float fDistance = lCullingPlanes_[i].normal*mPos + lCullingPlanes_[i].dist;
        if (fDistance < -fRadius)
            return false;
    }

    return true;
}

void camera::make_projection_() const
{
    if (!bUpdateProjMatrix_)
        return;

    float fTop = fNear_*tan(fFOVy_/2.0);
    float fBottom = -fTop;
    float fRight = fTop*fAspectRatio_;
    float fLeft = -fRight;

    float A = 2.0f*fNear_/(fRight - fLeft);
    float B = 2.0f*fNear_/(fTop - fBottom);
    float C = -(fFar_ + fNear_)/(fFar_ - fNear_);
    float D = -2.0f*fFar_*fNear_/(fFar_ - fNear_);

    mProjMatrix_ = {
        A   , 0.0f,  0.0f, 0.0f,
        0.0f, B   ,  0.0f, 0.0f,
        0.0f, 0.0f,  C   , D   ,
        0.0f, 0.0f, -1.0f, 0.0f
    };

    bUpdateProjMatrix_ = false;
    bUpdateCullingPlanes_ = true;
}

void camera::make_view_() const
{
    if (!bUpdateViewMatrix_)
        return;

    vector3f   mPos, mScale;
    quaternion mOrient;

    get_absolute_transform(mPos, mScale, mOrient);

    mViewMatrix_ = matrix::IDENTITY;

    vector3f mLookAt = (mOrient*vector3f::UNIT_Z).get_unit();
    vector3f mUp     = (mOrient*vector3f::UNIT_Y).get_unit();
    vector3f mLeft   = mUp^mLookAt;

    mViewMatrix_(0,0) = mLeft.x;
    mViewMatrix_(0,1) = mLeft.y;
    mViewMatrix_(0,2) = mLeft.z;

    mViewMatrix_(1,0) = mUp.x;
    mViewMatrix_(1,1) = mUp.y;
    mViewMatrix_(1,2) = mUp.z;

    mViewMatrix_(2,0) = mLookAt.x;
    mViewMatrix_(2,1) = mLookAt.y;
    mViewMatrix_(2,2) = mLookAt.z;

    matrix mTrans = matrix::translation(-mPos);

    mViewMatrix_ = mViewMatrix_*mTrans;

    bUpdateViewMatrix_ = false;
    bUpdateCullingPlanes_ = true;
}

void camera::make_culling_planes_() const
{
    if (!bUpdateCullingPlanes_)
        return;

    make_projection_();
    make_view_();

    matrix mTemp = mProjMatrix_*mViewMatrix_;

    lCullingPlanes_[LEFT].normal   = vector3f(mTemp(3,0) + mTemp(0,0), mTemp(3,1) + mTemp(0,1), mTemp(3,2) + mTemp(0,2));
    lCullingPlanes_[RIGHT].normal  = vector3f(mTemp(3,0) - mTemp(0,0), mTemp(3,1) - mTemp(0,1), mTemp(3,2) - mTemp(0,2));
    lCullingPlanes_[TOP].normal    = vector3f(mTemp(3,0) - mTemp(1,0), mTemp(3,1) - mTemp(1,1), mTemp(3,2) - mTemp(1,2));
    lCullingPlanes_[BOTTOM].normal = vector3f(mTemp(3,0) + mTemp(1,0), mTemp(3,1) + mTemp(1,1), mTemp(3,2) + mTemp(1,2));
    lCullingPlanes_[NEAR].normal   = vector3f(mTemp(3,0) + mTemp(2,0), mTemp(3,1) + mTemp(2,1), mTemp(3,2) + mTemp(2,2));
    lCullingPlanes_[FAR].normal    = vector3f(mTemp(3,0) - mTemp(2,0), mTemp(3,1) - mTemp(2,1), mTemp(3,2) - mTemp(2,2));

    lCullingPlanes_[LEFT].dist     = mTemp(3,3) + mTemp(0,3);
    lCullingPlanes_[RIGHT].dist    = mTemp(3,3) - mTemp(0,3);
    lCullingPlanes_[TOP].dist      = mTemp(3,3) - mTemp(1,3);
    lCullingPlanes_[BOTTOM].dist   = mTemp(3,3) + mTemp(1,3);
    lCullingPlanes_[NEAR].dist     = mTemp(3,3) + mTemp(2,3);
    lCullingPlanes_[FAR].dist      = mTemp(3,3) - mTemp(2,3);

    for (int i = 0; i < 6; ++i)
    {
        float length = lCullingPlanes_[i].normal.get_norm();

        lCullingPlanes_[i].normal /= length;
        lCullingPlanes_[i].dist   /= length;
    }

    bUpdateCullingPlanes_ = false;
}

void camera::bind() const
{
    make_projection_();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<float*>(&mProjMatrix_));

    make_view_();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(reinterpret_cast<float*>(&mViewMatrix_));
}
