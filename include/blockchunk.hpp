#ifndef BLOCKCHUNK_H
#define BLOCKCHUNK_H

#include <lxgui/utils.hpp>
#include <map>
#include "color.hpp"
#include "axisalignedbox.hpp"
//#include "cylinder.h"
#include "block.hpp"
//#include "light.h"
#include "unit.hpp"
//#include "vector2.hpp"
#include "vertexbufferobject.hpp"

class  world;
class  block_chunk;
struct ray;

struct block_collision_data
{
    block*                       pBlock = nullptr;
    std::shared_ptr<block_chunk> pChunk;
    block::face                  mFace = block::FRONT;
    float                        fDist = 0.0;
};

struct vertex_cache
{
    std::vector<block_face> lData;
    uint                    uiNumNormalVertex = 0;
    uint                    uiNumTwoSidedVertex = 0;
    uint                    uiNumAlphaBlendedVertex = 0;
};

class block_chunk
{
friend class world;
public :

    enum anchor
    {
        LEFT = 0,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
    };

    block_chunk(utils::wptr<world> pWorld, const vector3i& mPos);
    ~block_chunk();

    void set_self(std::weak_ptr<block_chunk> pSelf);
    utils::wptr<const world> get_world() const;
    utils::wptr<world> get_world();

    void flag_clear_links() const;
    void clear_links() const;
    void clear_vbos();

    inline vector3f get_position(std::weak_ptr<const block_chunk> pRelativeTo) const {
        static const float size = CHUNK_SIZE;
        return size*(mCoordinates_ - pRelativeTo.lock()->mCoordinates_);
    }

    inline vector3f get_position() const {
        static const float size = CHUNK_SIZE;
        return size*mCoordinates_;
    }

    inline const vector3i& get_coordinates() const {
        return mCoordinates_;
    }

    vector3f get_block_world_position(const block* pBlock) const;
    vector3i get_block_position(const block* pBlock) const;

    bool is_empty() const;

    void attach_to(std::weak_ptr<block_chunk> pChunk, block_chunk::anchor mAnchor);

    std::weak_ptr<const block_chunk>        get_chunk(vector3i mPos) const;
    std::weak_ptr<block_chunk>              get_chunk(vector3i mPos);
    inline std::weak_ptr<const block_chunk> get_chunk(anchor mAnchor) const;
    inline std::weak_ptr<block_chunk>       get_chunk(anchor mAnchor);

    inline const block* get_block(const vector3i& mPos) const {
        static const int half = HALF_CHUNK_SIZE;
        static const int size = CHUNK_SIZE;
        static const int size2 = size*size;

        return &lBlockList_[0] +
            (mPos.x+half) +
            (mPos.y+half)*size +
            (mPos.z+half)*size2;
    }

    inline block* get_block(const vector3i& mPos) {
        static const int half = HALF_CHUNK_SIZE;
        static const int size = CHUNK_SIZE;
        static const int size2 = size*size;

        return &lBlockList_[0] +
            (mPos.x+half) +
            (mPos.y+half)*size +
            (mPos.z+half)*size2;
    }

    std::pair<std::weak_ptr<const block_chunk>,const block*> get_block(block::face mFace, const block* pBlock) const;
    std::pair<std::weak_ptr<block_chunk>,block*>             get_block(block::face mFace, block* pBlock);

    // std::vector<std::pair<std::weak_ptr<block_chunk>,block*>> get_surrounding_blocks(const cylinder& mCylinder);

    void set_block(const vector3i& mPos, block::type mType);
    void set_block(block* pBlock, block::type mType);
    void set_block_fast(const vector3i& mPos, block::type mType);
    void set_block_fast(block* pBlock, block::type mType);
    void remove_block(const vector3i& mPos);

    void get_visible_faces(const block& mBlock, const vector3i& mPos, const block* lFaces[]) const;
    void get_nearby_blocks(const block* pBlock, block::corner mCorner, std::array<const block*,8>& lArray) const;

    bool cast_ray(const ray& mRay, block_collision_data& mData);

    void flag_mesh_update(const block* pBlock);
    // void flag_light_update(block* pBlock);
    // void flag_light_update();
    void update_texture(block* pBlock);

