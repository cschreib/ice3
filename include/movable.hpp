#ifndef MOVABLE_HPP
#define MOVABLE_HPP

#include <utils.hpp>
#include <utils_wptr.hpp>
#include <utils_refptr.hpp>
#include <gui_eventreceiver.hpp>
#include "vector3.hpp"
#include "quaternion.hpp"
#include "matrix.hpp"
#include <vector>

class world;

class movable : public gui::event_receiver
{
public :

    movable(utils::wptr<world> pWorld);
    movable(utils::wptr<world> pWorld, const vector3f& mPosition);
    virtual ~movable() = 0;

    virtual void set_self(utils::wptr<movable> pSelf);

    utils::wptr<const world> get_world() const { return pWorld_; }
    utils::wptr<world>       get_world() { return pWorld_; }

    void set_parent(utils::wptr<movable> pParent);
    utils::wptr<movable> get_parent() const;

    virtual void set_position(const vector3f& mPos);
    virtual void translate(const vector3f& mTrans);
    virtual void set_scale(const vector3f& mScale);
    virtual void scale_up(const vector3f& mScale);
    virtual void set_orientation(const quaternion& mQuat);
    virtual void rotate(const quaternion& mQuat);
    virtual void yaw(float fYaw);
    virtual void pitch(float fPitch);
    virtual void roll(float fRoll);

    const vector3f&   get_position() const;
    vector3f          get_absolute_position() const;
    const vector3f&   get_scale() const;
    vector3f          get_absolute_scale() const;
    const quaternion& get_orientation() const;
    quaternion        get_absolute_orientation() const;
    matrix            get_absolute_transform() const;
    void              get_absolute_transform(vector3f& mPosition, vector3f& mScale, quaternion& mOrientation) const;

    void on_event(const gui::event& mEvent) override;

    enum movement_type
    {
        MVT_TRANS,
        MVT_SCALE,
        MVT_ROT,
        MVT_OTHER
    };

protected :

    virtual void on_moved_(movement_type mType);

    utils::wptr<movable> pSelf_;
    utils::wptr<world>   pWorld_;

    utils::wptr<movable>              pParent_;
    std::vector<utils::wptr<movable>> lChildList_;

    vector3f   mPosition_;
    vector3f   mScale_;
    quaternion mOrientation_;

    bool bInheritScale_;
    bool bInheritRotation_;

};

#endif
