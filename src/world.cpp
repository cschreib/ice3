#include "world.hpp"
#include "texture_manager.hpp"
#include "application.hpp"
#include "vertex.hpp"
#include "camera.hpp"
#include "unit.hpp"
#include "block.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "worker_thread.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/gui_texture.hpp>
#include <lxgui/gui_fontstring.hpp>
#include <lxgui/gui_button.hpp>
#include <lxgui/gui_editbox.hpp>
#include <lxgui/impl/gui_gl_manager.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/luapp.hpp>
#include <iostream>

void register_glues(lua::state& mState);

world::world(application& mApp, bool bPlay, bool bShow) :
    state(mApp, bPlay, bShow), mApp_(mApp),
    mGUIManager_(
        mApp_.get_input_data().mInput.get_handler(),
        mApp_.get_locale(),
        mApp_.get_window().getSize().x,
        mApp_.get_window().getSize().y,
        utils::refptr<gui::manager_impl>(new gui::gl::manager())
    ),
    pCamera_(new camera(*this)), pUpdaterThread_(new updater_thread_t),
    pLoadWorker_(new load_worker_t(*this)), pSaveWorker_(new save_worker_t(*this)),
    pLoaderThread_(new loader_thread_t(*pLoadWorker_, *pSaveWorker_)),
    mLightingArray_(
        uiLightingArraySize_, uiLightingArraySize_, texture::CLAMP, texture::NONE
    )
{
    bVBOSupported_ = vertex_buffer_object::is_supported();
    std::cout << "Note : vertex buffer objects are " <<
        (bVBOSupported_ ? "" : "not ") << "suported." << std::endl;

    bShadersSupported_ = shader::is_supported();
    std::cout << "Note : shaders are " <<
        (bShadersSupported_ ? "" : "not ") << "suported." << std::endl;

    std::cout << stamp << " Loading GUI..." << std::endl;

    gui::out.rdbuf(std::cout.rdbuf());

    mGUIManager_.add_addon_directory("interface/addons");
    mGUIManager_.create_lua([this]() {
        mGUIManager_.register_region_type<gui::texture>("Texture");
        mGUIManager_.register_region_type<gui::font_string>("FontString");
        mGUIManager_.register_frame_type<gui::button>("Button");
        mGUIManager_.register_frame_type<gui::edit_box>("EditBox");

        utils::refptr<lua::state> pLua = mGUIManager_.get_lua().lock();

        pLua->do_file("key_codes.lua");
        pLua->push_userdata(this);
        pLua->set_global("_WRLD");

        register_glues(*pLua);
    });
    mGUIManager_.read_files();
    mGUIManager_.enable_caching(true);
    std::cout << stamp << " Done." << std::endl;

    mRenderData_.uiFrameNbr = 0;
    mRenderData_.fRenderTime = 0.0f;
    mRenderData_.fUpdateTime = 0.0f;

    initialize_block_data(mTextureManager_);

    std::cout << stamp << " Reading configuration..." << std::endl;
    read_config_();
    std::cout << stamp << " Done." << std::endl;

    bUseVBO_ = bVBOSupported_ && bVBOEnabled_;
    bUseShaders_ = bShadersSupported_ && (bShadersEnabled_ || bUseVBO_);
    if (bUseShaders_)
    {
        std::cout << stamp << " Loading shaders..." << std::endl;
        reload_shaders_();
        std::cout << stamp << " Done." << std::endl;
    }

    pCamera_->set_on_moved_listener(std::bind(&world::on_camera_moved_, this, std::placeholders::_1));

    std::cout << stamp << " Creating threads..." << std::endl;
    //pUpdaterThread_->start_new();
    pLoadWorker_->start_new(*this);
    pSaveWorker_->start_new(*this);
    pLoaderThread_->start();
    std::cout << stamp << " Done." << std::endl;

    vector3f mStartPos(0.0f, 0.0f, 0.0f);
    pLoadWorker_->get_worker().get_terrain_height(mStartPos);
    mStartPos.y = std::max(0.0f, mStartPos.y) + 1.0f + 1.0f;

    generate_chunks_around(block_chunk::to_chunk_pos(mStartPos));

    pGod_ = create_unit<god>(
        U"TheAlmighty", pCurrentChunk_, pCurrentChunk_->get_block(
            block_chunk::to_block_pos(mStartPos)
        )
    );
    control_unit(pGod_);
}

