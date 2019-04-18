#include "block.hpp"
#include "texture_manager.hpp"
#include "texture.hpp"

std::array<color,256> block::make_hue_table_()
{
    std::array<color,256> lArray;
    // NOTE : Cannot use color::WHITE because it may not be initialized yet...
    lArray[0] = color(1.0f, 1.0f, 1.0f);

    for (int i = 1; i < 256; ++i)
        lArray[i] = (i-1)/254.0f*color(0.6f, 1.0f, 0.4f) + (255-i)/254.0f*color(1.0f, 1.0f, 0.4f);

    return lArray;
}

std::array<color,256> block::HUE_TABLE = make_hue_table_();

utils::refptr<texture> block::BLOCK_TEXTURE;
utils::refptr<texture> block::BLOCK_SELECTED_TEXTURE;

const std::array<std::array<block::corner,4>,6> block::CORNER_LIST = {{
    {{block::FRONT_BOTTOM_LEFT,  block::FRONT_TOP_LEFT,     block::BACK_TOP_LEFT,      block::BACK_BOTTOM_LEFT}},  // LEFT
    {{block::FRONT_BOTTOM_RIGHT, block::BACK_BOTTOM_RIGHT,  block::BACK_TOP_RIGHT,     block::FRONT_TOP_RIGHT}},   // RIGHT
    {{block::FRONT_BOTTOM_LEFT,  block::FRONT_BOTTOM_RIGHT, block::FRONT_TOP_RIGHT,    block::FRONT_TOP_LEFT}},    // FRONT
    {{block::BACK_BOTTOM_LEFT,   block::BACK_TOP_LEFT,      block::BACK_TOP_RIGHT,     block::BACK_BOTTOM_RIGHT}}, // BACK
    {{block::BACK_TOP_LEFT,      block::FRONT_TOP_LEFT,     block::FRONT_TOP_RIGHT,    block::BACK_TOP_RIGHT}},    // TOP
    {{block::BACK_BOTTOM_LEFT,   block::BACK_BOTTOM_RIGHT,  block::FRONT_BOTTOM_RIGHT, block::FRONT_BOTTOM_LEFT}}  // BOTTOM
}};

// Details :
//   1 : contributes to smooth lighting only
//   2 : contributes to occlusion and smooth lighting
//   3 : contributes to occlusion only if no 2
//   0 : doesn't contribute
const std::array<std::array<std::array<uchar,8>,4>,6> block::OCCLUSION_SELECT_LIST = {{
    {{   // LEFT
        {{2, 0, 1, 0, 3, 0, 2, 0}}, // FRONT_BOTTOM_LEFT
        {{1, 0, 2, 0, 2, 0, 3, 0}}, // FRONT_TOP_LEFT
        {{2, 0, 3, 0, 1, 0, 2, 0}}, // BACK_TOP_LEFT
        {{3, 0, 2, 0, 2, 0, 1, 0}}  // BACK_BOTTOM_LEFT
    }},
    {{   // RIGHT
        {{0, 2, 0, 1, 0, 3, 0, 2}}, // FRONT_BOTTOM_RIGHT
        {{0, 3, 0, 2, 0, 2, 0, 1}}, // BACK_BOTTOM_RIGHT
        {{0, 2, 0, 3, 0, 1, 0, 2}}, // BACK_TOP_RIGHT
        {{0, 1, 0, 2, 0, 2, 0, 3}}  // FRONT_TOP_RIGHT
    }},
    {{   // FRONT
        {{0, 0, 0, 0, 3, 2, 2, 1}}, // FRONT_BOTTOM_LEFT
        {{0, 0, 0, 0, 2, 3, 1, 2}}, // FRONT_BOTTOM_RIGHT
        {{0, 0, 0, 0, 1, 2, 2, 3}}, // FRONT_TOP_RIGHT
        {{0, 0, 0, 0, 2, 1, 3, 2}}  // FRONT_TOP_LEFT
    }},
    {{   // BACK
        {{3, 2, 2, 1, 0, 0, 0, 0}}, // BACK_BOTTOM_LEFT
        {{2, 1, 3, 2, 0, 0, 0, 0}}, // BACK_TOP_LEFT
        {{1, 2, 2, 3, 0, 0, 0, 0}}, // BACK_TOP_RIGHT
        {{2, 3, 1, 2, 0, 0, 0, 0}}  // BACK_BOTTOM_RIGHT
    }},
    {{   // TOP
        {{0, 0, 3, 2, 0, 0, 2, 1}}, // BACK_TOP_LEFT
        {{0, 0, 2, 1, 0, 0, 3, 2}}, // FRONT_TOP_LEFT
        {{0, 0, 1, 2, 0, 0, 2, 3}}, // FRONT_TOP_RIGHT
        {{0, 0, 2, 3, 0, 0, 1, 2}}  // BACK_TOP_RIGHT
    }},
    {{   // BOTTOM
        {{3, 2, 0, 0, 2, 1, 0, 0}}, // BACK_BOTTOM_LEFT
        {{2, 3, 0, 0, 1, 2, 0, 0}}, // BACK_BOTTOM_RIGHT
        {{1, 2, 0, 0, 2, 3, 0, 0}}, // FRONT_BOTTOM_RIGHT
        {{2, 1, 0, 0, 3, 2, 0, 0}}  // FRONT_BOTTOM_LEFT
    }}
}};

