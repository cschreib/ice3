#include "unit.hpp"
#include "world.hpp"

#include <gui_event.hpp>
#include <gui_eventmanager.hpp>
#include <utils_string.hpp>

unit::unit(const utils::ustring& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock) :
    movable(pWorld, vector3f(pChunk.lock()->get_block_position(pBlock))),
    sName_(sName), pChunk_(pChunk), pBlock_(pBlock)
{
    pCameraNode_ = utils::refptr<node>(new node(pWorld_));
    pCameraNode_->set_self(pCameraNode_);

    sType_ = "UNIT";
}

unit::~unit()
{
    pWorld_->notify_unit_unloaded(pUnitSelf_);
}

void unit::set_self(utils::wptr<movable> pSelf)
{
    movable::set_self(pSelf);
    pCameraNode_->set_parent(pSelf);
    pUnitSelf_ = utils::wptr<unit>::dyn_cast(pSelf);
    pWorld_->notify_unit_loaded(pUnitSelf_);
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
    movable::translate(pCameraNode_->get_absolute_orientation()*mTrans);
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
    pCameraNode_->set_orientation(quaternion(vector3f::UNIT_X, fTotalPitch_));
}

void unit::roll(float fRoll)
{
}

void unit::on_moved_(movable::movement_type mType)
{
    vector3f mPos  = get_absolute_position();
    vector3i mCPos = block_chunk::to_chunk_pos(mPos);
    vector3i mBPos = block_chunk::to_block_pos(mPos);

    std::shared_ptr<block_chunk> pCurrentLocked = pWorld_->get_current_chunk().lock();
    if (pCurrentLocked)
        mCPos += pCurrentLocked->get_coordinates();

    std::shared_ptr<block_chunk> pChunkLocked = pChunk_.lock();
    if (!pChunkLocked)
        return;

    if (mCPos != pChunkLocked->get_coordinates())
    {
        std::shared_ptr<block_chunk> pNewChunk = pWorld_->get_chunk(mCPos).lock();

        if (!pNewChunk)
            return;

        utils::refptr<unit> pSelfLocked = pUnitSelf_.lock();

        pChunkLocked->remove_unit(pUnitSelf_);
        pChunk_ = pNewChunk;
        mPosition_ += block_chunk::get_difference(pNewChunk, pChunkLocked);
        pChunkLocked = pNewChunk;
        pBlock_ = pChunkLocked->get_block(mBPos);
        pChunkLocked->add_unit(pSelfLocked);

        if (bCurrent_)
            pWorld_->set_current_chunk(pChunk_);
    }
    else if (mBPos != pChunkLocked->get_block_position(pBlock_))
        pBlock_ = pChunkLocked->get_block(mBPos);

    movable::on_moved_(mType);
}

god::god(const utils::ustring& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock) :
    unit(sName, pWorld, pChunk, pBlock)
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
        /*pWorld_->SelectblockOnScreen(pCamera_->GetMouseRay(0.5f, 0.5f));

        if (InputManager::GetSingleton()->MouseIsDown(MOUSE_LEFT))
            FirstAction(fDelta);
        if (InputManager::GetSingleton()->MouseIsDown(MOUSE_RIGHT))
            SecondAction(fDelta);

        if (InputManager::GetSingleton()->KeyIsPressed(KEY_L))
        {
            if (pWorld_->GetSelectedblock_chunk())
            {
                std::pair<std::weak_ptr<block_chunk>,block*> mNext =
                    pWorld_->GetSelectedblock_chunk()->get_block(
                        pWorld_->GetSelectedblockFace(), pWorld_->GetSelectedblock()
                    );

                if (mNext.second)
                {
                    if (InputManager::GetSingleton()->ShiftPressed())
                        pWorld_->DeleteLight(mNext.first, mNext.second);
                    else
                        pWorld_->CreateLight(mNext.first, mNext.second);
                }
            }
        }

        if (InputManager::GetSingleton()->KeyIsPressed(KEY_C))
        {
            vector3f mPosition = pNode_->get_position() + pChunk_->get_position(pWorld_->get_current_chunk());
            std::weak_ptr<block_chunk> pChunk = pWorld_->get_chunk(mPosition);
            if (pChunk)
            {
                if (pChunk != pChunk_)
                {
                    utils::refptr<unit> pSelfLocked = pSelf_.lock();

                    pChunk_->Removeunit(pSelf_);

                    pNode_->set_position(mPosition + block_chunk::get_difference(pChunk, pChunk_));
                    pChunk_ = pChunk;

                    pChunk_->Addunit(pSelfLocked);

                    pWorld_->SetCurrentChunk(pChunk_);
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
