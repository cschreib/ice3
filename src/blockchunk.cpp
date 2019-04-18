#include "blockchunk.hpp"
#include "unit.hpp"
#include "world.hpp"
#include "camera.hpp"
#include "axisalignedbox.hpp"
#include "vertexbufferobject.hpp"

//#define PRINT_UPDATES

const int  block_chunk::HALF_CHUNK_SIZE = block_chunk::CHUNK_SIZE/2;
uint       block_chunk::CHUNK_COUNT = 0u;

const axis_aligned_box block_chunk::BOUNDING_BOX = axis_aligned_box(
    vector3f(
       -0.5f*float(block_chunk::CHUNK_SIZE),
       -0.5f*float(block_chunk::CHUNK_SIZE),
       -0.5f*float(block_chunk::CHUNK_SIZE)
    ),
    vector3f(
        0.5f*float(block_chunk::CHUNK_SIZE),
        0.5f*float(block_chunk::CHUNK_SIZE),
        0.5f*float(block_chunk::CHUNK_SIZE)
    )
);

const std::array<block_chunk::anchor,6> block_chunk::OPPOSED_LIST = {{
    block_chunk::RIGHT, block_chunk::LEFT,   block_chunk::BACK,
    block_chunk::FRONT, block_chunk::BOTTOM, block_chunk::TOP
}};

block_chunk::block_chunk(utils::wptr<world> pWorld, const vector3i& mPos) :
    pWorld_(pWorld), mCoordinates_(mPos)
{
    for (auto& data : lFaceTranspBlockCount_)
        data = CHUNK_SIZE*CHUNK_SIZE;

    ++CHUNK_COUNT;
}

block_chunk::~block_chunk()
{
    --CHUNK_COUNT;

    if (bClearNeighborList_)
        clear_links();
}

void block_chunk::set_self(std::weak_ptr<block_chunk> pSelf)
{
    pSelf_ = pSelf;
}

utils::wptr<const world> block_chunk::get_world() const
{
    return pWorld_;
}

utils::wptr<world> block_chunk::get_world()
{
    return pWorld_;
}

void block_chunk::flag_clear_links() const
{
    bClearNeighborList_ = true;
}

void block_chunk::clear_links() const
{
    for (int i = 0; i < 6; ++i)
    {
        std::shared_ptr<block_chunk> pChunk = lNeighborList_[i].lock();
        if (pChunk)
        {
            pChunk->lNeighborList_[OPPOSED_LIST[i]].reset();
            --pChunk->uiValidNeighbor_;
            pChunk->update_burried_state_();
        }
    }

    bClearNeighborList_ = false;
}

void block_chunk::clear_vbos()
{
    pNormalVBO_       = nullptr;
    pTwoSidedVBO_     = nullptr;
    pAlphaBlendedVBO_ = nullptr;
    bVBOCleared_      = true;
}

vector3i block_chunk::get_block_position(const block* pBlock) const
{
    static const int size     = CHUNK_SIZE;
    static const int size2    = size*size;
    static const int halfsize = HALF_CHUNK_SIZE;

    int diff = pBlock - &lBlockList_[0];

    vector3i pos;
    pos.z = diff/size2;
    pos.x = diff%size;
    pos.y = diff/size - pos.z*size;

    pos.x -= halfsize;
    pos.y -= halfsize;
    pos.z -= halfsize;

    return pos;
}

vector3f block_chunk::get_block_world_position(const block* pBlock) const
{
    return vector3f(get_block_position(pBlock)) + get_position(pWorld_->get_current_chunk());
}

bool block_chunk::is_empty() const
{
    return uiPlainBlockCount_ == 0u;
}

void block_chunk::attach_to(std::weak_ptr<block_chunk> pWChunk, block_chunk::anchor mAnchor)
{
    std::shared_ptr<block_chunk> pChunk = pWChunk.lock();
    std::shared_ptr<block_chunk> pOldNeighbour = lNeighborList_[mAnchor].lock();

    #ifdef PRINT_UPDATES
    std::cout << "attach_to : " << get_coordinates();
    if (pChunk)
        std::cout << " -> " << pChunk->get_coordinates() << std::endl;
    else
        std::cout << " -> null" << std::endl;
    #endif

    if (pOldNeighbour == pChunk)
        return;

    block_chunk::anchor mOpp = get_opposed(mAnchor);

    if (pOldNeighbour)
    {
        pOldNeighbour->lNeighborList_[mOpp].reset();
        --pOldNeighbour->uiValidNeighbor_;
        pOldNeighbour->bUpdateCache_ = true;
        pOldNeighbour->update_burried_state_();
    }

    if (!pOldNeighbour)
        ++uiValidNeighbor_;
    else
        --uiValidNeighbor_;

    bUpdateCache_ = true;

    lNeighborList_[mAnchor] = pChunk;
    update_burried_state_();

    if (!pChunk)
        return;

    pChunk->lNeighborList_[mOpp] = pSelf_;
    pChunk->update_burried_state_();
    ++pChunk->uiValidNeighbor_;
    pChunk->bUpdateCache_ = true;
}

std::weak_ptr<const block_chunk> block_chunk::get_chunk(vector3i pos) const
{
    std::shared_ptr<block_chunk> pChunk = pSelf_.lock();

    while (!pos.is_null() && pChunk)
    {
        if (fabs(pos.x) >= fabs(pos.z) && fabs(pos.x) >= fabs(pos.y))
        {
            if (pos.x < 0)
            {
                pChunk = pChunk->lNeighborList_[LEFT].lock();
                ++pos.x;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[RIGHT].lock();
                --pos.x;
            }
        }
        else if (fabs(pos.z) >= fabs(pos.x) && fabs(pos.z) >= fabs(pos.y))
        {
            if (pos.z < 0)
            {
                pChunk = pChunk->lNeighborList_[FRONT].lock();
                ++pos.z;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[BACK].lock();
                --pos.z;
            }
        }
        else
        {
            if (pos.y < 0)
            {
                pChunk = pChunk->lNeighborList_[BOTTOM].lock();
                ++pos.y;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[TOP].lock();
                --pos.y;
            }
        }
    }

    return pChunk;
}