world::~world()
{
    clear();

    pUpdaterThread_ = nullptr;
    pLoaderThread_ = nullptr;
    pLoadWorker_ = nullptr;
    pSaveWorker_ = nullptr;
}

void world::read_config_()
{
    if (mApp_.is_constant_defined("enable_shaders"))
        bShadersEnabled_ = mApp_.get_constant<bool>("enable_shaders");

    /*if (mApp_.is_constant_defined("no_world_save"))
        bNoSave_ = mApp_.get_constant<bool>("no_world_save");*/

    if (mApp_.is_constant_defined("enable_vbos"))
        bVBOEnabled_ = mApp_.get_constant<bool>("enable_vbos");

    if (mApp_.is_constant_defined("enable_smooth_lighting"))
        bSmoothLighting_ = mApp_.get_constant<bool>("enable_smooth_lighting");

    if (mApp_.is_constant_defined("lighting_update_rate"))
        uiLightingUpdateRate_ = mApp_.get_constant<double>("lighting_update_rate");

    if (mApp_.is_constant_defined("view_distance"))
        fRenderDistance_ = std::max(0.0, mApp_.get_constant<double>("view_distance"));

    if (mApp_.is_constant_defined("day_duration"))
        fDayDuration_ = std::max(0.0, mApp_.get_constant<double>("day_duration"));

    if (mApp_.is_constant_defined("world_seed"))
        iWorldSeed_ = mApp_.get_constant<double>("world_seed");

    if (mApp_.is_constant_defined("max_generated_chunks"))
        uiNumReceivedChunkPerFrame_ = mApp_.get_constant<double>("max_generated_chunks");

    if (mApp_.is_constant_defined("sun_day_color"))
        mSunDayColor_ = mApp_.get_constant<color>("sun_day_color");

    if (mApp_.is_constant_defined("sun_dawn_color"))
        mSunDawnColor_ = mApp_.get_constant<color>("sun_dawn_color");

    if (mApp_.is_constant_defined("sun_night_color"))
        mSunNightColor_ = mApp_.get_constant<color>("sun_night_color");

    if (mApp_.is_constant_defined("torch_light_color"))
        mTorchLight_ = mApp_.get_constant<color>("torch_light_color");

    if (mApp_.is_constant_defined("sky_day_color"))
        mSkyDayColor_ = mApp_.get_constant<color>("sky_day_color");

    if (mApp_.is_constant_defined("sky_dawn_color"))
        mSkyDawnColor_ = mApp_.get_constant<color>("sky_dawn_color");

    if (mApp_.is_constant_defined("sky_night_color"))
        mSkyNightColor_ = mApp_.get_constant<color>("sky_night_color");
}

void world::clear()
{
    pLoaderThread_->abort();
    pUpdaterThread_->abort();

    lRegisteredUnitList_.clear();
    pGod_ = nullptr;
    control_unit(nullptr);

    lLoadedChunkList_.clear();
    pCurrentChunk_ = nullptr;

    lLoadingChunkList_.clear();
    lVisibleChunkList_.clear();

    pSelectedBlock_ = nullptr;
    pSelectedChunk_ = std::weak_ptr<block_chunk>();

    sFile_ = sDirectory_ = "";

    bEmpty_ = true;

    fDayTime_ = 0.0f;
}

void world::renew()
{
    clear();

    pUpdaterThread_->start_new();
    pLoadWorker_->start_new(*this);
    pSaveWorker_->start_new(*this);
    pLoaderThread_->start();
}

world::render_data world::get_render_data() const
{
    render_data mData = mRenderData_;
    if (mData.uiFrameNbr != 0)
    {
        mData.fRenderTime /= mData.uiFrameNbr;
        mData.fUpdateTime /= mData.uiFrameNbr;
    }

    mData.uiNumChunk = lLoadedChunkList_.size();
    mData.uiCreateQueuedChunkCount = lLoadingChunkList_.size();

    mData.uiNumRenderedChunk = lVisibleChunkList_.size();
    mData.uiUpdateQueuedChunkCount = 0;

    mRenderData_.uiFrameNbr  = 0;
    mRenderData_.fRenderTime = 0.0f;
    mRenderData_.fUpdateTime = 0.0f;

    return mData;
}

