#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <lxgui/utils.hpp>
#include "axisalignedbox.hpp"
#include "movable.hpp"
#include "node.hpp"
#include <array>
#include <functional>

// Undefine ugly macros from Windows if necessary
#ifdef NEAR
#undef NEAR
#endif
#ifdef FAR
#undef FAR
#endif

class camera : public movable
{
public :

    explicit camera(world& mWorld);
    camera(world& mWorld, const vector3f& mPosition);
    ~camera() = default;

    void  set_params(float fFOVy, float fAspectRatio, float fNear, float fFar);
    void  set_aspect_ratio(float fAspectRatio);
    float get_aspect_ratio() const;
    float get_fov_y() const;
    void  set_far_distance(float fFar);
    float get_far_distance() const;

    ray get_mouse_ray(float fMX, float fMY) const;
    vector3f project(const vector3f& mPos) const;

    bool is_visible(const axis_aligned_box& mBox) const;
    bool is_visible(const vector3f& mPos) const;
    bool is_visible(const vector3f& mPos, float fRadius) const;

    void yaw(float fYaw) override;
    void pitch(float fPitch) override;
    void roll(float fRoll) override;

    void bind() const;

    void set_on_moved_listener(std::function<void (movable::movement_type mType)> mFunc);

private :

    void on_moved_(movable::movement_type mType) override;

    void make_projection_() const;
    void make_view_() const;

    void make_culling_planes_() const;

    float fFOVy_ = 1.25f;
    float fNear_ = 0.1f;
    float fFar_ = 100.0f;
    float fAspectRatio_ = 1.0f;

    mutable matrix mViewMatrix_;
    mutable matrix mProjMatrix_;

    mutable bool bUpdateProjMatrix_ = true;
    mutable bool bUpdateViewMatrix_ = true;

    struct plane
    {
        vector3f normal;
        float  dist;
    };

    enum planes
    {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        NEAR,
        FAR
    };

    mutable std::array<plane,6> lCullingPlanes_;
    mutable bool                bUpdateCullingPlanes_ = true;

    std::function<void (movable::movement_type mType)> mOnMoved_;
};

#endif