std::weak_ptr<block_chunk> block_chunk::get_chunk(vector3i pos)
{
    std::shared_ptr<block_chunk> pChunk = pSelf_.lock();

    while (!pos.is_null() && pChunk)
    {
        if (fabs(pos.x) >= fabs(pos.z) && fabs(pos.x) >= fabs(pos.y))
        {
            if (pos.x < 0)
            {
                pChunk = pChunk->lNeighborList_[LEFT].lock();
                ++pos.x;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[RIGHT].lock();
                --pos.x;
            }
        }
        else if (fabs(pos.z) >= fabs(pos.x) && fabs(pos.z) >= fabs(pos.y))
        {
            if (pos.z < 0)
            {
                pChunk = pChunk->lNeighborList_[FRONT].lock();
                ++pos.z;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[BACK].lock();
                --pos.z;
            }
        }
        else
        {
            if (pos.y < 0)
            {
                pChunk = pChunk->lNeighborList_[BOTTOM].lock();
                ++pos.y;
            }
            else
            {
                pChunk = pChunk->lNeighborList_[TOP].lock();
                --pos.y;
            }
        }
    }

    return pChunk;
}

std::weak_ptr<const block_chunk> block_chunk::get_chunk(anchor mAnchor) const
{
    return lNeighborList_[mAnchor];
}

std::weak_ptr<block_chunk> block_chunk::get_chunk(anchor mAnchor)
{
    return lNeighborList_[mAnchor];
}

std::pair<std::weak_ptr<const block_chunk>,const block*> block_chunk::get_neighbor_block_(block_chunk::anchor mAnchor, size_t id) const
{
    typedef std::pair<std::weak_ptr<const block_chunk>,const block*> pair;
    std::shared_ptr<const block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (pChunk)
        return pair(pChunk, &pChunk->lBlockList_[id]);
    else
        return pair(std::weak_ptr<const block_chunk>(), nullptr);
}