void world::update(input_data& mData)
{
    if (bJustLoaded_)
    {
        mData.fDelta = 0.0166f;
        bJustLoaded_ = false;
    }

    sf::Clock c;

    if (mData.mInput.can_receive_input("WORLD"))
    {
        if (mData.mInput.key_is_released(input::key::K_ESCAPE))
            ask_shutdown();

        /*if (mData.mInput.key_is_pressed(input::key::K_P))
            std::cout << mGUIManager_.print_ui() << std::endl;*/

        if (mData.mInput.key_is_pressed(input::key::K_LSHIFT) ||
            mData.mInput.key_is_pressed(input::key::K_RSHIFT))
        {
            if (mApp_.is_mouse_grabbed())
            {
                mApp_.grab_mouse(false);
                mData.mInput.block_input("UNIT");
                mGUIManager_.enable_input(true);
            }
            else
            {
                mApp_.grab_mouse(true);
                mData.mInput.allow_input("UNIT");
                mGUIManager_.enable_input(false);
            }
        }
    }

    receive_loaded_chunks_();

    update_lighting_(mData.fDelta);

    for (auto& iter : lLoadedChunkList_)
        update_chunk_(iter.second);

    for (auto& unit : lRegisteredUnitList_)
        unit.second->update(mData);

    update_visible_chunk_list_();

    select_block_in_ray(pCamera_->get_mouse_ray(0.5f, 0.5f));

    mGUIManager_.update(mData.fDelta);
    mData.mInput.set_focus(mGUIManager_.get_input_manager()->is_focused());

    mRenderData_.fUpdateTime += c.getElapsedTime().asSeconds();
}

