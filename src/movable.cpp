#include "movable.hpp"
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_exception.hpp>

movable::movable(world& mWorld) : mWorld_(mWorld)
{
}

movable::movable(world& mWorld, const vector3f& mPosition) :
    mWorld_(mWorld), mPosition_(mPosition)
{
}


movable::~movable()
{
    set_parent(nullptr);

    for (auto child : lChildList_)
    {
        child->pParent_ = nullptr;
        child->on_moved_(MVT_OTHER);
    }
}

void movable::set_parent(movable* pParent)
{
    if (pParent == this) {
        throw utils::exception("# Error # : movable : cannot set self as parent.");
    }

    if (pParent_ != pParent)
    {
        if (pParent_)
            pParent_->lChildList_.erase(utils::find(pParent_->lChildList_, this));

        pParent_ = pParent;

        if (pParent_)
            pParent_->lChildList_.push_back(this);

        on_moved_(MVT_OTHER);
    }
}

movable* movable::get_parent() const
{
    return pParent_;
}

void movable::set_position(const vector3f& mPos)
{
    mPosition_ = mPos;
    on_moved_(MVT_TRANS);
}

void movable::translate(const vector3f& mTrans)
{
    if (mTrans == vector3f::ZERO)
        return;

    mPosition_ += get_absolute_orientation()*mTrans;
    on_moved_(MVT_TRANS);
}

void movable::set_scale(const vector3f& mScale)
{
    mScale_ = mScale;
    on_moved_(MVT_SCALE);
}

void movable::scale_up(const vector3f& mScale)
{
    if (mScale == vector3f::UNIT)
        return;

    mScale_.scale_up(mScale);
    on_moved_(MVT_SCALE);
}

void movable::set_orientation(const quaternion& mQuat)
{
    mOrientation_ = mQuat;
    on_moved_(MVT_ROT);
}

void movable::rotate(const quaternion& mQuat)
{
    if (mQuat == quaternion::UNIT)
        return;

    mOrientation_ = mOrientation_*mQuat;
    on_moved_(MVT_ROT);
}

void movable::yaw(float fYaw)
{
    if (fYaw == 0.0f)
        return;

    mOrientation_ = mOrientation_*quaternion(vector3f::UNIT_Y, fYaw);
    on_moved_(MVT_ROT);
}

void movable::pitch(float fPitch)
{
    if (fPitch == 0.0f)
        return;

    mOrientation_ = mOrientation_*quaternion(vector3f::UNIT_X, fPitch);
    on_moved_(MVT_ROT);
}

void movable::roll(float fRoll)
{
    if (fRoll == 0.0f)
        return;

    mOrientation_ = mOrientation_*quaternion(vector3f::UNIT_Z, fRoll);
    on_moved_(MVT_ROT);
}

const vector3f& movable::get_position() const
{
    return mPosition_;
}

vector3f movable::get_absolute_position() const
{
    if (pParent_)
    {
        vector3f mAbs = pParent_->get_absolute_orientation()*vector3f::scale_up(mPosition_, pParent_->get_absolute_scale());
        mAbs += pParent_->get_absolute_position();
        return mAbs;
    }
    else
        return mPosition_;
}

const vector3f& movable::get_scale() const
{
    return mScale_;
}

vector3f movable::get_absolute_scale() const
{
    vector3f mAbs = mScale_;
    if (bInheritScale_ && pParent_)
        mAbs = vector3f::scale_up(mAbs, pParent_->get_absolute_scale());

    return mAbs;
}

const quaternion& movable::get_orientation() const
{
    return mOrientation_;
}

quaternion movable::get_absolute_orientation() const
{
    quaternion mAbs = mOrientation_;
    if (bInheritRotation_ && pParent_)
        mAbs = pParent_->get_absolute_orientation()*mAbs;

    return mAbs;
}

matrix movable::get_absolute_transform() const
{
    vector3f   mPos, mScale;
    quaternion mOrient;

    get_absolute_transform(mPos, mScale, mOrient);

    return matrix::transform(mPos, mScale, mOrient);
}

void movable::get_absolute_transform(vector3f& mPosition, vector3f& mScale, quaternion& mOrientation) const
{
    mPosition    = mPosition_;
    mScale       = mScale_;
    mOrientation = mOrientation_;

    if (pParent_)
    {
        vector3f mParentScale = pParent_->get_absolute_scale();
        if (bInheritScale_)
            mScale = vector3f::scale_up(mScale, mParentScale);

        quaternion mParentOrient = pParent_->get_absolute_orientation();
        if (bInheritRotation_)
            mOrientation = mParentOrient*mOrientation;

        mPosition = mParentOrient*vector3f::scale_up(mPosition_, mParentScale);
        mPosition += pParent_->get_absolute_position();
    }
}

void movable::on_moved_(movement_type mType)
{
    for (auto& child : lChildList_)
        child->on_moved_(MVT_OTHER);
}

void movable::on_event(const gui::event& mEvent)
{
}