const std::array<std::array<block::face,7>,8> block::OCCLUSION_LIST1 = {{
    {{block::BACK,  block::BOTTOM, block::LEFT,  block::BOTTOM, block::LEFT,  block::BACK,  block::LEFT}},  // BACK_BOTTOM_LEFT
    {{block::BACK,  block::BOTTOM, block::RIGHT, block::BOTTOM, block::RIGHT, block::BACK,  block::RIGHT}}, // BACK_BOTTOM_RIGHT
    {{block::BACK,  block::TOP,    block::LEFT,  block::TOP,    block::LEFT,  block::BACK,  block::LEFT}},  // BACK_TOP_LEFT
    {{block::BACK,  block::TOP,    block::RIGHT, block::TOP,    block::RIGHT, block::BACK,  block::RIGHT}}, // BACK_TOP_RIGHT
    {{block::FRONT, block::BOTTOM, block::LEFT,  block::BOTTOM, block::LEFT,  block::FRONT, block::LEFT}},  // FRONT_BOTTOM_LEFT
    {{block::FRONT, block::BOTTOM, block::RIGHT, block::BOTTOM, block::RIGHT, block::FRONT, block::RIGHT}}, // FRONT_BOTTOM_RIGHT
    {{block::FRONT, block::TOP,    block::LEFT,  block::TOP,    block::LEFT,  block::FRONT, block::LEFT}},  // FRONT_TOP_LEFT
    {{block::FRONT, block::TOP,    block::RIGHT, block::TOP,    block::RIGHT, block::FRONT, block::RIGHT}}  // FRONT_TOP_RIGHT
}};

const std::array<std::array<block::corner,8>,8> block::OCCLUSION_LIST2 = {{
    {{block::FRONT_TOP_RIGHT,    block::BACK_TOP_RIGHT,     block::FRONT_BOTTOM_RIGHT, block::FRONT_TOP_LEFT,
      block::BACK_BOTTOM_RIGHT,  block::FRONT_BOTTOM_LEFT,  block::BACK_TOP_LEFT,      block::BACK_BOTTOM_LEFT}},   // BACK_BOTTOM_LEFT
    {{block::FRONT_TOP_LEFT,     block::BACK_TOP_LEFT,      block::FRONT_BOTTOM_LEFT,  block::FRONT_TOP_RIGHT,
      block::BACK_BOTTOM_LEFT,   block::FRONT_BOTTOM_RIGHT, block::BACK_TOP_RIGHT,     block::BACK_BOTTOM_RIGHT}},  // BACK_BOTTOM_RIGHT
    {{block::FRONT_BOTTOM_RIGHT, block::BACK_BOTTOM_RIGHT,  block::FRONT_TOP_RIGHT,    block::FRONT_BOTTOM_LEFT,
      block::BACK_TOP_RIGHT,     block::FRONT_TOP_LEFT,     block::BACK_BOTTOM_LEFT,   block::BACK_TOP_LEFT}},      // BACK_TOP_LEFT
    {{block::FRONT_BOTTOM_LEFT,  block::BACK_BOTTOM_LEFT,   block::FRONT_TOP_LEFT,     block::FRONT_BOTTOM_RIGHT,
      block::BACK_TOP_LEFT,      block::FRONT_TOP_RIGHT,    block::BACK_BOTTOM_RIGHT,  block::BACK_TOP_RIGHT}},     // BACK_TOP_RIGHT
    {{block::BACK_TOP_RIGHT,     block::FRONT_TOP_RIGHT,    block::BACK_BOTTOM_RIGHT,  block::BACK_TOP_LEFT,
      block::FRONT_BOTTOM_RIGHT, block::BACK_BOTTOM_LEFT,   block::FRONT_TOP_LEFT,     block::FRONT_BOTTOM_LEFT}},  // FRONT_BOTTOM_LEFT
    {{block::BACK_TOP_LEFT,      block::FRONT_TOP_LEFT,     block::BACK_BOTTOM_LEFT,   block::BACK_TOP_RIGHT,
      block::FRONT_BOTTOM_LEFT,  block::BACK_BOTTOM_RIGHT,  block::FRONT_TOP_RIGHT,    block::FRONT_BOTTOM_RIGHT}}, // FRONT_BOTTOM_RIGHT
    {{block::BACK_BOTTOM_RIGHT,  block::FRONT_BOTTOM_RIGHT, block::BACK_TOP_RIGHT,     block::BACK_BOTTOM_LEFT,
      block::FRONT_TOP_RIGHT,    block::BACK_TOP_LEFT,      block::FRONT_BOTTOM_LEFT,  block::FRONT_TOP_LEFT}},     // FRONT_TOP_LEFT
    {{block::BACK_BOTTOM_LEFT,   block::FRONT_BOTTOM_LEFT,  block::BACK_TOP_LEFT,      block::BACK_BOTTOM_RIGHT,
      block::FRONT_TOP_LEFT,     block::BACK_TOP_RIGHT,     block::FRONT_BOTTOM_RIGHT, block::FRONT_TOP_RIGHT}}     // FRONT_TOP_RIGHT
}};