void world::update_lighting_(float fDelta)
{
    float fOldDayTime = fDayTime_;

    if (bDayNightCycle_)
    {
        fDayTime_ += fDelta/fDayDuration_;
        while (fDayTime_ > 1.0f)
            fDayTime_ -= 1.0f;
    }

    const float fDayRatio = 0.5f;
    const float fDawnRatio = 0.1f;
    const float fNightRatio = 1.0f - fDayRatio - 2.0f*fDawnRatio;

    const float fDayEnd = fDayRatio;                      // 0.5f
    const float fDawnMid = fDayEnd + fDawnRatio/2.0f;     // 0.55f
    const float fDawnEnd = fDayEnd + fDawnRatio;          // 0.6f
    const float fNightEnd = fDawnEnd + fNightRatio;       // 0.9f
    const float fDuskMid = fNightEnd + fDawnRatio/2.0f;   // 0.95f

#if ONLY_DAY
    fDayTime_ = 0.5f*fDayRatio;
#elif ONLY_NIGHT
    fDayTime_ = fDawnEnd + 0.5f*fNightRatio;
#endif

    if (fDayTime_ < fDayEnd)
    {
        mSkyColor_ = mSkyDayColor_;
        mSunColor_ = mSunDayColor_;

        if (fOldDayTime > fDayEnd)
            bBuildLightingArray_ = true;

        if (!bUseShaders_)
            bUseVBO_ = (bVBOSupported_ && bVBOEnabled_);
    }
    else if (fDayTime_ < fDawnMid)
    {
        float fCoef = 2.0f*(fDayTime_ - fDayEnd)/fDawnRatio;
        mSkyColor_ = mSkyDayColor_*(1.0f - fCoef) + mSkyDawnColor_*fCoef;
        mSunColor_ = mSunDayColor_*(1.0f - fCoef) + mSunDawnColor_*fCoef;

        float fOldCoef = (fOldDayTime - fDayEnd)/fDawnRatio;
        for (uint i = 1; i <= uiLightingUpdateRate_; ++i)
        {
            if (fCoef/2.0f >= (float)i/(uiLightingUpdateRate_+1) &&
                fOldCoef    < (float)i/(uiLightingUpdateRate_+1))
            {
                bBuildLightingArray_ = true;
                break;
            }
        }

        if (!bUseShaders_)
            bUseVBO_ = false;
    }
    else if (fDayTime_ < fDawnEnd)
    {
        float fCoef = 2.0f*(fDayTime_ - fDawnMid)/fDawnRatio;
        mSkyColor_ = mSkyDawnColor_*(1.0f - fCoef) + mSkyNightColor_*fCoef;
        mSunColor_ = mSunDawnColor_*(1.0f - fCoef) + mSunNightColor_*fCoef;

        float fOldCoef = (fOldDayTime - fDawnMid)/fDawnRatio;
        for (uint i = 1; i <= uiLightingUpdateRate_; ++i)
        {
            if (fCoef/2.0f + 0.5f >= (float)i/(uiLightingUpdateRate_+1) &&
                fOldCoef           < (float)i/(uiLightingUpdateRate_+1))
            {
                bBuildLightingArray_ = true;
                break;
            }
        }

        if (!bUseShaders_)
            bUseVBO_ = false;
    }
    else if (fDayTime_ < fNightEnd)
    {
        mSkyColor_ = mSkyNightColor_;
        mSunColor_ = mSunNightColor_;

        if (fOldDayTime > fNightEnd || fOldDayTime < fDawnEnd)
            bBuildLightingArray_ = true;

        if (!bUseShaders_)
            bUseVBO_ = (bVBOSupported_ && bVBOEnabled_);
    }
    else if (fDayTime_ < fDuskMid)
    {
        float fCoef = 2.0f*(fDayTime_ - fNightEnd)/fDawnRatio;
        mSkyColor_ = mSkyNightColor_*(1.0f - fCoef) + mSkyDawnColor_*fCoef;
        mSunColor_ = mSunNightColor_*(1.0f - fCoef) + mSunDawnColor_*fCoef;

        float fOldCoef = (fOldDayTime - fNightEnd)/fDawnRatio;
        for (uint i = 1; i <= uiLightingUpdateRate_; ++i)
        {
            if (fCoef/2.0f >= (float)i/(uiLightingUpdateRate_+1) &&
                fOldCoef    < (float)i/(uiLightingUpdateRate_+1))
            {
                bBuildLightingArray_ = true;
                break;
            }
        }

        if (!bUseShaders_)
            bUseVBO_ = false;
    }
    else
    {
        float fCoef = 2.0f*(fDayTime_ - fDuskMid)/fDawnRatio;
        mSkyColor_ = mSkyDawnColor_*(1.0f - fCoef) + mSkyDayColor_*fCoef;
        mSunColor_ = mSunDawnColor_*(1.0f - fCoef) + mSunDayColor_*fCoef;

        float fOldCoef = (fOldDayTime - fNightEnd)/fDawnRatio;
        for (uint i = 1; i <= uiLightingUpdateRate_; ++i)
        {
            if (fCoef/2.0f + 0.5f >= (float)i/(uiLightingUpdateRate_+1) &&
                fOldCoef           < (float)i/(uiLightingUpdateRate_+1))
            {
                bBuildLightingArray_ = true;
                break;
            }
        }

        if (!bUseShaders_)
            bUseVBO_ = false;
    }

    if (bBuildLightingArray_)
    {
        if (bSunLight_ && bLights_)
        {
            color mSun;
            color mLight;

            for (uint s = 0; s < uiLightingArraySize_; ++s)
            {
                mSun = mSunColor_*light::INTENSITY_TABLE[s*uiLightingArrayRatio_];
                for (uint l = 0; l < uiLightingArraySize_; ++l)
                {
                    mLight = mSun + mTorchLight_*light::INTENSITY_TABLE[l*uiLightingArrayRatio_];
                    mLight.saturate();
                    mLight.a = block::OCCLUSION_TABLE2[std::max(s, l)*uiLightingArrayRatio_];

                    mLightingArray_.set_pixel(s, l, texture::color(mLight));
                }
            }
        }
        else if (!bSunLight_ && bLights_)
        {
            color mLight;

            for (uint l = 0; l < uiLightingArraySize_; ++l)
            {
                mLight = mTorchLight_*light::INTENSITY_TABLE[l*uiLightingArrayRatio_];
                mLight.a = block::OCCLUSION_TABLE2[l*uiLightingArrayRatio_];

                for (uint s = 0; s < uiLightingArraySize_; ++s)
                    mLightingArray_.set_pixel(s, l, texture::color(mLight));
            }
        }
        else if (bSunLight_ && !bLights_)
        {
            color mLight;

            for (uint s = 0; s < uiLightingArraySize_; ++s)
            {
                mLight = mSunColor_*light::INTENSITY_TABLE[s*uiLightingArrayRatio_];
                mLight.a = block::OCCLUSION_TABLE2[s*uiLightingArrayRatio_];

                for (uint l = 0; l < uiLightingArraySize_; ++l)
                    mLightingArray_.set_pixel(s, l, texture::color(mLight));
            }
        }
        else
        {
            for (uint s = 0; s < uiLightingArraySize_; ++s)
            for (uint l = 0; l < uiLightingArraySize_; ++l)
                mLightingArray_.set_pixel(s, l, texture::color(255, 255, 255));
        }

        mLightingArray_.update_texture();
        //mLightingArray_.save_to_file("light.png");

        bBuildLightingArray_ = false;
    }
}

