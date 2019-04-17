#include "world.hpp"
#include "worldfile.hpp"
#include "worker_thread.hpp"
#include <utils_filesystem.hpp>
#include <fstream>

const int world::REGION_SIZE = 9;

template<typename T>
void file_read(std::ifstream& f, T& obj)
{
    f.read((char*)&obj, sizeof(T));
}

template<typename T>
void file_write(std::ofstream& f, T& obj)
{
    f.write((char*)&obj, sizeof(T));
}

void world::set_file_(const std::string& sFile)
{
    if (sFile.empty())
    {
        std::string sTest = "new_world_001";
        uint i = 1;
        while (utils::file_exists("saves/worlds/"+sTest+"/"+sTest+".wrld"))
        {
            ++i;
            sTest = "new_world"+utils::to_string(i, 3);
        }

        sFile_ = "saves/worlds/"+sTest+"/"+sTest+".wrld";
        sDirectory_ = "saves/worlds/"+sTest;
        utils::make_directory(sDirectory_);
        utils::make_directory(sDirectory_+"/regions");
    }
    else
    {
        sFile_ = sFile;
        sDirectory_ = sFile_;
        size_t uiFileNameSize = utils::cut(utils::cut(sFile_, "/").back(), "\\").back().size();
        sDirectory_.erase(sDirectory_.size() - uiFileNameSize - 1, uiFileNameSize + 1);
        utils::make_directory(sDirectory_);
        utils::make_directory(sDirectory_+"/regions");
    }
}

void world::load(const std::string& sFile)
{
    if (!utils::file_exists(sFile))
        throw utils::exception("world::load", "cannot find file '"+sFile+"'.");

    std::cout << "world : loading world '"+sFile+"'..." << std::endl;
    if (!bEmpty_)
        renew();

    std::ifstream mFile(sFile, std::ios::binary);

    world_file::header mHeader;
    file_read(mFile, mHeader);

    if (mHeader.sFileType[0] != 'W' ||
        mHeader.sFileType[1] != 'R' ||
        mHeader.sFileType[2] != 'L' ||
        mHeader.sFileType[3] != 'D')
    {
        throw utils::exception("world::load",
            sFile+ ":\n    "
            "wrong file format, expected 'WRLD', got '"+
            utils::to_string(mHeader.sFileType[0])+
            utils::to_string(mHeader.sFileType[1])+
            utils::to_string(mHeader.sFileType[2])+
            utils::to_string(mHeader.sFileType[3])+"'."
        );
    }

    if (mHeader.sVersion[0] != '0' ||
        mHeader.sVersion[1] != '0' ||
        mHeader.sVersion[2] != '0' ||
        mHeader.sVersion[3] != '1')
    {
        throw utils::exception("world::load",
            sFile+ ":\n    "
            "wrong file version, expected '0001', got '"+
            utils::to_string(mHeader.sVersion[0])+
            utils::to_string(mHeader.sVersion[1])+
            utils::to_string(mHeader.sVersion[2])+
            utils::to_string(mHeader.sVersion[3])+"'."
        );
    }

    if (mHeader.ucChunkSize != block_chunk::CHUNK_SIZE)
    {
        throw utils::exception("world::load",
            sFile+ ":\n    "
            "loaded world and current one have different chunk size : "+
            utils::to_string(mHeader.ucChunkSize)+" and "+utils::to_string(block_chunk::CHUNK_SIZE)+"."
        );
    }

    world_file::unit mUnit;
    for (uint i = 0; i < mHeader.uiNumUnit; ++i)
    {
        mFile.seekg(mHeader.pUnitOffset + i*sizeof(world_file::unit));
        if (mFile.eof())
            throw utils::exception("world::load", sFile+ ":\n    unexpected end of file, file corrupted ?");

        file_read(mFile, mUnit);

        utils::ustring sName;
        for (uint j = 0; j < world_file::unit::MAX_NAME_LENGTH; ++j)
        {
            if (mUnit.sName[j] == 0)
                break;

            sName.push_back(mUnit.sName[j]);
        }

        lUnitLookupList_.insert(sName);
    }

    set_file_(sFile);

    bJustLoaded_ = true;
    std::cout << "world : done.\n" << std::endl;

    bEmpty_ = false;
}

void world::load_chunk_(const chunk_id& id)
{
    static const float d2 = block_chunk::CHUNK_SIZE*block_chunk::CHUNK_SIZE;

    std::set<chunk_id>::iterator iter = lLoadingChunkList_.find(id);
    if (iter == lLoadingChunkList_.end())
    {
        const float coef = d2/(fRenderDistance_*fRenderDistance_);

        size_t priority = std::min(pLoadWorker_->get_num_queues()-1, (size_t)((pLoadWorker_->get_num_queues()-1)
            *(id.pos - pCurrentChunk_->get_coordinates()).get_norm_squared()*coef
        ));

        lLoadingChunkList_.insert(id);
        pLoadWorker_->add_task(priority, id);
    }
}