std::array<float,256> block::make_occlusion_table1_()
{
    const float fMinLight = 0.25f;

    std::array<float,256> lArray;
    for (int i = 0; i < 256; ++i)
        lArray[i] = ((255-i)/255.0f)*(1.0f-fMinLight) + fMinLight;

    return lArray;
}

const std::array<float,256> block::OCCLUSION_TABLE1 = block::make_occlusion_table1_();

std::array<float,256> block::make_occlusion_table2_()
{
    std::array<float,256> lArray;
    for (int i = 0; i < 256; ++i)
        lArray[i] = i/255.0f;

    return lArray;
}

const std::array<float,256> block::OCCLUSION_TABLE2 = block::make_occlusion_table2_();

const std::array<vector3f,6> block::NORMAL_ARRAY = {{
   vector3f(-1, 0, 0), vector3f(+1, 0, 0),
   vector3f( 0, 0,-1), vector3f( 0, 0,+1),
   vector3f( 0,+1, 0), vector3f( 0,-1, 0)
}};

const std::array<std::string,6> block::FACE_NAME_ARRAY = {{
    "LEFT", "RIGHT", "FRONT", "BACK", "TOP", "BOTTOM"
}};

const std::array<block::face,6> block::OPPOSED_LIST = {{
    block::RIGHT, block::LEFT,   block::BACK,
    block::FRONT, block::BOTTOM, block::TOP
}};

const float block::BLOCK_TEXTURE_SIZE = 1.0f/16.0f;

std::array<std::array<uv_coordinates,4>,block::TEX_MAX> block::make_uv_list_()
{
    std::array<std::array<uv_coordinates,4>,block::TEX_MAX> data;

    for (size_t tex = 0; tex < 256; ++tex)
    {
        float x = tex % 16;
        float y = tex / 16;

        data[tex] = {{
            uv_coordinates((x+0.000f)*block::BLOCK_TEXTURE_SIZE, (y+0.000f)*block::BLOCK_TEXTURE_SIZE),
            uv_coordinates((x+0.999f)*block::BLOCK_TEXTURE_SIZE, (y+0.000f)*block::BLOCK_TEXTURE_SIZE),
            uv_coordinates((x+0.999f)*block::BLOCK_TEXTURE_SIZE, (y+0.999f)*block::BLOCK_TEXTURE_SIZE),
            uv_coordinates((x+0.000f)*block::BLOCK_TEXTURE_SIZE, (y+0.999f)*block::BLOCK_TEXTURE_SIZE)
        }};
    }

    return data;
}

const std::array<std::array<uv_coordinates,4>,block::TEX_MAX> block::UV_LIST = make_uv_list_();