void world::update_visible_chunk_list_()
{
    if (!bUpdateVisibleChunkList_)
        return;

    lVisibleChunkList_.clear();

    std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter;
    foreach (iter, lLoadedChunkList_)
    {
        block_chunk& mChunk = *iter->second;
        if (!mChunk.bBurried_ && mChunk.uiPlainBlockCount_ != 0u &&
            pCamera_->is_visible(mChunk.get_bounding_box()))
        {
            lVisibleChunkList_.push_back(&mChunk);
        }
    }

    bUpdateVisibleChunkList_ = false;
}

void world::on_camera_moved_(movable::movement_type mType)
{
    flag_update_visible_chunk_list();
}

unit* world::get_unit(const utils::ustring& sName)
{
    std::map<utils::ustring, unit*>::iterator iter = lRegisteredUnitList_.find(sName);
    if (iter != lRegisteredUnitList_.end())
        return iter->second;

    return nullptr;
}

const unit* world::get_unit(const utils::ustring& sName) const
{
    std::map<utils::ustring, unit*>::const_iterator iter = lRegisteredUnitList_.find(sName);
    if (iter != lRegisteredUnitList_.end())
        return iter->second;

    return nullptr;
}

unit* world::get_current_unit()
{
    return pCurrentUnit_;
}

void world::notify_unit_loaded(unit& mUnit)
{
    lRegisteredUnitList_[mUnit.get_name()] = &mUnit;
}

void world::notify_unit_unloaded(unit& mUnit)
{
    lRegisteredUnitList_.erase(mUnit.get_name());
}

void world::control_unit(unit* pUnit)
{
    if (pUnit == pCurrentUnit_)
        return;

    if (pCurrentUnit_)
        pCurrentUnit_->notify_current(false);

    pCurrentUnit_ = pUnit;

    if (pCurrentUnit_)
    {
        pCamera_->set_parent(&pCurrentUnit_->get_camera_anchor());
        pCamera_->set_position(vector3f::ZERO);
        pCamera_->set_scale(vector3f::UNIT);
        pCamera_->set_orientation(quaternion::UNIT);
        pCurrentUnit_->notify_current(true);
    }
    else
    {
        vector3f pos, scale; quaternion orient;
        pCamera_->get_absolute_transform(pos, scale, orient);
        pCamera_->set_parent(nullptr);
        pCamera_->set_position(pos);
        pCamera_->set_scale(scale);
        pCamera_->set_orientation(orient);
    }
}

void world::enable_vbos(bool bVBOEnabled)
{
    if (bVBOEnabled_ != bVBOEnabled)
        toggle_vbos();
}