void world::load_chunk_no_check_(const chunk_id& id)
{
    static const float d2 = block_chunk::CHUNK_SIZE*block_chunk::CHUNK_SIZE;
    const float coef = d2/(fRenderDistance_*fRenderDistance_);

    size_t priority = std::min(pLoadWorker_->get_num_queues()-1, (size_t)((pLoadWorker_->get_num_queues()-1)
        *(id.pos - pCurrentChunk_->get_coordinates()).get_norm_squared()*coef
    ));

    lLoadingChunkList_.insert(id);
    pLoadWorker_->add_task(priority, id);
}

void world::unload_chunk_(const chunk_id& id)
{
    std::map<chunk_id, std::shared_ptr<block_chunk>>::iterator iter = lLoadedChunkList_.find(id);
    if (iter != lLoadedChunkList_.end())
    {
        unload_chunk_(iter->second);
        lLoadedChunkList_.erase(iter);
    }
}

void world::unload_chunk_(std::shared_ptr<block_chunk> pChunk)
{
    pChunk->flag_clear_links();
    pChunk->clear_vbos();
    save_chunk_(pChunk);
}

const double load_chunk_task::dPersistence[NUM_SETS] = {0.5, 0.4};
const double load_chunk_task::dFrequency[NUM_SETS] = {0.01, 0.05};
const uint   load_chunk_task::uiOctaves[NUM_SETS] = {5, 4};
const int    load_chunk_task::iSeed[NUM_SETS] = {2, 654};

load_chunk_task::load_chunk_task(utils::wptr<world> pWorld) : w(pWorld)
{
    for (size_t i = 0; i < NUM_SETS; ++i)
        noise[i] = {dPersistence[i], dFrequency[i], uiOctaves[i], w->get_seed() + iSeed[i]};
}

// Must return a number between 0 and 1
// d is in [-1;1]
double noise_function(const double& d)
{
    const double pi = acos(-1);
    double s = sin(d*0.5*pi);
    if (d < 0.0)
        return -0.25*s*s + 0.25;
    else
        return 0.75*s*s + 0.25;

    //return 0.5*(d + 1.0);
}

template<class T>
T clamp(const T& t, const T& tmin, const T& tmax)
{
    if (t < tmin)
        return tmin;
    if (t > tmax)
        return tmax;

    return t;
}

std::shared_ptr<block_chunk> load_chunk_task::operator () (const chunk_id& chunk_pos) const
{
    // TODO
    return generate_chunk(chunk_pos);
}

std::shared_ptr<block_chunk> load_chunk_task::generate_chunk(const chunk_id& chunk_pos) const
{
    static const int size = block_chunk::CHUNK_SIZE;
    static const int half = block_chunk::HALF_CHUNK_SIZE;

    std::shared_ptr<block_chunk> pChunk(new block_chunk(w, chunk_pos.pos));
    pChunk->set_self(pChunk);

    vector3i pos;
    for (pos.z = -half; pos.z <= half; ++pos.z)
    for (pos.x = -half; pos.x <= half; ++pos.x)
    {
        double dNoiseHeight = noise[0](chunk_pos.pos.x*size + pos.x, chunk_pos.pos.z*size + pos.z);
        double dHeight = 100.0*noise_function(dNoiseHeight) - 25.0;
        int height = dHeight;

        int height_distance = height - chunk_pos.pos.y*size;
        int water_distance = -chunk_pos.pos.y*size;

        double dNoiseStoneDepth = noise[1](chunk_pos.pos.x*size + pos.x, chunk_pos.pos.z*size + pos.z);
        double dStoneDepth = 9.0*dNoiseStoneDepth - 10.0;

        int stone_distance = height_distance + (int)dStoneDepth;

        for (pos.y = -half; pos.y <= half; ++pos.y)
        {
            if (pos.y < stone_distance)
                pChunk->set_block_fast(pos, block::STONE);
            else if (pos.y < height_distance)
                pChunk->set_block_fast(pos, block::DIRT);
            else if (pos.y == height_distance)
            {
                if (water_distance <= height_distance)
                {
                    if (height == water_distance)
                    {
                        double dSandFraction = dHeight - height;
                        if (dSandFraction < 0.00001)
                            pChunk->set_block_fast(pos, block::SAND);
                        else
                            pChunk->set_block_fast(pos, block::GRASS);
                    }
                    else
                        pChunk->set_block_fast(pos, block::GRASS);
                }
                else
                    pChunk->set_block_fast(pos, block::SAND);
            }
            else if (pos.y <= water_distance)
            {
                pChunk->set_block_fast(pos, block::WATER);
                pChunk->get_block(pos)->sunlight = 255u;
            }
            else
                pChunk->get_block(pos)->sunlight = 255u;
        }
    }

    /*for (pos.z = -half; pos.z <= half; ++pos.z)
    for (pos.x = -half; pos.x <= half; ++pos.x)
    for (pos.y = -half; pos.y <= half; ++pos.y)
    {
        if (pos.y + chunk_pos.pos.y*size < 0)
            pChunk->set_block_fast(pos, block::STONE);
        else
            pChunk->get_block(pos)->sunlight = 255u;
    }*/

    return pChunk;
}

