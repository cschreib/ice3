#include "world.hpp"
#include <luapp.hpp>

int l_load_chunk(lua_State* pLua)
{
    lua::function mFunc("load_chunk", pLua);
    mFunc.add(0, "x", lua::TYPE_NUMBER);
    mFunc.add(1, "y", lua::TYPE_NUMBER);
    mFunc.add(2, "z", lua::TYPE_NUMBER);
    if (mFunc.check())
    {
        mFunc.get_state()->get_global("_WRLD");
        world* pWorld = mFunc.get_state()->get_userdata<world>();

        pWorld->load_chunk_(chunk_id(vector3i(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number(),
            mFunc.get(2)->get_number()
        )));
    }

    return mFunc.on_return();
}

int l_unload_chunk(lua_State* pLua)
{
    lua::function mFunc("unload_chunk", pLua);
    mFunc.add(0, "x", lua::TYPE_NUMBER);
    mFunc.add(1, "y", lua::TYPE_NUMBER);
    mFunc.add(2, "z", lua::TYPE_NUMBER);
    if (mFunc.check())
    {
        mFunc.get_state()->get_global("_WRLD");
        world* pWorld = mFunc.get_state()->get_userdata<world>();

        pWorld->unload_chunk_(chunk_id(vector3i(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number(),
            mFunc.get(2)->get_number()
        )));
    }

    return mFunc.on_return();
}

int l_update_chunk(lua_State* pLua)
{
    lua::function mFunc("reload_chunk", pLua);
    mFunc.add(0, "x", lua::TYPE_NUMBER);
    mFunc.add(1, "y", lua::TYPE_NUMBER);
    mFunc.add(2, "z", lua::TYPE_NUMBER);
    if (mFunc.check())
    {
        mFunc.get_state()->get_global("_WRLD");
        world* pWorld = mFunc.get_state()->get_userdata<world>();

        chunk_id mID(vector3i(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number(),
            mFunc.get(2)->get_number()
        ));
        pWorld->unload_chunk_(mID);
        pWorld->load_chunk_(mID);
    }

    return mFunc.on_return();
}

int l_get_render_stats(lua_State* pLua)
{
    lua::state* pState = lua::state::get_state(pLua);

    pState->get_global("_WRLD");
    world* pWorld = pState->get_userdata<world>();
    pState->pop();

    const world::render_data& mData = pWorld->get_render_data();
    pState->push_number(mData.uiNumRenderedChunk);
    pState->push_number(mData.uiNumChunk);
    pState->push_number(mData.uiNumRenderedVertex);
    pState->push_number(mData.fRenderTime);
    pState->push_number(mData.fUpdateTime);
    pState->push_number(mData.uiUpdateQueuedChunkCount);
    pState->push_number(mData.uiCreateQueuedChunkCount);

    return 7;
}

int l_get_render_states(lua_State* pLua)
{
    lua::state* pState = lua::state::get_state(pLua);

    pState->get_global("_WRLD");
    world* pWorld = pState->get_userdata<world>();
    pState->pop();

    pState->push_string(utils::to_string(pWorld->is_smooth_lighting_enabled()));
    pState->push_string(utils::to_string(pWorld->are_vbos_enabled()));
    pState->push_string(utils::to_string(pWorld->are_shaders_enabled()));

    return 3;
}

int l_get_block_uv(lua_State* pLua)
{
    lua::function mFunc("get_block_uv", pLua, 4);
    mFunc.add(0, "block_id", lua::TYPE_NUMBER);

    if (mFunc.check())
    {
        block::type mType = (block::type)mFunc.get(0)->get_number();
        uv_coordinates uv = block::get_uv(block::get_texture(mType, block::TOP), 0);

        mFunc.push(uv.u);
        mFunc.push(uv.v);
        mFunc.push(uv.u+1.0f/16.0f);
        mFunc.push(uv.v+1.0f/16.0f);
    }

    return mFunc.on_return();
}

int l_get_available_world_name(lua_State* pLua)
{
    return 0;
}

int l_new_world(lua_State* pLua)
{
    return 0;
}

int l_open_world(lua_State* pLua)
{
    return 0;
}

int l_get_selected_block_pos(lua_State* pLua)
{
    lua::state* pState = lua::state::get_state(pLua);

    pState->get_global("_WRLD");
    world* pWorld = pState->get_userdata<world>();
    pState->pop();

    block* pBlock = pWorld->get_selected_block();
    if (pBlock)
    {
        std::shared_ptr<block_chunk> pChunk = pWorld->get_selected_block_chunk().lock();
        vector3i mChunkPos = pChunk->get_coordinates();
        vector3i mBlockPos = pChunk->get_block_position(pBlock);
        pState->push_number(mChunkPos.x);
        pState->push_number(mChunkPos.y);
        pState->push_number(mChunkPos.z);
        pState->push_number(mBlockPos.x);
        pState->push_number(mBlockPos.y);
        pState->push_number(mBlockPos.z);
    }
    else
        pState->push_nil(6);

    return 6;
}

int l_get_current_unit_pos(lua_State* pLua)
{
    lua::state* pState = lua::state::get_state(pLua);

    pState->get_global("_WRLD");
    world* pWorld = pState->get_userdata<world>();
    pState->pop();

    utils::refptr<unit> pUnit = pWorld->get_current_unit().lock();
    if (!pUnit)
    {
        pState->push_nil(6);
        return 6;
    }

    std::shared_ptr<block_chunk> pChunk = pUnit->get_chunk().lock();
    if (!pChunk)
    {
        pState->push_nil(6);
        return 6;
    }

    vector3i mChunkPos = pChunk->get_coordinates();
    block* pBlock = pUnit->get_block();
    vector3i mBlockPos = pChunk->get_block_position(pBlock);
    pState->push_number(mChunkPos.x);
    pState->push_number(mChunkPos.y);
    pState->push_number(mChunkPos.z);
    pState->push_number(mBlockPos.x);
    pState->push_number(mBlockPos.y);
    pState->push_number(mBlockPos.z);

    return 6;
}

int l_show_mouse_cursor(lua_State* pLua)
{
    return 0;
}

int l_save_bindings(lua_State* pLua)
{
    return 0;
}

int l_get_key_string(lua_State* pLua)
{
    return 0;
}

void register_glues(lua::state& mState)
{
    mState.reg("get_render_stats", l_get_render_stats);
    mState.reg("get_block_uv", l_get_block_uv);
    mState.reg("get_available_world_name", l_get_available_world_name);
    mState.reg("new_world", l_new_world);
    mState.reg("open_world", l_open_world);
    mState.reg("get_render_states", l_get_render_states);
    mState.reg("get_selected_block_pos", l_get_selected_block_pos);
    mState.reg("get_current_unit_pos", l_get_current_unit_pos);
    mState.reg("show_mouse_cursor", l_show_mouse_cursor);
    mState.reg("save_bindings", l_save_bindings);
    mState.reg("get_key_string", l_get_key_string);
    mState.reg("load_chunk", l_load_chunk);
    mState.reg("unload_chunk", l_unload_chunk);
}