void world::toggle_vbos()
{
    bool bOld = bUseVBO_;
    bVBOEnabled_ = !bVBOEnabled_;
    bUseVBO_ = bVBOEnabled_ && bVBOSupported_;

    if (bOld == bUseVBO_)
        return;

    if (bUseVBO_)
    {
        for (auto& iter : lLoadedChunkList_)
        {
            block_chunk& mChunk = *iter.second;
            mChunk.move_vertex_cache(mChunk.get_position(pCurrentChunk_));
            update_chunk_vbo_immediate(iter.second);
        }
    }
    else
    {
        for (auto& iter : lLoadedChunkList_)
        {
            block_chunk& mChunk = *iter.second;
            mChunk.move_vertex_cache(-mChunk.get_position(pCurrentChunk_));
        }
    }
}

bool world::are_vbos_enabled() const
{
    return bUseVBO_;
}

void world::enable_shaders(bool bShadersEnabled)
{
    if (bShadersEnabled_ != bShadersEnabled)
        toggle_shaders();
}

void world::toggle_shaders()
{
    bool bOld = bUseShaders_;
    bShadersEnabled_ = !bShadersEnabled_;
    bUseShaders_ = (bShadersEnabled_ || bUseVBO_) && bShadersSupported_;

    if (bOld == bUseShaders_)
        return;

    reload_shaders_();
}

bool world::are_shaders_enabled() const
{
    return bUseShaders_;
}

void world::reload_shaders_()
{
    if (bUseShaders_)
    {
        if (bSmoothLighting_)
            pBlockShader_ = std::shared_ptr<shader>(new shader("shaders/block_smooth_vs.glsl", "shaders/block_smooth_fs.glsl"));
        else
            pBlockShader_ = std::shared_ptr<shader>(new shader("shaders/block_vs.glsl", "shaders/block_fs.glsl"));

        if (!pBlockShader_)
        {
            pBlockShader_ = nullptr;
            bShadersSupported_ = false;
            bUseShaders_ = false;
        }
    }
    else
        pBlockShader_ = nullptr;
}

bool world::is_smooth_lighting_enabled() const
{
    return bSmoothLighting_;
}

void world::lighten_color(color& c, uchar sunlight, uchar light) const
{
}

float world::get_occlusion(uchar sunlight, uchar light) const
{
    return 0.0f;
}

std::weak_ptr<block_chunk> world::get_chunk(const vector3i& pos)
{
    std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter = lLoadedChunkList_.find(chunk_id(pos));
    if (iter != lLoadedChunkList_.end())
        return iter->second;
    else
        return std::weak_ptr<block_chunk>();
}

vector3f world::set_current_chunk(std::weak_ptr<block_chunk> pWChunk)
{
    std::shared_ptr<block_chunk> pChunk = pWChunk.lock();
    if (!pChunk)
        return vector3f::ZERO;

    vector3i delta = pChunk->get_coordinates();

    if (pCurrentChunk_)
        delta -= pCurrentChunk_->get_coordinates();

    pCurrentChunk_ = pChunk;

    vector3f mDiff = block_chunk::CHUNK_SIZE*delta;

    if (!bUseVBO_)
    {
        std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter;
        for (auto& p : lLoadedChunkList_)
            p.second->move_vertex_cache(mDiff);
    }

    update_loaded_chunk_list_();

    return mDiff;
}

std::weak_ptr<block_chunk> world::get_current_chunk()
{
    return pCurrentChunk_;
}

std::weak_ptr<const block_chunk> world::get_current_chunk() const
{
    return pCurrentChunk_;
}

