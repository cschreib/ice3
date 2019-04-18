#ifndef WORLD_HPP
#define WORLD_HPP

#include "state.hpp"
#include "texture_manager.hpp"
#include "movable.hpp"
#include "threading_policies.hpp"
#include "worker_thread_fwd.hpp"
#include "blockchunk.hpp"
#include "unit.hpp"
#include "perlin.hpp"

#include <lxgui/gui_manager.hpp>
#include <lxgui/utils_string.hpp>
#include <set>

class camera;
class unit;
class shader;
class world;

struct update_chunk_task
{
    update_chunk_task() {}

    float operator () (float arg) const { return arg; }
};

struct load_chunk_task
{
    static const uint   NUM_SETS = 2;
    static const double dPersistence[NUM_SETS];
    static const double dFrequency[NUM_SETS];
    static const uint   uiOctaves[NUM_SETS];
    static const int    iSeed[NUM_SETS];
    perlin_noise        noise[NUM_SETS];

    utils::wptr<world> w;

    load_chunk_task() {}
    load_chunk_task(utils::wptr<world> pWorld);

    std::shared_ptr<block_chunk> operator () (const chunk_id& id) const;
    std::shared_ptr<block_chunk> generate_chunk(const chunk_id& chunk_pos) const;
    void                         get_terrain_height(vector3f& pos) const;

private :
};

struct save_chunk_task
{
    utils::wptr<world> w;

    save_chunk_task() {}
    save_chunk_task(utils::wptr<world> pWorld);

    void operator () (std::shared_ptr<block_chunk> chunk) const;

private :
};

class world : public state
{
public :

    world(application& mApp, bool bPlay, bool bShow);
    ~world();

    world(const world&) = delete;
    world& operator = (const world&) = delete;

    void set_self(utils::wptr<state> pSelf) override;
    void update(input_data& mData) override;
    void render() override;

    void load(const std::string& sFile);
    void clear();
    void renew();

    inline int get_seed() const {
        return iWorldSeed_;
    }

    std::weak_ptr<block_chunk>       get_chunk(const vector3i& pos);
    vector3f                         set_current_chunk(std::weak_ptr<block_chunk> pChunk);
    std::weak_ptr<block_chunk>       get_current_chunk();
    std::weak_ptr<const block_chunk> get_current_chunk() const;

    bool get_block_in_ray(const ray& mRay, block_collision_data& mData, float fMaxDistance = std::numeric_limits<float>::infinity());
    void select_block_in_ray(const ray& mRay, float fMaxDistance = std::numeric_limits<float>::infinity());
    void select_block(block* pBlock, std::weak_ptr<block_chunk> pBlockChunk, block::face mFace);

    block*                     get_selected_block();
    std::weak_ptr<block_chunk> get_selected_block_chunk();
    block::face                get_selected_block_face();

    void add_chunk(std::shared_ptr<block_chunk> pChunk);

    void generate_chunks_around(const vector3i& pos);

    void flag_update_visible_chunk_list() const;

    void update_chunk(std::weak_ptr<block_chunk> pChunk) const;
    void update_chunk_immediate(std::weak_ptr<block_chunk> pChunk) const;
    void update_chunk_vbo_immediate(std::weak_ptr<block_chunk> pChunk) const;

    void load_chunk_(const chunk_id& id);
    void unload_chunk_(const chunk_id& id);
    void unload_chunk_(std::shared_ptr<block_chunk> pChunk);

    template<class T>
    utils::wptr<T> create_unit(const utils::ustring& sName, std::weak_ptr<block_chunk> pChunk, block* pBlock)
    {
        utils::refptr<T> pUnit;

        std::shared_ptr<block_chunk> pLocked = pChunk.lock();
        if (!pLocked)
            return pUnit;

        std::set<utils::ustring>::iterator iter = lUnitLookupList_.find(sName);
        if (iter == lUnitLookupList_.end())
        {
            pUnit = utils::refptr<T>(new T(sName, utils::wptr<world>::cast(pSelf_), pChunk, pBlock));
            pUnit->set_self(pUnit);
            pLocked->add_unit(pUnit);
            notify_unit_created(pUnit);
        }
        else
        {
            std::cout << "# Warning # : world : cannot create new unit with name '"
                << utils::unicode_to_UTF8(sName) << "', already taken." << std::endl;
        }

        return pUnit;
    }

    utils::wptr<unit>       get_unit(const utils::ustring& sName);
    utils::wptr<const unit> get_unit(const utils::ustring& sName) const;
    utils::wptr<unit>       get_current_unit();

    void notify_unit_loaded(utils::wptr<unit> pUnit);
    void notify_unit_unloaded(utils::wptr<unit> pUnit);
    void notify_unit_created(utils::wptr<unit> pUnit);
    void notify_unit_destroyed(utils::wptr<unit> pUnit);
    void control_unit(utils::wptr<unit> pUnit);

    void enable_vbos(bool bVBOEnabled);
    void toggle_vbos();
    bool are_vbos_enabled() const;

    void enable_shaders(bool bShadersEnabled);
    void toggle_shaders();
    bool are_shaders_enabled() const;