std::array<std::array<block::texture_id,6>,block::BLOCK_MAX> block::make_texture_list_()
{
    std::array<std::array<block::texture_id,6>,block::BLOCK_MAX> data;

    data[STONE] =
        {{TEX_STONE,        TEX_STONE,        TEX_STONE,        TEX_STONE,        TEX_STONE,        TEX_STONE}};
    data[DIRT] =
        {{TEX_DIRT,         TEX_DIRT,         TEX_DIRT,         TEX_DIRT,         TEX_DIRT,         TEX_DIRT}};
    data[COBBLESTONE] =
        {{TEX_COBBLESTONE,  TEX_COBBLESTONE,  TEX_COBBLESTONE,  TEX_COBBLESTONE,  TEX_COBBLESTONE,  TEX_COBBLESTONE}};
    data[WOODEN_PLANK] =
        {{TEX_WOODEN_PLANK, TEX_WOODEN_PLANK, TEX_WOODEN_PLANK, TEX_WOODEN_PLANK, TEX_WOODEN_PLANK, TEX_WOODEN_PLANK}};
    data[WATER] =
        {{TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER}};
    data[WATER_SOURCE] =
        {{TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER,        TEX_WATER}};
    data[LAVA] =
        {{TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA}};
    data[LAVA_SOURCE] =
        {{TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA,         TEX_LAVA}};
    data[SAND] =
        {{TEX_SAND,         TEX_SAND,         TEX_SAND,         TEX_SAND,         TEX_SAND,         TEX_SAND}};
    data[GRAVEL] =
        {{TEX_GRAVEL,       TEX_GRAVEL,       TEX_GRAVEL,       TEX_GRAVEL,       TEX_GRAVEL,       TEX_GRAVEL}};
    data[GOLD_VEIN] =
        {{TEX_GOLD_VEIN,    TEX_GOLD_VEIN,    TEX_GOLD_VEIN,    TEX_GOLD_VEIN,    TEX_GOLD_VEIN,    TEX_GOLD_VEIN}};
    data[IRON_VEIN] =
        {{TEX_IRON_VEIN,    TEX_IRON_VEIN,    TEX_IRON_VEIN,    TEX_IRON_VEIN,    TEX_IRON_VEIN,    TEX_IRON_VEIN}};
    data[COAL_VEIN] =
        {{TEX_COAL_VEIN,    TEX_COAL_VEIN,    TEX_COAL_VEIN,    TEX_COAL_VEIN,    TEX_COAL_VEIN,    TEX_COAL_VEIN}};
    data[LEAVES] =
        {{TEX_LEAVES,       TEX_LEAVES,       TEX_LEAVES,       TEX_LEAVES,       TEX_LEAVES,       TEX_LEAVES}};
    data[GLASS] =
        {{TEX_GLASS,        TEX_GLASS,        TEX_GLASS,        TEX_GLASS,        TEX_GLASS,        TEX_GLASS}};
    data[BRICK] =
        {{TEX_BRICK,        TEX_BRICK,        TEX_BRICK,        TEX_BRICK,        TEX_BRICK,        TEX_BRICK}};
    data[GRASS] =
        {{TEX_GRASS_SIDE,   TEX_GRASS_SIDE,   TEX_GRASS_SIDE,   TEX_GRASS_SIDE,   TEX_GRASS_UP,     TEX_DIRT}};
    data[WOOD] =
        {{TEX_WOOD_SIDE,    TEX_WOOD_SIDE,    TEX_WOOD_SIDE,    TEX_WOOD_SIDE,    TEX_WOOD_UP,      TEX_WOOD_SIDE}};

    return data;
}

const std::array<std::array<block::texture_id,6>,block::BLOCK_MAX> block::TEXTURE_LIST = make_texture_list_();

std::array<block_data,block::BLOCK_MAX> block::make_block_data_()
{
    std::array<block_data,block::BLOCK_MAX> data;

    //                    Trans, Walk, 2Sided, Alpha, Solid, LightAtt
    data[EMPTY]        = {true,  true,  false, false, 0.0f,   1};
    data[STONE]        = {false, false, false, false, 30.0f,  0};
    data[GRASS]        = {false, false, false, false, 3.0f,   0};
    data[DIRT]         = {false, false, false, false, 2.5f,   0};
    data[COBBLESTONE]  = {false, false, false, false, 30.0f,  0};
    data[WOODEN_PLANK] = {false, false, false, false, 15.0f,  0};
    data[WATER]        = {true,  true,  false, true,  500.0f, 2};
    data[WATER_SOURCE] = {true,  true,  false, true,  500.0f, 2};
    data[LAVA]         = {false, true,  false, false, 500.0f, 0};
    data[LAVA_SOURCE]  = {false, true,  false, false, 500.0f, 0};
    data[SAND]         = {false, false, false, false, 2.5f,   0};
    data[GRAVEL]       = {false, false, false, false, 3.0f,   0};
    data[GOLD_VEIN]    = {false, false, false, false, 15.0f,  0};
    data[IRON_VEIN]    = {false, false, false, false, 15.0f,  0};
    data[COAL_VEIN]    = {false, false, false, false, 15.0f,  0};
    data[WOOD]         = {false, false, false, false, 10.0f,  0};
    data[LEAVES]       = {true,  false, true,  false, 1.0f,   1};
    data[GLASS]        = {true,  false, true,  false, 1.5f,   1};
    data[BRICK]        = {false, false, false, false, 30.0f,  0};

    return data;
}