void world::add_chunk(std::shared_ptr<block_chunk> pChunk)
{
    if (!pChunk)
        return;

    bEmpty_ = false;

    chunk_id mID(pChunk);

    std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter = lLoadedChunkList_.find(mID);
    if (iter != lLoadedChunkList_.end())
    {
        std::cout << "# Warning # : world : a chunk with coordinates "
            << mID.pos << " already exists." << std::endl;
        return;
    }

    lLoadedChunkList_.insert(std::make_pair(mID, pChunk));

    /*PackedID mTopID(pChunk->iX_, 0, pChunk->iZ_);
    std::map<PackedID, std::weak_ptr<BlockChunk>>::iterator iterTop = lTopChunkList_.find(mTopID);
    if (iterTop != lTopChunkList_.End())
    {
        if (iterTop->second->iY_ < pChunk->iY_)
            iterTop->second = pChunk;
    }
    else
        lTopChunkList_.Insert(MakePair(mTopID, pChunk.CreateWeak()));*/

    pChunk->attach_to(get_chunk(mID.pos - vector3i::UNIT_X), block_chunk::LEFT);
    pChunk->attach_to(get_chunk(mID.pos + vector3i::UNIT_X), block_chunk::RIGHT);
    pChunk->attach_to(get_chunk(mID.pos - vector3i::UNIT_Z), block_chunk::FRONT);
    pChunk->attach_to(get_chunk(mID.pos + vector3i::UNIT_Z), block_chunk::BACK);
    pChunk->attach_to(get_chunk(mID.pos - vector3i::UNIT_Y), block_chunk::BOTTOM);
    pChunk->attach_to(get_chunk(mID.pos + vector3i::UNIT_Y), block_chunk::TOP);

    flag_update_visible_chunk_list();
}

void world::generate_chunks_around(const vector3i& pos)
{
    std::cout << stamp << "Generating chunks around " << pos << std::endl;
    std::shared_ptr<block_chunk> chunk = pLoadWorker_->get_worker()(pos);
    add_chunk(chunk);
    set_current_chunk(chunk);
    update_chunk_immediate(chunk);
}

void world::flag_update_visible_chunk_list() const
{
    bUpdateVisibleChunkList_ = true;
}

void world::update_loaded_chunk_list_()
{
    if (!pCurrentChunk_)
        return;

    std::map<chunk_id, std::shared_ptr<block_chunk>> lUnloadMap = lLoadedChunkList_;
    lLoadedChunkList_.clear();
    //lTopChunkList_.clear();

    chunk_id mID(pCurrentChunk_);
    lLoadedChunkList_.insert(std::make_pair(mID, pCurrentChunk_));

    float fTemp = fRenderDistance_/block_chunk::CHUNK_SIZE;
    int viewmax = std::max(1.0, ceil(fTemp));
    int dmax2 = std::max(3.0, ceil(fTemp*fTemp));

    vector3i pos;
    for (pos.z = -viewmax; pos.z <= viewmax; ++pos.z)
    for (pos.y = -viewmax; pos.y <= viewmax; ++pos.y)
    for (pos.x = -viewmax; pos.x <= viewmax; ++pos.x)
    {
        if (pos.get_norm_squared() > dmax2)
            continue;

        chunk_id mTempID(mID.pos + pos);
        std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter = lUnloadMap.find(mTempID);
        if (iter != lUnloadMap.end())
        {
            lLoadedChunkList_.insert(std::make_pair(mTempID, iter->second));
            lUnloadMap.erase(iter);
        }
        else
            load_chunk_(mTempID);
    }

    std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter;
    for (auto& p : lUnloadMap)
        unload_chunk_(p.second);

    flag_update_visible_chunk_list();
}

