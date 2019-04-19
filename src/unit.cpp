#include "unit.hpp"
#include "world.hpp"

#include <lxgui/gui_event.hpp>
#include <lxgui/gui_eventmanager.hpp>
#include <lxgui/utils_string.hpp>

unit::unit(const utils::ustring& sName, world& mWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock) :
    movable(mWorld, vector3f(pChunk.lock()->get_block_position(pBlock))),
    sName_(sName), mCameraNode_(mWorld_), pChunk_(pChunk), pBlock_(pBlock)
{
    mCameraNode_.set_parent(this);

    sType_ = "UNIT";

    mWorld_.notify_unit_loaded(*this);
}

unit::~unit()
{
    mWorld_.notify_unit_unloaded(*this);
}

void unit::notify_current(bool bCurrent)
{
    bCurrent_ = bCurrent;
}

void unit::on_event(const gui::event& mEvent)
{
}

bool unit::occupies_block(const std::pair<std::weak_ptr<block_chunk>,block*>& mBlock) const
{
    std::shared_ptr<block_chunk> pChunk = mBlock.first.lock();
    if (!pChunk || !mBlock.second)
        return false;

    axis_aligned_box mBlockBox(
        vector3f(-0.5f, -0.5f, -0.5f),
        vector3f( 0.5f,  0.5f,  0.5f)
    );

    mBlockBox += pChunk->get_block_world_position(mBlock.second);

    axis_aligned_box mBox = mCylinder_.make_bounding_box();
    mBox += get_absolute_position();

    return mBox.contains(mBlockBox);
}

bool unit::occupies_block(const std::pair<std::weak_ptr<const block_chunk>,const block*>& mBlock) const
{
    std::shared_ptr<const block_chunk> pChunk = mBlock.first.lock();
    if (!pChunk || !mBlock.second)
        return false;

    axis_aligned_box mBlockBox(
        vector3f(-0.5f, -0.5f, -0.5f),
        vector3f( 0.5f,  0.5f,  0.5f)
    );

    mBlockBox += pChunk->get_block_world_position(mBlock.second);

    axis_aligned_box mBox = mCylinder_.make_bounding_box();
    mBox += get_absolute_position();

    return mBox.contains(mBlockBox);
}

void unit::translate(const vector3f& mTrans)
{
    movable::translate(mCameraNode_.get_absolute_orientation()*mTrans);
}

void unit::yaw(float fYaw)
{
    if (fYaw != 0.0f)
        set_orientation(quaternion(vector3f::UNIT_Y, fYaw)*get_orientation());
}

void unit::pitch(float fPitch)
{
    if (fPitch == 0.0f)
        return;
    if (fTotalPitch_ == +1.57f && fPitch > 0)
        return;
    if (fTotalPitch_ == -1.57f && fPitch < 0)
        return;

    fTotalPitch_ += fPitch;
    if (fTotalPitch_ < -1.57f) fTotalPitch_ = -1.57f;
    if (fTotalPitch_ > +1.57f) fTotalPitch_ = +1.57f;
    mCameraNode_.set_orientation(quaternion(vector3f::UNIT_X, fTotalPitch_));
}

void unit::roll(float fRoll)
{
}

void unit::on_moved_(movable::movement_type mType)
{
    vector3f mPos  = get_absolute_position();
    vector3i mCPos = block_chunk::to_chunk_pos(mPos);
    vector3i mBPos = block_chunk::to_block_pos(mPos);

    std::shared_ptr<block_chunk> pCurrentLocked = mWorld_.get_current_chunk().lock();
    if (pCurrentLocked)
        mCPos += pCurrentLocked->get_coordinates();

    std::shared_ptr<block_chunk> pChunkLocked = pChunk_.lock();
    if (!pChunkLocked)
        return;

    if (mCPos != pChunkLocked->get_coordinates())
    {
        std::shared_ptr<block_chunk> pNewChunk = mWorld_.get_chunk(mCPos).lock();

        if (!pNewChunk)
            return;

        pChunkLocked->transfer_unit_ownership(*this, *pNewChunk);
        pChunk_ = pNewChunk;
        mPosition_ += block_chunk::get_difference(pNewChunk, pChunkLocked);
        pChunkLocked = pNewChunk;
        pBlock_ = pChunkLocked->get_block(mBPos);

        if (bCurrent_)
            mWorld_.set_current_chunk(pChunk_);
    }
    else if (mBPos != pChunkLocked->get_block_position(pBlock_))
        pBlock_ = pChunkLocked->get_block(mBPos);

    movable::on_moved_(mType);
}