const std::array<block_data,block::BLOCK_MAX> block::BLOCK_DATA = block::make_block_data_();

void initialize_block_data(texture_manager& mTextureManager)
{
    block::BLOCK_TEXTURE          = mTextureManager.load_texture("textures/terrain.png");
    block::BLOCK_SELECTED_TEXTURE = mTextureManager.load_texture("textures/selected.png");
}

block::block() : t(EMPTY), sunlight(0u), light(0u), selflight(0u), open(false)
{
}

block::~block()
{
}

void block::add_face(const vector3f& mBlockPos, block::face mFace, const block* pNextBlock, uchar ucHue, std::vector<block_face>& lArray) const
{
    add_face(mBlockPos, mFace, pNextBlock, ucHue, get_texture(mFace), lArray);
}

void block::add_face(const vector3f& mBlockPos, block::face mFace, const block* pNextBlock, uchar ucHue, block::texture_id mTex, std::vector<block_face>& lArray) const
{
    static const vector3f mmm = vector3f(-0.500f, -0.500f, -0.500f);
    static const vector3f ppp = vector3f( 0.500f,  0.500f,  0.500f);

    static const vector3f mpm = vector3f(-0.500f,  0.500f, -0.500f);
    static const vector3f mmp = vector3f(-0.500f, -0.500f,  0.500f);
    static const vector3f mpp = vector3f(-0.500f,  0.500f,  0.500f);

    static const vector3f pmm = vector3f( 0.500f, -0.500f, -0.500f);
    static const vector3f pmp = vector3f( 0.500f, -0.500f,  0.500f);

    static const vector3f ppm = vector3f( 0.500f,  0.500f, -0.500f);

    block_face mBlockFace;
    mBlockFace.face = mFace;
    mBlockFace.b = const_cast<block*>(this);
    mBlockFace.hue = ucHue;
    mBlockFace.light = pNextBlock->light;
    mBlockFace.sunlight = pNextBlock->sunlight;

    switch (mFace)
    {
        case block::LEFT :
            mBlockFace.v1 = vertex(mmm + mBlockPos, block::get_uv(mTex, 3));
            mBlockFace.v2 = vertex(mpm + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v3 = vertex(mpp + mBlockPos, block::get_uv(mTex, 1));
            mBlockFace.v4 = vertex(mmp + mBlockPos, block::get_uv(mTex, 2));
            break;

        case block::RIGHT :
            mBlockFace.v1 = vertex(pmm + mBlockPos, block::get_uv(mTex, 2));
            mBlockFace.v2 = vertex(pmp + mBlockPos, block::get_uv(mTex, 3));
            mBlockFace.v3 = vertex(ppp + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v4 = vertex(ppm + mBlockPos, block::get_uv(mTex, 1));
            break;

        case block::FRONT :
            mBlockFace.v1 = vertex(mmm + mBlockPos, block::get_uv(mTex, 2));
            mBlockFace.v2 = vertex(pmm + mBlockPos, block::get_uv(mTex, 3));
            mBlockFace.v3 = vertex(ppm + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v4 = vertex(mpm + mBlockPos, block::get_uv(mTex, 1));
            break;

        case block::BACK :
            mBlockFace.v1 = vertex(mmp + mBlockPos, block::get_uv(mTex, 3));
            mBlockFace.v2 = vertex(mpp + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v3 = vertex(ppp + mBlockPos, block::get_uv(mTex, 1));
            mBlockFace.v4 = vertex(pmp + mBlockPos, block::get_uv(mTex, 2));
            break;

        case block::TOP :
            mBlockFace.v1 = vertex(mpp + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v2 = vertex(mpm + mBlockPos, block::get_uv(mTex, 1));
            mBlockFace.v3 = vertex(ppm + mBlockPos, block::get_uv(mTex, 2));
            mBlockFace.v4 = vertex(ppp + mBlockPos, block::get_uv(mTex, 3));
            break;

        case block::BOTTOM :
            mBlockFace.v1 = vertex(mmp + mBlockPos, block::get_uv(mTex, 0));
            mBlockFace.v2 = vertex(pmp + mBlockPos, block::get_uv(mTex, 1));
            mBlockFace.v3 = vertex(pmm + mBlockPos, block::get_uv(mTex, 2));
            mBlockFace.v4 = vertex(mmm + mBlockPos, block::get_uv(mTex, 3));
            break;

        default : return;
    }

    lArray.push_back(mBlockFace);
}