    // void                AddLight(utils::refptr<Light> pLight);
    // void                AddLightDelayed(utils::refptr<Light> pLight);
    // void                RemoveLight(utils::wptr<Light> pLight);
    // void                RemoveLight(const std::string& sName);
    // utils::wptr<Light>       GetLight(const std::string& sName);
    // utils::wptr<const Light> GetLight(const std::string& sName) const;
    // void                MoveLight(utils::wptr<Light> pLight, block* pOldblock);
    // void                UpdateLight(utils::wptr<Light> pLight);
    // void                UpdateblockLight(block* pBlock);

    // void                PropagateLight();
    // void                SeekLight();
    // void                UpdateSunLightDirect(std::vector<Point<int>>& lRays);
    // void                BuildSunLightDirect();
    // void                PropagateSunLightDirect();
    // void                UpdateSunLightIndirect();

    void                    add_unit(utils::refptr<unit> pUnit);
    void                    remove_unit(utils::wptr<unit> pUnit);
    utils::wptr<unit>       get_unit(const utils::ustring& sName);
    utils::wptr<const unit> get_unit(const utils::ustring& sName) const;

    // void UpdateLighting() const;
    // void UpdateLighting(std::vector<block_face>& lVertexCache) const;
    void update_cache() const;
    void update_cache(vertex_cache& lVertexCache) const;
    void build_occlusion() const;
    void build_occlusion(std::vector<block_face>& lVertexCache) const;
    void move_vertex_cache(const vector3f& mDiff) const;
    void move_vertex_cache(const vector3f& mDiff, std::vector<block_face>& lVertexCache) const;
    void set_normal_vbo_data(utils::refptr<vbo_data> pData) const;
    void set_two_sided_vbo_data(utils::refptr<vbo_data> pData) const;
    void set_alpha_blended_vbo_data(utils::refptr<vbo_data> pData) const;

    bool               is_normal_vertex_cache_empty() const;
    bool               is_two_sided_vertex_cache_empty() const;
    bool               is_alpha_blended_vertex_cache_empty() const;
    std::vector<block_face> get_cached_faces(const block* pBlock) const;
    std::vector<block_face> get_old_cached_faces(const block* pBlock) const;
    void               get_cached_world_faces(const block* pBlock, std::vector<block_face>& lFaces) const;

    static vector3f get_difference(std::weak_ptr<block_chunk> pFrom, std::weak_ptr<block_chunk> pTo);
    static inline anchor get_opposed(anchor mAnchor) {
        return OPPOSED_LIST[mAnchor];
    }

    static inline int div(int i) {
        static const int size2 = CHUNK_SIZE/2;
        static const int size = CHUNK_SIZE;
        if (i < 0) return (i - size2)/size;
        else       return (i + size2)/size;
    }

    static inline int mod(int i) {
        static const int size2 = CHUNK_SIZE/2;
        static const int size = CHUNK_SIZE;
        if (i < 0) return (i - size2)%size + size2;
        else       return (i + size2)%size - size2;
    }

    static inline int round(float f) {
        if (f > 0.0f) return static_cast<int>(f + 0.5f);
        else          return static_cast<int>(f - 0.5f);
    }

    static inline vector3i to_chunk_pos(const vector3f& v) {
        return vector3i(div(round(v.x)), div(round(v.y)), div(round(v.z)));
    }

    static inline vector3i to_block_pos(const vector3f& v) {
        return vector3i(mod(round(v.x)), mod(round(v.y)), mod(round(v.z)));
    }

    axis_aligned_box get_bounding_box() const;

    // NOTE : CHUNK_SIZE is supposed to be odd, to sum positions with integers on [-half; half]
    // I'm being lazy, yes, but that's not so much of a restriction :)
    static const uint CHUNK_SIZE = 15u;
    static const int  HALF_CHUNK_SIZE;
    static const axis_aligned_box BOUNDING_BOX;
    static uint CHUNK_COUNT;

private :

    static const std::array<block_chunk::anchor,6> OPPOSED_LIST;