void load_chunk_task::get_terrain_height(vector3f& pos) const
{
    double dNoiseHeight = noise[0](pos.x, pos.z);
    pos.y = int(100.0*noise_function(dNoiseHeight) - 25.0);
}

void world::receive_loaded_chunks_()
{
    std::shared_ptr<block_chunk> chunk;
    size_t uiNumReceivedChunk = 0u;

    while (uiNumReceivedChunk < uiNumReceivedChunkPerFrame_ && pLoadWorker_->consume(chunk))
    {
        receive_loaded_chunk_(chunk);
        ++uiNumReceivedChunk;
    }
}

void world::receive_loaded_chunk_(std::shared_ptr<block_chunk> chunk, bool immediate)
{
    add_chunk(chunk);
    lLoadingChunkList_.erase(chunk_id(chunk));

    if (immediate)
        update_chunk_immediate(chunk);
    /*else
        update_chunk_(chunk);*/
}

void world::save_chunk_(std::shared_ptr<block_chunk> chunk)
{
    // TODO
}

save_chunk_task::save_chunk_task(utils::wptr<world> pWorld) : w(pWorld)
{
    // TODO
}

void save_chunk_task::operator () (std::shared_ptr<block_chunk> chunk) const
{
    // TODO
}

void world::update_chunk_(std::weak_ptr<block_chunk> pWChunk) const
{
    update_chunk_immediate(pWChunk);
}

void world::update_chunk_immediate(std::weak_ptr<block_chunk> pWChunk) const
{
    std::shared_ptr<block_chunk> pChunk = pWChunk.lock();
    if (!pChunk)
        return;

    if (pChunk->bClearNeighborList_)
    {
        pChunk->clear_links();
        return;
    }

    if (pChunk->bUpdateCache_)
    {
        pChunk->update_cache();
        if (!bUseVBO_)
            pChunk->move_vertex_cache(-pChunk->get_position(pCurrentChunk_));
    }
    /*else if (pChunk->bUpdateLighting_)
    {
        pChunk->UpdateLighting();
        mVBOTask.bUpdate = true;
    }*/

    if (bSmoothLighting_ && pChunk->bBuildOcclusion_)
        pChunk->build_occlusion();

    if (bUseVBO_ && pChunk->bUpdateVBO_)
    {
        if (pChunk->mVertexCache_.uiNumNormalVertex != 0u)
        {
            pChunk->set_normal_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                0, pChunk->mVertexCache_.uiNumNormalVertex
            ));
        }

        if (pChunk->mVertexCache_.uiNumTwoSidedVertex != 0u)
        {
            pChunk->set_two_sided_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                pChunk->mVertexCache_.uiNumNormalVertex,
                pChunk->mVertexCache_.uiNumTwoSidedVertex
            ));
        }

        if (pChunk->mVertexCache_.uiNumAlphaBlendedVertex != 0u)
        {
            pChunk->set_alpha_blended_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                pChunk->mVertexCache_.uiNumNormalVertex +
                pChunk->mVertexCache_.uiNumTwoSidedVertex,
                pChunk->mVertexCache_.uiNumAlphaBlendedVertex
            ));
        }

        pChunk->bUpdateVBO_ = false;
    }
}

void world::update_chunk_vbo_immediate(std::weak_ptr<block_chunk> pWChunk) const
{
    std::shared_ptr<block_chunk> pChunk = pWChunk.lock();
    if (!pChunk)
        return;

    if (bUseVBO_ && pChunk->bUpdateVBO_)
    {
        if (pChunk->mVertexCache_.uiNumNormalVertex != 0u)
        {
            pChunk->set_normal_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                0, pChunk->mVertexCache_.uiNumNormalVertex
            ));
        }

        if (pChunk->mVertexCache_.uiNumTwoSidedVertex != 0u)
        {
            pChunk->set_two_sided_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                pChunk->mVertexCache_.uiNumNormalVertex,
                pChunk->mVertexCache_.uiNumTwoSidedVertex
            ));
        }

        if (pChunk->mVertexCache_.uiNumAlphaBlendedVertex != 0u)
        {
            pChunk->set_alpha_blended_vbo_data(pChunk->make_vbo_data_(
                pChunk->mVertexCache_.lData,
                pChunk->mVertexCache_.uiNumNormalVertex +
                pChunk->mVertexCache_.uiNumTwoSidedVertex,
                pChunk->mVertexCache_.uiNumAlphaBlendedVertex
            ));
        }

        pChunk->bUpdateVBO_ = false;
    }
}