god::god(const utils::ustring& sName, world& mWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock) :
    unit(sName, mWorld, pChunk, pBlock)
{
    sType_ = "GOD";
}

void god::update(input_data& mData)
{
    if (!bCurrent_)
        return;

    const float fSpeed = 10.0f;
    const float fTurnSpeed = 3.0f;

    if (!mData.mInput.can_receive_input("WORLD") ||
        !mData.mInput.can_receive_input("UNIT"))
        return;

    if (mData.mInput.key_is_down(input::key::K_Z))
        translate(vector3f::UNIT_Z*(-fSpeed*mData.fDelta));

    if (mData.mInput.key_is_down(input::key::K_S))
        translate(vector3f::UNIT_Z*(+fSpeed*mData.fDelta));

    if (mData.mInput.key_is_down(input::key::K_Q))
        translate(vector3f::UNIT_X*(-fSpeed*mData.fDelta));

    if (mData.mInput.key_is_down(input::key::K_D))
        translate(vector3f::UNIT_X*(+fSpeed*mData.fDelta));

    if (mData.mInput.key_is_down(input::key::K_A))
        translate(vector3f::UNIT_Y*(+fSpeed*mData.fDelta));

    if (mData.mInput.key_is_down(input::key::K_E))
        translate(vector3f::UNIT_Y*(-fSpeed*mData.fDelta));

    static bool bFirstMovement = true;

    if (!bFirstMovement && !mData.mInput.shift_is_pressed())
    {
        yaw  (-mData.mInput.get_mouse_rel_dx()*fTurnSpeed);
        pitch(-mData.mInput.get_mouse_rel_dy()*fTurnSpeed);
    }

    if (mData.mInput.get_mouse_rel_dx() != 0.0f ||
        mData.mInput.get_mouse_rel_dy() != 0.0f)
        bFirstMovement = false;

    /*int iMouseWheel = mData.mInput.get_mouse_wheel();
    if (iMouseWheel != 0)
        SelectItemSlot(-iMouseWheel);*/

    if (bCurrent_)
    {
        /*mWorld_.SelectblockOnScreen(pCamera_->GetMouseRay(0.5f, 0.5f));

        if (InputManager::GetSingleton()->MouseIsDown(MOUSE_LEFT))
            FirstAction(fDelta);
        if (InputManager::GetSingleton()->MouseIsDown(MOUSE_RIGHT))
            SecondAction(fDelta);

        if (InputManager::GetSingleton()->KeyIsPressed(KEY_L))
        {
            if (mWorld_.GetSelectedblock_chunk())
            {
                std::pair<std::weak_ptr<block_chunk>,block*> mNext =
                    mWorld_.GetSelectedblock_chunk()->get_block(
                        mWorld_.GetSelectedblockFace(), mWorld_.GetSelectedblock()
                    );

                if (mNext.second)
                {
                    if (InputManager::GetSingleton()->ShiftPressed())
                        mWorld_.DeleteLight(mNext.first, mNext.second);
                    else
                        mWorld_.CreateLight(mNext.first, mNext.second);
                }
            }
        }

        if (InputManager::GetSingleton()->KeyIsPressed(KEY_C))
        {
            vector3f mPosition = pNode_->get_position() + pChunk_->get_position(mWorld_.get_current_chunk());
            std::weak_ptr<block_chunk> pChunk = mWorld_.get_chunk(mPosition);
            if (pChunk)
            {
                if (pChunk != pChunk_)
                {
                    std::shared_ptr<unit> pSelfLocked = pSelf_.lock();

                    pChunk_->Removeunit(pSelf_);

                    pNode_->set_position(mPosition + block_chunk::get_difference(pChunk, pChunk_));
                    pChunk_ = pChunk;

                    pChunk_->Addunit(pSelfLocked);

                    mWorld_.SetCurrentChunk(pChunk_);
                }
            }
        }*/
    }
}

void god::translate(const vector3f& mTrans)
{
    movable::translate(mTrans);
}

void god::yaw(float fYaw)
{
    if (fYaw != 0.0f)
        set_orientation(quaternion(vector3f::UNIT_Y, fYaw)*get_orientation());
}

void god::pitch(float fPitch)
{
    if (fPitch != 0.0f)
        set_orientation(get_orientation()*quaternion(vector3f::UNIT_X, fPitch));
}