bool world::get_block_in_ray(const ray& mRay, block_collision_data& mData, float fMaxDistance)
{
    if (!pCurrentChunk_)
        return false;

    std::pair<std::weak_ptr<block_chunk>, block*> mPair, mNewPair;
    std::shared_ptr<block_chunk>                  pChunk, pNewChunk;

    // Get the chunk that contains the ray origin
    vector3i mChunkPos = block_chunk::to_chunk_pos(mRay.mOrigin);
    mPair.first = get_chunk(mChunkPos + pCurrentChunk_->get_coordinates());
    pChunk = mPair.first.lock();

    if (!pChunk)
        return false;

    mData.pChunk = pChunk;

    // Transport the ray in chunk local coordinates
    ray mLocalRay = mRay;
    mLocalRay.mOrigin -= float(block_chunk::CHUNK_SIZE)*mChunkPos;

    vector3f               mNextPoint;
    axis_aligned_box::face mNextFace;

    const axis_aligned_box mBox(
        vector3f(-0.5f, -0.5f, -0.5f),
        vector3f( 0.5f,  0.5f,  0.5f)
    );

    // Get the block that contains the ray origin
    vector3i mBlockPos = block_chunk::to_block_pos(mLocalRay.mOrigin);
    mPair.second = pChunk->get_block(mBlockPos);

    // Transport the ray in block local coordinates
    mLocalRay.mOrigin -= vector3f(mBlockPos);

    // Get the face of the block that is pointed by the ray
    mBox.get_inside_ray_intersection(mLocalRay, mNextPoint, mNextFace);

    // Compute the step to get to that face
    vector3f mStep = mNextPoint - mLocalRay.mOrigin;
    vector3f mBlockDelta = block::NORMAL_ARRAY[(block::face)(int)mNextFace];

    // If block is not transparant, then we have a match
    if (mPair.second->t != block::EMPTY && mPair.second->t != block::WATER)
    {
        mData.pBlock = mPair.second;
        mData.mFace  = (block::face)(int)mNextFace;
        mData.fDist  = mStep.get_norm();
        return true;
    }

    // Else, we have to move on to the next block on the ray
    mNewPair = pChunk->get_block((block::face)(int)mNextFace, mPair.second);
    pNewChunk = mNewPair.first.lock();
    if (pNewChunk != pChunk)
    {
        mPair.first = mNewPair.first;
        pChunk = pNewChunk;
        mData.pChunk = pChunk;
    }

    mPair.second = mNewPair.second;

    while (pChunk && mPair.second)
    {
        block::face mFace = block::OPPOSED_LIST[(block::face)(int)mNextFace];

        // If block is not transparant, then we have a match
        if (mPair.second->t != block::EMPTY && mPair.second->t != block::WATER)
        {
            mData.pBlock = mPair.second;
            mData.mFace  = mFace;
            mData.fDist  = mStep.get_norm();
            return true;
        }

        // Else we have to find out which block to go next
        // Build a new ray that starts where the previous block stopped
        ray mTempRay = mLocalRay;
        mTempRay.mOrigin += mStep;

        // Transport it to block local coordinates
        mTempRay.mOrigin -= mBlockDelta;

        // Get the intersection of the ray and the current block's bounding box,
        // ignoring the face that was pierced by the previous ray
        mBox.get_inside_ray_intersection_ignore_face(
            mTempRay, mNextPoint, mNextFace, (axis_aligned_box::face)(int)mFace
        );

        // Add the distance to the face to the step
        mStep += mNextPoint - mTempRay.mOrigin;
        mBlockDelta += block::NORMAL_ARRAY[(block::face)(int)mNextFace];

        // Get the next block
        mNewPair = pChunk->get_block((block::face)(int)mNextFace, mPair.second);
        pNewChunk = mNewPair.first.lock();
        if (pNewChunk != pChunk)
        {
            mPair.first = mNewPair.first;
            pChunk = pNewChunk;
            mData.pChunk = pChunk;
        }

        mPair.second = mNewPair.second;
    }

    return false;
}

void world::select_block_in_ray(const ray& mRay, float fMaxDistance)
{
    block_collision_data mData;
    if (!get_block_in_ray(mRay, mData, fMaxDistance))
    {
        select_block(nullptr, std::weak_ptr<block_chunk>(), mData.mFace);
        return;
    }

    vector3f mBlockPosition = mData.pChunk->get_block_world_position(mData.pBlock);

    if ((mBlockPosition - mRay.mOrigin).get_norm() > fMaxDistance)
        select_block(nullptr, std::weak_ptr<block_chunk>(), mData.mFace);
    else
        select_block(mData.pBlock, mData.pChunk, mData.mFace);
}

void world::select_block(block* pBlock, std::weak_ptr<block_chunk> pBlockChunk, block::face mFace)
{
    pSelectedBlock_ = pBlock;
    pSelectedChunk_ = pBlockChunk;
    mSelectedFace_  = mFace;
}

block* world::get_selected_block()
{
    return pSelectedBlock_;
}

std::weak_ptr<block_chunk> world::get_selected_block_chunk()
{
    return pSelectedChunk_;
}

block::face world::get_selected_block_face()
{
    return mSelectedFace_;
}