    const block* get_block_(block::face mFace, block* pBlock, const vector3i& mPos) const;
    std::pair<std::weak_ptr<block_chunk>,block*>             get_neighbor_block_(block_chunk::anchor mAnchor, size_t id);
    std::pair<std::weak_ptr<const block_chunk>,const block*> get_neighbor_block_(block_chunk::anchor mAnchor, size_t id) const;
    const block*                                             get_neighbor_block__(block_chunk::anchor mAnchor, size_t id) const;

    // void AddNearbySunLightOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);
    // void AddNearbyLightOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);
    // void AddNearbyLightCheckOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);

    // void PropagateSunLight_(const std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lblockList);
    // void PropagateSunLight_(block* pBlock, uint uiNumblockDown = 0);
    // void PropagateLight_(block* pBlock);

    // void AddNearbySeekSunLightOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);
    // void AddNearbySeekSunLightBorderOpenblocks_(block* pBlock, std::map<uchar, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>>& lEmitterList);
    // void AddNearbySeekSunLightPropOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);
    // void AddNearbySeekLightOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);
    // void AddNearbySeekLightBorderOpenblocks_(block* pBlock, std::map<uchar, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>>& lEmitterList);
    // void AddNearbySeekLightPropOpenblocks_(block* pBlock, std::vector<std::pair<std::weak_ptr<block_chunk>,block*>>& lOpenList);

    // void SeekSunLight_(block* pBlock, uint uiRange, uint uiNumblockDown = 0);
    // void SeekLight_(block* pBlock, uint uiRange);

    // utils::wptr<Light> GetBrightestLight_(block* pBlock);

    void add_transparent_block_(block_chunk::anchor mAnchor);
    void remove_transparent_block_(block_chunk::anchor mAnchor);
    uint count_open_blocks_() const;
    void update_burried_state_() const;

    bool is_neighbor_face_open_(block_chunk::anchor mAnchor) const;
    void flag_update_neighbor_cache_(block_chunk::anchor mAnchor);
    void flag_update_collision_faces_(const block* pBlock);

    void optimize_cache_(vertex_cache& lVertexCache) const;

    utils::refptr<vbo_data> make_vbo_data_(const std::vector<block_face>& lVertexCache, uint uiOffset, uint uiNum) const;
    void updade_vbo_() const;

    std::weak_ptr<block_chunk> pSelf_;
    utils::wptr<world>         pWorld_;

    vector3i mCoordinates_;

    std::array<std::weak_ptr<block_chunk>,6> lNeighborList_;
    mutable bool                             bClearNeighborList_ = false;
    uint                                     uiValidNeighbor_ = 0;

    std::array<block, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE> lBlockList_;
    uint               uiPlainBlockCount_ = 0;
    std::array<uint,6> lFaceTranspBlockCount_;

    bool         bVisible_ = false;
    mutable bool bBurried_ = false;

    // std::map<std::string,utils::refptr<Light>> lLightList_;
    // bool                                       bNoSunLight_;
    // bool                                       bFullSunLight_;

    std::map<utils::ustring, utils::refptr<unit>> lUnitList_;

    mutable bool         bUpdateCache_ = true;
    // mutable bool         bUpdateLighting_ = false;
    mutable bool         bBuildOcclusion_ = false;
    mutable vertex_cache mVertexCache_;

    mutable bool                                bUpdateVBO_ = true;
    mutable bool                                bVBOReceived_ = true;
    mutable bool                                bVBOCleared_ = false;
    mutable utils::refptr<vertex_buffer_object> pNormalVBO_;
    mutable utils::refptr<vertex_buffer_object> pTwoSidedVBO_;
    mutable utils::refptr<vertex_buffer_object> pAlphaBlendedVBO_;

    mutable bool bNew_ = false;
};

struct chunk_id
{
    chunk_id() {}

    explicit chunk_id(std::weak_ptr<const block_chunk> pChunk) : pos(pChunk.lock()->get_coordinates()) {}

    chunk_id(const vector3i& p) : pos(p) {}

    bool operator < (const chunk_id& i) const {
        if (pos.x < i.pos.x) return true;
        if (pos.x > i.pos.x) return false;

        if (pos.y < i.pos.y) return true;
        if (pos.y > i.pos.y) return false;

        if (pos.z < i.pos.z) return true;

        return false;
    }

    bool operator == (const chunk_id& i) const {
        return pos == i.pos;
    }

    vector3i pos;
};

#endif