std::pair<std::weak_ptr<const block_chunk>,const block*> block_chunk::get_block(block::face mFace, const block* pBlock) const
{
    static const int half = HALF_CHUNK_SIZE;
    static const int half2 = 2.0*half;
    static const int size = CHUNK_SIZE;
    static const int size2 = size*size;

    typedef std::pair<std::weak_ptr<const block_chunk>,const block*> pair;

    vector3i pos = get_block_position(pBlock);

    switch (mFace)
    {
        case block::LEFT :
            if (pos.x == -half)
                return get_neighbor_block_(LEFT, half2 + (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock - 1);

        case block::RIGHT :
            if (pos.x == half)
                return get_neighbor_block_(RIGHT, (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock + 1);

        case block::FRONT :
            if (pos.z == -half)
                return get_neighbor_block_(FRONT, (pos.x+half) + (pos.y+half)*size + half2*size2);
            else
                return pair(pSelf_, pBlock - size2);

        case block::BACK :
            if (pos.z == half)
                return get_neighbor_block_(BACK, (pos.x+half) + (pos.y+half)*size);
            else
                return pair(pSelf_, pBlock + size2);

        case block::TOP :
            if (pos.y == half)
                return get_neighbor_block_(TOP, (pos.x+half) + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock + size);

        case block::BOTTOM :
            if (pos.y == -half)
                return get_neighbor_block_(BOTTOM, (pos.x+half) + half2*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock - size);

        default : return pair(std::weak_ptr<const block_chunk>(), nullptr);
    }
}

std::pair<std::weak_ptr<block_chunk>, block*> block_chunk::get_neighbor_block_(block_chunk::anchor mAnchor, size_t id)
{
    typedef std::pair<std::weak_ptr<block_chunk>, block*> pair;
    std::shared_ptr<block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (pChunk)
        return pair(pChunk, &pChunk->lBlockList_[id]);
    else
        return pair(std::weak_ptr<block_chunk>(), nullptr);
}

std::pair<std::weak_ptr<block_chunk>,block*> block_chunk::get_block(block::face mFace, block* pBlock)
{
    static const int half = HALF_CHUNK_SIZE;
    static const int half2 = 2.0*half;
    static const int size = CHUNK_SIZE;
    static const int size2 = size*size;

    typedef std::pair<std::weak_ptr<block_chunk>,block*> pair;

    vector3i pos = get_block_position(pBlock);

    switch (mFace)
    {
        case block::LEFT :
            if (pos.x == -half)
                return get_neighbor_block_(LEFT, half2 + (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock - 1);

        case block::RIGHT :
            if (pos.x == half)
                return get_neighbor_block_(RIGHT, (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock + 1);

        case block::FRONT :
            if (pos.z == -half)
                return get_neighbor_block_(FRONT, (pos.x+half) + (pos.y+half)*size + half2*size2);
            else
                return pair(pSelf_, pBlock - size2);

        case block::BACK :
            if (pos.z == half)
                return get_neighbor_block_(BACK, (pos.x+half) + (pos.y+half)*size);
            else
                return pair(pSelf_, pBlock + size2);

        case block::TOP :
            if (pos.y == half)
                return get_neighbor_block_(TOP, (pos.x+half) + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock + size);

        case block::BOTTOM :
            if (pos.y == -half)
                return get_neighbor_block_(BOTTOM, (pos.x+half) + half2*size + (pos.z+half)*size2);
            else
                return pair(pSelf_, pBlock - size);

        default : return pair(std::weak_ptr<block_chunk>(), nullptr);
    }
}

const block* block_chunk::get_neighbor_block__(block_chunk::anchor mAnchor, size_t id) const
{
    std::shared_ptr<const block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (pChunk)
        return &pChunk->lBlockList_[id];
    else
        return nullptr;
}

const block* block_chunk::get_block_(block::face mFace, block* pBlock, const vector3i& pos) const
{
    static const int half = HALF_CHUNK_SIZE;
    static const int half2 = 2.0*half;
    static const int size = CHUNK_SIZE;
    static const int size2 = size*size;

    switch (mFace)
    {
        case block::LEFT :
            if (pos.x == -half)
                get_neighbor_block__(LEFT, half2 + (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pBlock - 1;

        case block::RIGHT :
            if (pos.x == half)
                get_neighbor_block__(RIGHT, (pos.y+half)*size + (pos.z+half)*size2);
            else
                return pBlock + 1;

        case block::FRONT :
            if (pos.z == -half)
                get_neighbor_block__(FRONT, (pos.x+half) + (pos.y+half)*size + half2*size2);
            else
                return pBlock - size2;

        case block::BACK :
            if (pos.z == half)
                get_neighbor_block__(BACK, (pos.x+half) + (pos.y+half)*size);
            else
                return pBlock + size2;

        case block::TOP :
            if (pos.y == half)
                get_neighbor_block__(TOP, (pos.x+half) + (pos.z+half)*size2);
            else
                return pBlock + size;

        case block::BOTTOM :
            if (pos.y == -half)
                get_neighbor_block__(BOTTOM, (pos.x+half) + half2*size + (pos.z+half)*size2);
            else
                return pBlock - size;

        default : return nullptr;
    }
}

void block_chunk::get_visible_faces(const block& mBlock, const vector3i& pos, const block* lFaces[]) const
{
    static const int half = HALF_CHUNK_SIZE;
    static const int half2 = 2.0*half;
    static const int size = CHUNK_SIZE;
    static const int size2 = size*size;

    if (pos.x == -half)
        lFaces[LEFT] = get_neighbor_block__(LEFT, half2 + (pos.y+half)*size + (pos.z+half)*size2);
    else
        lFaces[LEFT] = &mBlock - 1;

    if (pos.x == +half)
        lFaces[RIGHT] = get_neighbor_block__(RIGHT, (pos.y+half)*size + (pos.z+half)*size2);
    else
        lFaces[RIGHT] = &mBlock + 1;

    if (pos.y == -half)
        lFaces[BOTTOM] = get_neighbor_block__(BOTTOM, (pos.x+half) + half2*size + (pos.z+half)*size2);
    else
        lFaces[BOTTOM] = &mBlock - size;

    if (pos.y == +half)
        lFaces[TOP] = get_neighbor_block__(TOP, (pos.x+half) + (pos.z+half)*size2);
    else
        lFaces[TOP] = &mBlock + size;

    if (pos.z == -half)
        lFaces[FRONT] = get_neighbor_block__(FRONT, (pos.x+half) + (pos.y+half)*size + half2*size2);
    else
        lFaces[FRONT] = &mBlock - size2;

    if (pos.z == +half)
        lFaces[BACK] = get_neighbor_block__(BACK, (pos.x+half) + (pos.y+half)*size);
    else
        lFaces[BACK] = &mBlock + size2;

    for (uint i = 0; i < 6; ++i)
    {
        if (!lFaces[i] || !lFaces[i]->is_transparent() || (mBlock.t == lFaces[i]->t && mBlock.t != block::LEAVES))
            lFaces[i] = nullptr;
    }
}

void block_chunk::get_nearby_blocks(const block* pBlock, block::corner mCorner, std::array<const block*,8>& lArray) const
{
    std::pair<std::weak_ptr<const block_chunk>, const block*> mBlock1;
    std::pair<std::weak_ptr<const block_chunk>, const block*> mBlock2;
    std::pair<std::weak_ptr<const block_chunk>, const block*> mBlock3;

    mBlock1 = get_block(block::OCCLUSION_LIST1[mCorner][0], pBlock);
    mBlock2 = get_block(block::OCCLUSION_LIST1[mCorner][1], pBlock);
    mBlock3 = get_block(block::OCCLUSION_LIST1[mCorner][2], pBlock);

    lArray[block::OCCLUSION_LIST2[mCorner][0]] = pBlock;

    lArray[block::OCCLUSION_LIST2[mCorner][1]] = mBlock1.second;
    lArray[block::OCCLUSION_LIST2[mCorner][2]] = mBlock2.second;
    lArray[block::OCCLUSION_LIST2[mCorner][3]] = mBlock3.second;

    if (std::shared_ptr<const block_chunk> p = mBlock1.first.lock())
        mBlock1 = p->get_block(block::OCCLUSION_LIST1[mCorner][3], mBlock1.second);
    if (std::shared_ptr<const block_chunk> p = mBlock2.first.lock())
        mBlock2 = p->get_block(block::OCCLUSION_LIST1[mCorner][4], mBlock2.second);
    if (std::shared_ptr<const block_chunk> p = mBlock3.first.lock())
        mBlock3 = p->get_block(block::OCCLUSION_LIST1[mCorner][5], mBlock3.second);

    lArray[block::OCCLUSION_LIST2[mCorner][4]] = mBlock1.second;
    lArray[block::OCCLUSION_LIST2[mCorner][5]] = mBlock2.second;
    lArray[block::OCCLUSION_LIST2[mCorner][6]] = mBlock3.second;

    if (std::shared_ptr<const block_chunk> p = mBlock1.first.lock())
        mBlock1 = p->get_block(block::OCCLUSION_LIST1[mCorner][6], mBlock1.second);

    lArray[block::OCCLUSION_LIST2[mCorner][7]] = mBlock1.second;
}

void block_chunk::add_transparent_block_(block_chunk::anchor mAnchor)
{
    ++lFaceTranspBlockCount_[mAnchor];
    std::shared_ptr<block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (lFaceTranspBlockCount_[mAnchor] == 1u && pChunk)
        pChunk->update_burried_state_();
}

void block_chunk::remove_transparent_block_(block_chunk::anchor mAnchor)
{
    --lFaceTranspBlockCount_[mAnchor];
    std::shared_ptr<block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (lFaceTranspBlockCount_[mAnchor] == 0u && pChunk)
        pChunk->update_burried_state_();
}

void block_chunk::set_block(const vector3i& pos, block::type mType)
{
    set_block(get_block(pos), mType);
}

void block_chunk::set_block(block* pBlock, block::type mType)
{
    static const int half = HALF_CHUNK_SIZE;

    if (pBlock)
    {
        if (pBlock->t != mType)
        {
            block::type mOldType = (block::type)pBlock->t;
            pBlock->t = mType;

            uint uiOldblockCount = uiPlainBlockCount_;

            if (mOldType == block::EMPTY)
                ++uiPlainBlockCount_;
            if (mType == block::EMPTY)
                --uiPlainBlockCount_;

            if (uiPlainBlockCount_ == 0u || uiOldblockCount == 0u)
                pWorld_->flag_update_visible_chunk_list();

            if (block::is_walkable(mType) != block::is_walkable(mOldType))
                flag_update_collision_faces_(pBlock);

            if (block::is_transparent(mType) != block::is_transparent(mOldType))
            {
                flag_mesh_update(pBlock);
                //UpdateblockLight(pBlock);
                for (uint i = 0; i < 6; ++i)
                {
                    std::shared_ptr<block_chunk> pChunk = lNeighborList_[i].lock();
                    if (pChunk)
                        pChunk->flag_update_collision_faces_(pBlock);
                }

                vector3i pos = get_block_position(pBlock);

                if (block::is_transparent(mType))
                {
                    if (pos.x == -half)
                        add_transparent_block_(LEFT);
                    else if (pos.x == half)
                        add_transparent_block_(RIGHT);

                    if (pos.y == -half)
                        add_transparent_block_(BOTTOM);
                    else if (pos.y == half)
                        add_transparent_block_(TOP);

                    if (pos.z == -half)
                        add_transparent_block_(FRONT);
                    else if (pos.z == half)
                        add_transparent_block_(BACK);
                }
                else
                {
                    if (pos.x == -half)
                        remove_transparent_block_(LEFT);
                    else if (pos.x == half)
                        remove_transparent_block_(RIGHT);

                    if (pos.y == -half)
                        remove_transparent_block_(BOTTOM);
                    else if (pos.y == half)
                        remove_transparent_block_(TOP);

                    if (pos.z == -half)
                        remove_transparent_block_(FRONT);
                    else if (pos.z == half)
                        remove_transparent_block_(BACK);
                }
            }
            else
            {
                if (mOldType == block::EMPTY || mType == block::EMPTY)
                    flag_mesh_update(pBlock);
                else
                    update_texture(pBlock);
            }
        }
    }
}

void block_chunk::set_block_fast(const vector3i& pos, block::type mType)
{
    static const int half = HALF_CHUNK_SIZE;

    block* pBlock = get_block(pos);
    block::type mOldType = (block::type)pBlock->t;

    if (mOldType == mType)
        return;

    pBlock->t = mType;

    if (mOldType == block::EMPTY)
        ++uiPlainBlockCount_;
    if (mType == block::EMPTY)
        --uiPlainBlockCount_;

    if (block::is_transparent(mType))
    {
        if (pos.x == -half)
            ++lFaceTranspBlockCount_[LEFT];
        else if (pos.x == half)
            ++lFaceTranspBlockCount_[RIGHT];

        if (pos.y == -half)
            ++lFaceTranspBlockCount_[BOTTOM];
        else if (pos.y == half)
            ++lFaceTranspBlockCount_[TOP];

        if (pos.z == -half)
            ++lFaceTranspBlockCount_[FRONT];
        else if (pos.z == half)
            ++lFaceTranspBlockCount_[BACK];
    }
    else if (block::is_transparent(mOldType))
    {
        if (pos.x == -half)
            --lFaceTranspBlockCount_[LEFT];
        else if (pos.x == half)
            --lFaceTranspBlockCount_[RIGHT];

        if (pos.y == -half)
            --lFaceTranspBlockCount_[BOTTOM];
        else if (pos.y == half)
            --lFaceTranspBlockCount_[TOP];

        if (pos.z == -half)
            --lFaceTranspBlockCount_[FRONT];
        else if (pos.z == half)
            --lFaceTranspBlockCount_[BACK];
    }
}

void block_chunk::set_block_fast(block* pBlock, block::type mType)
{
    static const int half = HALF_CHUNK_SIZE;

    block::type mOldType = (block::type)pBlock->t;

    if (mOldType == mType)
        return;

    pBlock->t = mType;

    if (mOldType == block::EMPTY)
        ++uiPlainBlockCount_;
    if (mType == block::EMPTY)
        --uiPlainBlockCount_;

    if (block::is_transparent(mType))
    {
        vector3i pos = get_block_position(pBlock);

        if (pos.x == -half)
            ++lFaceTranspBlockCount_[LEFT];
        else if (pos.x == half)
            ++lFaceTranspBlockCount_[RIGHT];

        if (pos.y == -half)
            ++lFaceTranspBlockCount_[BOTTOM];
        else if (pos.y == half)
            ++lFaceTranspBlockCount_[TOP];

        if (pos.z == -half)
            ++lFaceTranspBlockCount_[FRONT];
        else if (pos.z == half)
            ++lFaceTranspBlockCount_[BACK];
    }
    else if (block::is_transparent(mOldType))
    {
        vector3i pos = get_block_position(pBlock);

        if (pos.x == -half)
            --lFaceTranspBlockCount_[LEFT];
        else if (pos.x == half)
            --lFaceTranspBlockCount_[RIGHT];

        if (pos.y == -half)
            --lFaceTranspBlockCount_[BOTTOM];
        else if (pos.y == half)
            --lFaceTranspBlockCount_[TOP];

        if (pos.z == -half)
            --lFaceTranspBlockCount_[FRONT];
        else if (pos.z == half)
            --lFaceTranspBlockCount_[BACK];
    }
}

void block_chunk::flag_update_neighbor_cache_(block_chunk::anchor mAnchor)
{
    std::shared_ptr<block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    if (pChunk)
        pChunk->bUpdateCache_ = true;
}

void block_chunk::flag_mesh_update(const block* pBlock)
{
    bUpdateCache_ = true;

    vector3i pos = get_block_position(pBlock);

    if (pos.x ==  HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(RIGHT);
    if (pos.x == -HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(LEFT);

    if (pos.y ==  HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(TOP);
    if (pos.y == -HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(BOTTOM);

    if (pos.z ==  HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(BACK);
    if (pos.z == -HALF_CHUNK_SIZE)
        flag_update_neighbor_cache_(FRONT);
}

void block_chunk::update_texture(block* pBlock)
{
    if (bUpdateCache_)
        return;

    /*#ifdef PRINT_UPDATES
    Log("update_texture : "+get_position()+", "+get_block_position(pBlock));
    #endif*/

    std::vector<block_face>::iterator iter;
    foreach (iter, mVertexCache_.lData)
    {
        block_face& mFace = *iter;

        if (mFace.b == pBlock)
        {
            block::texture_id mTex = pBlock->get_texture(mFace.face);

            mFace.hue = 0;
            if (mFace.face == block::TOP && pBlock->t == block::GRASS)
                mFace.hue = 1;
            if (pBlock->t == block::LEAVES)
                mFace.hue = 1;

            switch (mFace.face)
            {
                case block::LEFT :
                    mFace.v1.uv = block::get_uv(mTex, 3);
                    mFace.v2.uv = block::get_uv(mTex, 0);
                    mFace.v3.uv = block::get_uv(mTex, 1);
                    mFace.v4.uv = block::get_uv(mTex, 2);
                    break;

                case block::RIGHT :
                    mFace.v1.uv = block::get_uv(mTex, 2);
                    mFace.v2.uv = block::get_uv(mTex, 3);
                    mFace.v3.uv = block::get_uv(mTex, 0);
                    mFace.v4.uv = block::get_uv(mTex, 1);
                    break;

                case block::FRONT :
                    mFace.v1.uv = block::get_uv(mTex, 2);
                    mFace.v2.uv = block::get_uv(mTex, 3);
                    mFace.v3.uv = block::get_uv(mTex, 0);
                    mFace.v4.uv = block::get_uv(mTex, 1);
                    break;

                case block::BACK :
                    mFace.v1.uv = block::get_uv(mTex, 3);
                    mFace.v2.uv = block::get_uv(mTex, 0);
                    mFace.v3.uv = block::get_uv(mTex, 1);
                    mFace.v4.uv = block::get_uv(mTex, 2);
                    break;

                case block::TOP :
                    mFace.v1.uv = block::get_uv(mTex, 0);
                    mFace.v2.uv = block::get_uv(mTex, 1);
                    mFace.v3.uv = block::get_uv(mTex, 2);
                    mFace.v4.uv = block::get_uv(mTex, 3);
                    break;

                case block::BOTTOM :
                    mFace.v1.uv = block::get_uv(mTex, 0);
                    mFace.v2.uv = block::get_uv(mTex, 1);
                    mFace.v3.uv = block::get_uv(mTex, 2);
                    mFace.v4.uv = block::get_uv(mTex, 3);
                    break;

                default : break;
            }
        }
    }

    bUpdateVBO_ = true;
}

bool block_chunk::is_normal_vertex_cache_empty() const
{
    return mVertexCache_.uiNumNormalVertex == 0u;
}

bool block_chunk::is_two_sided_vertex_cache_empty() const
{
    return mVertexCache_.uiNumTwoSidedVertex == 0u;
}

bool block_chunk::is_alpha_blended_vertex_cache_empty() const
{
    return mVertexCache_.uiNumAlphaBlendedVertex == 0u;
}

std::vector<block_face> block_chunk::get_cached_faces(const block* pBlock) const
{
    if (bUpdateCache_)
        pWorld_->update_chunk_immediate(pSelf_);

    std::vector<block_face> lArray;
    for (block_face& face : mVertexCache_.lData)
    {
        if (face.b == pBlock)
            lArray.push_back(face);
    }

    return lArray;
}

std::vector<block_face> block_chunk::get_old_cached_faces(const block* pBlock) const
{
    std::vector<block_face> lArray;
    for (block_face& face : mVertexCache_.lData)
    {
        if (face.b == pBlock)
            lArray.push_back(face);
    }

    return lArray;
}

void block_chunk::get_cached_world_faces(const block* pBlock, std::vector<block_face>& lArray) const
{
    if (bUpdateCache_)
        pWorld_->update_chunk_immediate(pSelf_);

    std::shared_ptr<const block_chunk> pCurrentChunk = pWorld_->get_current_chunk().lock();

    bool bCurrent = (pSelf_.lock() == pCurrentChunk);
    vector3f mPosition;
    if (!bCurrent && pWorld_->are_vbos_enabled())
        mPosition = get_position(pCurrentChunk);

    for (block_face& face : mVertexCache_.lData)
    {
        if (face.b == pBlock)
        {
            block_face mFace = face;
            if (!bCurrent && pWorld_->are_vbos_enabled())
            {
                vertex* v = &mFace.v1;
                for (uint i = 0; i < 4; ++i)
                    v[i].pos += mPosition;
            }
            lArray.push_back(mFace);
        }
    }
}

void block_chunk::update_cache() const
{
    update_cache(mVertexCache_);
}

void block_chunk::update_cache(vertex_cache& mVertexCache) const
{
    static const int half = HALF_CHUNK_SIZE;
    static const int size = CHUNK_SIZE;

    mVertexCache.lData.clear();
    mVertexCache.uiNumNormalVertex = 0u;
    mVertexCache.uiNumTwoSidedVertex = 0u;
    mVertexCache.uiNumAlphaBlendedVertex = 0u;

    if (uiPlainBlockCount_ == 0u || bBurried_)
    {
        bUpdateCache_ = false;
        return;
    }

    std::vector<block_face> lTwoSidedVertexCache;
    std::vector<block_face> lAlphaBlendedVertexCache;

    #ifdef PRINT_UPDATES
    std::cout << "update_cache : " << get_coordinates() << std::endl;
    #endif

    const block* lFaces[6];

    for (int k = -half; k <= half; ++k)
    for (int j = -half; j <= half; ++j)
    for (int i = -half; i <= half; ++i)
    {
        const block& mBlock = lBlockList_[i+half + (j+half)*size + (k+half)*size*size];

        if (mBlock.t == block::EMPTY)
            continue;

        vector3i mBlockPos = vector3i(i, j, k);

        get_visible_faces(mBlock, mBlockPos, lFaces);

        for (uint f = 0; f < 6; ++f)
        {
            if (!lFaces[f])
                continue;

            block::face face = (block::face)f;
            int tex = mBlock.get_texture(face);

            if (tex < 0 || tex > 255)
                continue;

            uchar hue = 0;
            if (face == block::TOP && mBlock.t == block::GRASS)
                hue = 127;
            if (mBlock.t == block::LEAVES)
                hue = 127;

            if (mBlock.is_alpha_blended())
            {
                mBlock.add_face(vector3f(mBlockPos), face, lFaces[f], hue, lAlphaBlendedVertexCache);
                ++mVertexCache.uiNumAlphaBlendedVertex;
            }
            else if (mBlock.is_two_sided())
            {
                mBlock.add_face(vector3f(mBlockPos), face, lFaces[f], hue, lTwoSidedVertexCache);
                ++mVertexCache.uiNumTwoSidedVertex;
            }
            else
            {
                mBlock.add_face(vector3f(mBlockPos), face, lFaces[f], hue, mVertexCache.lData);
                ++mVertexCache.uiNumNormalVertex;
            }
        }
    }

    optimize_cache_(mVertexCache);

    bUpdateCache_ = false;
    //bUpdateLighting_ = false;

    bBuildOcclusion_ = true;
    bUpdateVBO_ = true;

    for (auto& data : lTwoSidedVertexCache)
        mVertexCache.lData.push_back(data);

    for (auto& data : lAlphaBlendedVertexCache)
        mVertexCache.lData.push_back(data);
}

void block_chunk::optimize_cache_(vertex_cache& mVertexCache) const
{

}

void block_chunk::set_normal_vbo_data(utils::refptr<vbo_data> pData) const
{
    if (bVBOCleared_)
        return;

    pNormalVBO_ = nullptr;

    if (!pData || pData->uiNum == 0u)
        return;

    if (pNormalVBO_)
        pNormalVBO_->set_data(*pData);
    else
        pNormalVBO_ = utils::refptr<vertex_buffer_object>(new vertex_buffer_object(*pData));
}

void block_chunk::set_two_sided_vbo_data(utils::refptr<vbo_data> pData) const
{
    if (bVBOCleared_)
        return;

    pTwoSidedVBO_ = nullptr;

    if (!pData || pData->uiNum == 0u)
        return;

    if (pTwoSidedVBO_)
        pTwoSidedVBO_->set_data(*pData);
    else
        pTwoSidedVBO_ = utils::refptr<vertex_buffer_object>(new vertex_buffer_object(*pData));
}

void block_chunk::set_alpha_blended_vbo_data(utils::refptr<vbo_data> pData) const
{
    if (bVBOCleared_)
        return;

    pAlphaBlendedVBO_ = nullptr;

    if (!pData || pData->uiNum == 0u)
        return;

    if (pAlphaBlendedVBO_)
        pAlphaBlendedVBO_->set_data(*pData);
    else
        pAlphaBlendedVBO_ = utils::refptr<vertex_buffer_object>(new vertex_buffer_object(*pData));
}

void block_chunk::move_vertex_cache(const vector3f& mDiff, std::vector<block_face>& lVertexCache) const
{
    if (bUpdateCache_)
        return;

    for (block_face& face : lVertexCache)
    {
        face.v1.pos -= mDiff;
        face.v2.pos -= mDiff;
        face.v3.pos -= mDiff;
        face.v4.pos -= mDiff;
    }
}

void block_chunk::move_vertex_cache(const vector3f& mDiff) const
{
    move_vertex_cache(mDiff, mVertexCache_.lData);
}

vector3f block_chunk::get_difference(std::weak_ptr<block_chunk> pFrom, std::weak_ptr<block_chunk> pTo)
{
    return CHUNK_SIZE*(pTo.lock()->mCoordinates_ - pFrom.lock()->mCoordinates_);
}

uint block_chunk::count_open_blocks_() const
{
    uint uiCount = 0;

    for (const block& b : lBlockList_)
        if (b.open) ++uiCount;

    return uiCount;
}

bool block_chunk::cast_ray(const ray& mRay, block_collision_data& mData)
{
    mData.pChunk = pSelf_.lock();
    mData.fDist = std::numeric_limits<float>::infinity();

    axis_aligned_box mBox(
        vector3f(-0.5f, -0.5f, -0.5f),
        vector3f( 0.5f,  0.5f,  0.5f)
    );

    ray mTempRay = mRay;
    vector3f mIntersection;
    axis_aligned_box::face mFace;

    for (block& b : lBlockList_)
    {
        if (b.t == block::EMPTY || b.t == block::WATER)
            continue;

        mTempRay.mOrigin = mRay.mOrigin - vector3f(get_block_position(&b));

        if (!mBox.find_ray_intersection(mTempRay, mIntersection, mFace))
            continue;

        float fTempDist = (mIntersection - mTempRay.mOrigin).get_norm();
        if (!mData.pBlock || fTempDist < mData.fDist)
        {
            mData.pBlock = &b;
            mData.mFace = (block::face)(int)mFace;
            mData.fDist = fTempDist;
        }
    }

    return mData.pBlock != nullptr;
}

axis_aligned_box block_chunk::get_bounding_box() const
{
    return BOUNDING_BOX + get_position(pWorld_->get_current_chunk());
}

void block_chunk::flag_update_collision_faces_(const block* pBlock)
{
    /*for (auto& unit : lUnitList_)
        unit->FlagUpdateCollisionFaces(pBlock);*/
}

/*std::vector<std::pair<std::weak_ptr<block_chunk>,block*>> block_chunk::get_surrounding_blocks(const cylinder& mCylinder)
{
    std::vector<std::pair<std::weak_ptr<block_chunk>,block*>> lblocks, lOpenList, lNextOpenList;
    std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>::iterator iter;
    std::pair<std::weak_ptr<block_chunk>,block*> mNextblock;

    float fHSize = 2.0f*mCylinder.radius;
    float fVSize = mCylinder.height;

    uint   hsize = float::RoundUp(fHSize)+1;
    uint   vsize = float::RoundUp(fVSize)+1;
    vector3f mSize = vector3f(hsize, vsize, hsize);

    vector3f mPos = mCylinder.pos;
    mPos -= get_position(pWorld_->get_current_chunk());

    int x0 = float::Round(mPos.X());
    int y0 = float::Round(mPos.Y());
    int z0 = float::Round(mPos.Z());

    mPos = vector3f(x0, y0, z0) + get_position(pWorld_->get_current_chunk());

    axis_aligned_box mABox(mPos - mSize, mPos + mSize);

    block* pBlock = get_block(x0, y0, z0);
    lblocks.push_back(MakePair(pSelf_, pBlock));
    lOpenList.push_back(MakePair(pSelf_, pBlock));
    pBlock->open = true;

    while (!lOpenList.is_empty())
    {
        foreach (iter, lOpenList)
        {
            for (int i = 0; i < 6; ++i)
            {
                mNextblock = iter->first->get_block((block::face)i, iter->second);
                if (mNextblock.second && !mNextblock.second->open)
                {
                    vector3f mBlockPos = mNextblock.first->get_block_world_position(mNextblock.second);
                    if (mABox.Contains(mBlockPos))
                    {
                        lNextOpenList.push_back(mNextblock);
                        lblocks.push_back(mNextblock);
                        mNextblock.second->open = true;
                    }
                }
            }
        }

        lOpenList.Clear();

        foreach (iter, lNextOpenList)
        {
            for (int i = 0; i < 6; ++i)
            {
                mNextblock = iter->first->get_block((block::face)i, iter->second);
                if (mNextblock.second && !mNextblock.second->open)
                {
                    vector3f mBlockPos = mNextblock.first->get_block_position(mNextblock.second) +
                        mNextblock.first->get_position(pWorld_->get_current_chunk());

                    if (mABox.Contains(mBlockPos))
                    {
                        lOpenList.push_back(mNextblock);
                        lblocks.push_back(mNextblock);
                        mNextblock.second->open = true;
                    }
                }
            }
        }

        lNextOpenList.Clear();
    }

    iter = lblocks.Begin();
    while (iter != lblocks.End())
    {
        iter->second->open = false;
        if (iter->second->is_walkable())
            iter = lblocks.erase(iter);
        else
            ++iter;
    }

    return lblocks;
}*/

void block_chunk::add_unit(utils::refptr<unit> pUnit)
{
    lUnitList_[pUnit->get_name()] = pUnit;
}

void block_chunk::remove_unit(utils::wptr<unit> pUnit)
{
    lUnitList_.erase(pUnit->get_name());
}

utils::wptr<unit> block_chunk::get_unit(const utils::ustring& sName)
{
    std::map<utils::ustring, utils::refptr<unit>>::iterator iter = lUnitList_.find(sName);
    if (iter != lUnitList_.end())
        return iter->second;
    else
        return nullptr;
}

utils::wptr<const unit> block_chunk::get_unit(const utils::ustring& sName) const
{
    std::map<utils::ustring, utils::refptr<unit>>::const_iterator iter = lUnitList_.find(sName);
    if (iter != lUnitList_.end())
        return iter->second;
    else
        return nullptr;
}

bool block_chunk::is_neighbor_face_open_(block_chunk::anchor mAnchor) const
{
    std::shared_ptr<block_chunk> pChunk = lNeighborList_[mAnchor].lock();
    return (pChunk && pChunk->lFaceTranspBlockCount_[get_opposed(mAnchor)] != 0u);
}

void block_chunk::update_burried_state_() const
{
    bool bOld = bBurried_;
    if (is_neighbor_face_open_(LEFT) || is_neighbor_face_open_(RIGHT) ||
        is_neighbor_face_open_(FRONT) || is_neighbor_face_open_(BACK) ||
        is_neighbor_face_open_(TOP) || is_neighbor_face_open_(BOTTOM))
        bBurried_ = false;
    else
        bBurried_ = true;

    if (bOld != bBurried_)
    {
        bUpdateCache_ = true;
        pWorld_->flag_update_visible_chunk_list();
    }
}

utils::refptr<vbo_data> block_chunk::make_vbo_data_(const std::vector<block_face>& lVertexCache, uint uiOffset, uint uiNum) const
{
    utils::refptr<vbo_data> pData;

    uint uiNumFace;
    if (uiNum == uint(-1))
        uiNumFace = lVertexCache.size();
    else
        uiNumFace = uiNum;

    uint uiNumVertex = 0u;

    if (pWorld_->are_shaders_enabled())
    {
        if (!pWorld_->is_smooth_lighting_enabled())
        {
            pData = utils::refptr<vbo_data>(new vbo_data(4u*uiNumFace, vbo_vertex_3::TYPE));
            vbo_vertex_3* lArray = reinterpret_cast<vbo_vertex_3*>(pData->pData);

            for (uint i = 0; i < uiNumFace; ++i)
            {
                const block_face& mFace = lVertexCache[i+uiOffset];

                color mColor = block::HUE_TABLE[mFace.hue];

                const vertex* v = &mFace.v1;
                for (uint j = 0; j < 4; ++j)
                {
                    lArray[uiNumVertex+j].u   = v[j].uv.u;
                    lArray[uiNumVertex+j].v   = v[j].uv.v;
                    lArray[uiNumVertex+j].s   = mFace.sunlight/256.0f;
                    lArray[uiNumVertex+j].l   = mFace.light/256.0f;
                    lArray[uiNumVertex+j].r   = mColor.r;
                    lArray[uiNumVertex+j].g   = mColor.g;
                    lArray[uiNumVertex+j].b   = mColor.b;
                    lArray[uiNumVertex+j].pos = v[j].pos;
                }

                uiNumVertex += 4u;
            }
        }
        else
        {
            pData = utils::refptr<vbo_data>(new vbo_data(4u*uiNumFace, vbo_vertex_2::TYPE));
            vbo_vertex_2* lArray = reinterpret_cast<vbo_vertex_2*>(pData->pData);

            for (uint i = 0; i < uiNumFace; ++i)
            {
                const block_face& mFace = lVertexCache[i+uiOffset];

                color mColor = block::HUE_TABLE[mFace.hue];

                const vertex* v = &mFace.v1;
                for (uint j = 0; j < 4; ++j)
                {
                    lArray[uiNumVertex+j].u = v[j].uv.u;
                    lArray[uiNumVertex+j].v = v[j].uv.v;
                    lArray[uiNumVertex+j].s = v[j].sunlight/256.0f;
                    lArray[uiNumVertex+j].l = v[j].light/256.0f;
                    lArray[uiNumVertex+j].o = v[j].occlusion/256.0f;
                    lArray[uiNumVertex+j].r = mColor.r;
                    lArray[uiNumVertex+j].g = mColor.g;
                    lArray[uiNumVertex+j].b = mColor.b;
                    lArray[uiNumVertex+j].pos = v[j].pos;
                }

                uiNumVertex += 4u;
            }
        }
    }
    else
    {
        pData = utils::refptr<vbo_data>(new vbo_data(4u*uiNumFace, vbo_vertex_1::TYPE));
        vbo_vertex_1* lArray = reinterpret_cast<vbo_vertex_1*>(pData->pData);

        if (!pWorld_->is_smooth_lighting_enabled())
        {
            for (uint i = 0; i < uiNumFace; ++i)
            {
                const block_face& mFace = lVertexCache[i+uiOffset];

                color mColor = block::HUE_TABLE[mFace.hue];
                pWorld_->lighten_color(mColor, mFace.sunlight, mFace.light);

                const vertex* v = &mFace.v1;
                for (uint j = 0; j < 4; ++j)
                {
                    lArray[uiNumVertex+j].u   = v[j].uv.u;
                    lArray[uiNumVertex+j].v   = v[j].uv.v;
                    lArray[uiNumVertex+j].r   = mColor.r;
                    lArray[uiNumVertex+j].g   = mColor.g;
                    lArray[uiNumVertex+j].b   = mColor.b;
                    lArray[uiNumVertex+j].pos = v[j].pos;
                }

                uiNumVertex += 4u;
            }
        }
        else
        {
            for (uint i = 0; i < uiNumFace; ++i)
            {
                const block_face& mFace = lVertexCache[i+uiOffset];

                color mColor = block::HUE_TABLE[mFace.hue];
                float fOcclusion = pWorld_->get_occlusion(mFace.sunlight, mFace.light);

                const vertex* v = &mFace.v1;
                for (uint j = 0; j < 4; ++j)
                {
                    lArray[uiNumVertex+j].u = v[j].uv.u;
                    lArray[uiNumVertex+j].v = v[j].uv.v;

                    color mTempColor = mColor;
                    pWorld_->lighten_color(mTempColor, v[j].sunlight, v[j].light);

                    if (v[j].occlusion == 0u)
                    {
                        lArray[uiNumVertex+j].r = mTempColor.r;
                        lArray[uiNumVertex+j].g = mTempColor.g;
                        lArray[uiNumVertex+j].b = mTempColor.b;
                    }
                    else
                    {
                        float o = 1.0f + (block::OCCLUSION_TABLE1[v[j].occlusion]-1.0f)*fOcclusion;
                        lArray[uiNumVertex+j].r = mTempColor.r*o;
                        lArray[uiNumVertex+j].g = mTempColor.g*o;
                        lArray[uiNumVertex+j].b = mTempColor.b*o;
                    }

                    lArray[uiNumVertex+j].pos = v[j].pos;
                }

                uiNumVertex += 4u;
            }
        }
    }

    pData->uiNum = uiNumVertex;

    return pData;
}

void block_chunk::updade_vbo_() const
{
    bUpdateVBO_ = false;

    utils::refptr<vbo_data> pNormalData;
    if (mVertexCache_.uiNumNormalVertex != 0u)
    {
        pNormalData = make_vbo_data_(mVertexCache_.lData,
            0, mVertexCache_.uiNumNormalVertex
        );
    }

    utils::refptr<vbo_data> pTwoSidedData;
    if (mVertexCache_.uiNumTwoSidedVertex != 0u)
    {
        pTwoSidedData = make_vbo_data_(mVertexCache_.lData,
            mVertexCache_.uiNumNormalVertex,
            mVertexCache_.uiNumTwoSidedVertex
        );
    }

    utils::refptr<vbo_data> pAlphaBlendedData;
    if (mVertexCache_.uiNumAlphaBlendedVertex != 0u)
    {
        pAlphaBlendedData = make_vbo_data_(mVertexCache_.lData,
            mVertexCache_.uiNumNormalVertex +
            mVertexCache_.uiNumTwoSidedVertex,
            mVertexCache_.uiNumAlphaBlendedVertex
        );
    }

    set_normal_vbo_data(pNormalData);
    set_two_sided_vbo_data(pTwoSidedData);
    set_alpha_blended_vbo_data(pAlphaBlendedData);
}

void block_chunk::build_occlusion() const
{
    build_occlusion(mVertexCache_.lData);
}

void block_chunk::build_occlusion(std::vector<block_face>& lVertexCache) const
{
    /*#ifdef PRINT_UPDATES
    std::cout << "build_occlusion : " << get_coordinates() << std::endl;
    /endif*/

    std::array<const block*,8> lArray;

    std::vector<block_face>::iterator iter;
    foreach (iter, lVertexCache)
    {
        block_face& mFace = *iter;
        vertex* pVertex = &(mFace.v1);
        for (int i = 0; i < 4; ++i)
        {
            block::corner mCorner = block::CORNER_LIST[mFace.face][i];
            pVertex[i].occlusion = 0u;
            get_nearby_blocks(mFace.b, mCorner, lArray);

            float sun = 0.0f;
            float light = 0.0f;
            float count = 0.0f;
            uchar altocc = 0u;

            for (int j = 0; j < 8; ++j)
            {
                if (lArray[j])
                {
                    uchar select = block::OCCLUSION_SELECT_LIST[mFace.face][i][j];
                    if (!lArray[j]->is_transparent())
                    {
                        if (select == 2)
                            pVertex[i].occlusion += 255u/2u;
                        else if (select == 3)
                            altocc = 127u;
                    }
                    else
                    {
                        if (select == 2 || select == 1)
                        {
                            count += 1.0f;
                            sun += lArray[j]->sunlight;
                            light += lArray[j]->light;
                        }
                    }
                }
            }

            pVertex[i].sunlight = sun/count;
            pVertex[i].light = light/count;
            if (pVertex[i].occlusion == 0u)
                pVertex[i].occlusion = altocc;
        }
    }

    bBuildOcclusion_ = false;
    bUpdateVBO_ = true;
}