    bool is_smooth_lighting_enabled() const;
    void lighten_color(color& c, uchar sunlight, uchar light) const;
    float get_occlusion(uchar sunlight, uchar light) const;

    struct render_data
    {
        float fRenderTime, fUpdateTime;
        uint  uiFrameNbr;
        uint  uiNumRenderedChunk, uiNumChunk, uiNumRenderedVertex;
        uint  uiUpdateQueuedChunkCount, uiCreateQueuedChunkCount;
    };

    render_data get_render_data() const;

    static const int REGION_SIZE;

private :

    void set_file_(const std::string& sFile);
    void read_config_();

    void on_camera_moved_(movable::movement_type mType);
    void update_loaded_chunk_list_();
    void update_visible_chunk_list_();
    void load_chunk_no_check_(const chunk_id& id);
    void update_chunk_(std::weak_ptr<block_chunk> chunk) const;
    void save_chunk_(std::shared_ptr<block_chunk> chunk);
    void receive_loaded_chunks_();
    void receive_loaded_chunk_(std::shared_ptr<block_chunk> chunk, bool immediate = false);

    void reload_shaders_();

    void update_lighting_(float fDelta);

    void render_chunks_() const;
    void render_chunk_(const block_chunk& chunk) const;
    void render_chunk_two_sided_(const block_chunk& chunk) const;
    void render_chunk_alpha_blended_(const block_chunk& chunk) const;
    void render_faces_(const std::vector<block_face>& data, size_t offset, size_t num) const;
    void render_faces_shader_(const std::vector<block_face>& data, size_t offset, size_t num) const;
    void render_faces_smooth_(const std::vector<block_face>& data, size_t offset, size_t num) const;
    void render_faces_shader_smooth_(const std::vector<block_face>& data, size_t offset, size_t num) const;
    void render_vertex_(const vertex& v) const;
    void render_vertex_shader_(const vertex& v, const block_face& mFace) const;
    void render_vertex_smooth_(const vertex& v, const color& mColor, const float& fOcclusion) const;
    void render_vertex_shader_smooth_(const vertex& v) const;
    void render_selected_block_() const;

private :

    application& mApp_;

    texture_manager             mTextureManager_;
    utils::refptr<gui::manager> pGUIManager_;

    std::string sFile_;
    std::string sDirectory_;
    bool        bEmpty_;
    bool        bJustLoaded_;

    int iWorldSeed_;

    std::shared_ptr<block_chunk>                     pCurrentChunk_;
    std::map<chunk_id, std::shared_ptr<block_chunk>> lLoadedChunkList_;
    std::set<chunk_id>                               lLoadingChunkList_;
    std::vector<block_chunk*>                        lVisibleChunkList_;
    mutable bool bUpdateVisibleChunkList_;

    block*                     pSelectedBlock_;
    std::weak_ptr<block_chunk> pSelectedChunk_;
    block::face                mSelectedFace_;

    utils::refptr<camera>                       pCamera_;
    std::map<utils::ustring, utils::wptr<unit>> lRegisteredUnitList_;
    std::set<utils::ustring>                    lUnitLookupList_;
    utils::wptr<unit>                           pCurrentUnit_;

    utils::refptr<god> pGod_;

    static const size_t NUM_PRIORITY_QUEUES = 10;
    static const size_t SLEEP_TIME = 20;

    typedef worker_thread<
        update_chunk_task, NUM_PRIORITY_QUEUES,
        policies::sleep::sleep_for<SLEEP_TIME>
    > updater_thread_t;

    typedef multitask_worker_thread<
        policies::sleep::sleep_for<SLEEP_TIME>
    > loader_thread_t;

    typedef worker<
        load_chunk_task, NUM_PRIORITY_QUEUES
    > load_worker_t;

    typedef worker<
        save_chunk_task
    > save_worker_t;

    utils::refptr<updater_thread_t> pUpdaterThread_;
    utils::refptr<load_worker_t>    pLoadWorker_;
    utils::refptr<save_worker_t>    pSaveWorker_;
    utils::refptr<loader_thread_t>  pLoaderThread_;

    size_t uiNumReceivedChunkPerFrame_;

    mutable render_data mRenderData_;

    float fRenderDistance_;
    bool bVBOEnabled_, bVBOSupported_, bUseVBO_;
    bool bShadersEnabled_, bShadersSupported_, bUseShaders_;
    mutable utils::refptr<shader> pBlockShader_;
    bool bWireframe_, bFog_;

    bool bSmoothLighting_;
    bool bBuildLightingArray_;
    uint uiLightingArraySize_, uiLightingArrayRatio_;
    uint uiLightingUpdateRate_;
    utils::refptr<texture> lLightingArray_;
    bool bSunLight_, bLights_;

    color mSunDayColor_, mSunDawnColor_, mSunNightColor_;
    color mSkyDayColor_, mSkyDawnColor_, mSkyNightColor_;
    color mSkyColor_,    mSunColor_,     mTorchLight_;

    float fDayTime_, fDayDuration_;
    bool  bDayNightCycle_;
};

#endif
