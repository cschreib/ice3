#include "light.hpp"
#include "blockchunk.hpp"
#include "world.hpp"

const uchar light::ATTEN = 16u;

static std::array<float,256> make_intensity_table()
{
    std::array<float,256> lArray;
    for (int i = 0; i < 256; ++i)
        lArray[i] = exp(-(255-i)*(16.0f/255.0f)/5.0f);

    return lArray;
}

static std::array<uint,256> make_range_table()
{
    std::array<uint,256> lArray;
    for (int i = 0; i < 256; ++i)
        lArray[i] = ceil((i+1)/(float)light::ATTEN);

    return lArray;
}

const std::array<uint,256>  light::RANGE_TABLE     = make_range_table();
const std::array<float,256> light::INTENSITY_TABLE = make_intensity_table();

light::light(const std::string& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock) :
    movable(pWorld, vector3f(pChunk.lock()->get_block_position(pBlock))),
    sName_(sName), pChunk_(pChunk), pBlock_(pBlock), ucIntensity_(255u)
{
}

light::~light()
{
}

void light::set_self(utils::wptr<movable> pMovable)
{
    movable::set_self(pMovable);
    pLightSelf_ = utils::wptr<light>::dyn_cast(pMovable);
}

void light::set_intensity(uchar ucIntensity)
{
    if (ucIntensity_ != ucIntensity)
    {
        ucIntensity_ = ucIntensity;
        std::shared_ptr<block_chunk> pLocked = pChunk_.lock();
        /*if (pLocked)
            pLocked->update_light(pLightSelf_);*/
    }
}

uchar light::get_intensity() const
{
    return ucIntensity_;
}

void light::on_moved_(movable::movement_type mType)
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

        utils::refptr<light> pSelfLocked = pLightSelf_.lock();

        //pChunkLocked->remove_light(pLightSelf_);
        pChunk_ = pNewChunk;
        mPosition_ += block_chunk::get_difference(pNewChunk, pChunkLocked);
        pChunkLocked = pNewChunk;
        pBlock_ = pChunkLocked->get_block(mBPos);
        //pChunkLocked->add_light(pSelfLocked);
    }
    else if (mBPos != pChunkLocked->get_block_position(pBlock_))
    {
        block* pOldBlock = pBlock_;
        pBlock_ = pChunkLocked->get_block(mBPos);
        //pChunkLocked->move_light(pLightSelf_, pOldBlock);
    }

    movable::on_moved_(mType);
}
