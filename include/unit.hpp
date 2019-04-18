#ifndef UNIT_HPP
#define UNIT_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/gui_eventreceiver.hpp>
#include "node.hpp"
#include "cylinder.hpp"
#include <memory>

class  world;
class  block_chunk;
class  block;
struct input_data;

class unit : public movable
{
public :

    unit(const utils::ustring& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock);
    virtual ~unit();

    void set_self(utils::wptr<movable> pSelf) override;

    void translate(const vector3f& mTrans) override;
    void yaw(float fYaw) override;
    void pitch(float fPitch) override;
    void roll(float fRoll) override;

    const utils::ustring& get_name() const { return sName_; }
    const std::string&    get_type() const { return sType_; }

    std::weak_ptr<const block_chunk> get_chunk() const { return pChunk_; }
    std::weak_ptr<block_chunk>       get_chunk() { return pChunk_; }
    const block*                     get_block() const { return pBlock_; }
    block*                           get_block() { return pBlock_; }
    utils::wptr<const node>          get_camera_anchor() const { return pCameraNode_; }
    utils::wptr<node>                get_camera_anchor() { return pCameraNode_; }

    const cylinder& get_cylinder() const { return mCylinder_; }

    bool occupies_block(const std::pair<std::weak_ptr<block_chunk>,block*>& mBlock) const;
    bool occupies_block(const std::pair<std::weak_ptr<const block_chunk>,const block*>& mBlock) const;

    virtual void update(input_data& mData) = 0;
    void         on_event(const gui::event& mEvent) override;

    virtual void notify_current(bool bCurrent);

protected :

    virtual void on_moved_(movable::movement_type mType);

    /*virtual void yaw_(float fYaw);
    virtual void pitch_(float fPitch);
    virtual void translate_(const vector3f& mTrans);
    virtual void translate_world_(const vector3f& mTrans);*/

    //void update_block_();

    utils::ustring    sName_;
    utils::wptr<unit> pUnitSelf_;
    std::string       sType_;

    bool bCurrent_;

    utils::refptr<node>   pCameraNode_;
    float                 fTotalPitch_;

    std::weak_ptr<block_chunk> pChunk_;
    block*                     pBlock_;

    vector3f mHeadPosition_;
    cylinder mCylinder_;
};

class god : public unit
{
public :

    god(const utils::ustring& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock);

    void translate(const vector3f& mTrans) override;
    void yaw(float fYaw) override;
    void pitch(float fPitch) override;

    void update(input_data& mData) override;

private :

    /*void translate_(const vector3f& mTrans) override;
    void yaw_(float fYaw) override;
    void pitch_(float fPitch) override;*/

};

#endif
