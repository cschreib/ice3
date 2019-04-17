#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <utils.hpp>
#include <vector>
#include <array>
#include "color.hpp"
#include "vector3.hpp"
#include "vertex.hpp"
#include "texture.hpp"
#include <utils_refptr.hpp>

class texture_manager;

void initialize_block_data(texture_manager& mTextureManager);

struct block_face;

struct block_data
{
    bool  bTransparent;
    bool  bWalkable;
    bool  bTwoSided;
    bool  bAlphaBlended;
    float fSolidity;
    uchar ucLightAtten;
};

class block
{
public :

    enum type
    {
        EMPTY        = 0,
        STONE        = 1,
        GRASS        = 2,
        DIRT         = 3,
        COBBLESTONE  = 4,
        WOODEN_PLANK = 5,
        WATER        = 8,
        WATER_SOURCE = 9,
        LAVA         = 10,
        LAVA_SOURCE  = 11,
        SAND         = 12,
        GRAVEL       = 13,
        GOLD_VEIN    = 14,
        IRON_VEIN    = 15,
        COAL_VEIN    = 16,
        WOOD         = 17,
        LEAVES       = 18,
        GLASS        = 20,
        BRICK        = 45,

        BLOCK_MAX    = 256
    };

    enum texture_id
    {
        // Special
        TEX_SELECTED     = -1,

        // Terrain.png
        TEX_GRASS_UP     = 0,
        TEX_STONE        = 1,
        TEX_DIRT         = 2,
        TEX_GRASS_SIDE   = 3,
        TEX_WOODEN_PLANK = 4,
        TEX_BRICK        = 7,
        TEX_COBBLESTONE  = 16,
        TEX_SAND         = 18,
        TEX_GRAVEL       = 19,
        TEX_WOOD_SIDE    = 20,
        TEX_WOOD_UP      = 21,
        TEX_GOLD_VEIN    = 32,
        TEX_IRON_VEIN    = 33,
        TEX_COAL_VEIN    = 34,
        TEX_GLASS        = 49,
        TEX_LEAVES       = 52,
        TEX_WATER        = 223,
        TEX_LAVA         = 255,

        TEX_DAMAGE_1  = 240,
        TEX_DAMAGE_2  = 241,
        TEX_DAMAGE_3  = 242,
        TEX_DAMAGE_4  = 243,
        TEX_DAMAGE_5  = 244,
        TEX_DAMAGE_6  = 245,
        TEX_DAMAGE_7  = 246,
        TEX_DAMAGE_8  = 247,
        TEX_DAMAGE_9  = 248,
        TEX_DAMAGE_10 = 249,

        TEX_MAX       = 256
    };

    enum face
    {
        LEFT = 0,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
    };

    enum corner
    {
        BACK_BOTTOM_LEFT = 0,
        BACK_BOTTOM_RIGHT,
        BACK_TOP_LEFT,
        BACK_TOP_RIGHT,
        FRONT_BOTTOM_LEFT,
        FRONT_BOTTOM_RIGHT,
        FRONT_TOP_LEFT,
        FRONT_TOP_RIGHT
    };

    block();
    ~block();

    inline texture_id get_texture(face mFace) const {
        return TEXTURE_LIST[t][mFace];
    }

    inline bool is_transparent() const {
        return BLOCK_DATA[t].bTransparent;
    }

    inline bool is_walkable() const {
        return BLOCK_DATA[t].bWalkable;
    }

    inline bool is_two_sided() const {
        return BLOCK_DATA[t].bTwoSided;
    }

    inline bool is_alpha_blended() const {
        return BLOCK_DATA[t].bAlphaBlended;
    }

    inline float get_solidity() const {
        //return BLOCK_DATA[t].fSolidity;
        return 1.0f;
    }

    inline uchar get_light_attenuation() const {
        return BLOCK_DATA[t].ucLightAtten/**Light::ATTEN*/;
    }

    void add_face(const vector3f& mBlockPos, face mFace, const block* pNextBlock, uchar ucHue, std::vector<block_face>& lArray) const;
    void add_face(const vector3f& mBlockPos, face mFace, const block* pNextBlock, uchar ucHue, texture_id mTex, std::vector<block_face>& lArray) const;

    static utils::refptr<texture>                                 BLOCK_TEXTURE;
    static utils::refptr<texture>                                 BLOCK_SELECTED_TEXTURE;
    static const float                                            BLOCK_TEXTURE_SIZE;
    static std::array<color,256>                                  HUE_TABLE;
    static const std::array<std::array<block::corner,4>,6>        CORNER_LIST;
    static const std::array<std::array<std::array<uchar,8>,4>,6>  OCCLUSION_SELECT_LIST;
    static const std::array<std::array<block::face,7>,8>          OCCLUSION_LIST1;
    static const std::array<std::array<block::corner,8>,8>        OCCLUSION_LIST2;
    static const std::array<float,256>                            OCCLUSION_TABLE1;
    static const std::array<float,256>                            OCCLUSION_TABLE2;
    static const std::array<vector3f,6>                           NORMAL_ARRAY;
    static const std::array<std::string,6>                        FACE_NAME_ARRAY;
    static const std::array<block::face,6>                        OPPOSED_LIST;
    static const std::array<std::array<uv_coordinates,4>,TEX_MAX> UV_LIST;
    static const std::array<std::array<texture_id,6>,BLOCK_MAX>   TEXTURE_LIST;
    static const std::array<block_data,block::BLOCK_MAX>          BLOCK_DATA;

    static block::texture_id get_texture(block::type mType, block::face mFace) {
        return TEXTURE_LIST[mType][mFace];
    }

    inline static bool is_transparent(block::type mType) {
        return BLOCK_DATA[mType].bTransparent;
    }

    inline static bool is_walkable(block::type mType) {
        return BLOCK_DATA[mType].bWalkable;
    }

    inline static bool is_two_sided(block::type mType) {
        return BLOCK_DATA[mType].bTwoSided;
    }

    inline static bool is_alpha_blended(block::type mType) {
        return BLOCK_DATA[mType].bAlphaBlended;
    }

    inline static float get_solidity(block::type mType) {
        //return BLOCK_DATA[mType].fSolidity;
        return 1.0f;
    }

    inline static uv_coordinates get_uv(block::texture_id mTex, uint i) {
        return UV_LIST[(int)mTex][i];
    }

    inline static uchar get_light_attenuation(block::type mType) {
        return BLOCK_DATA[mType].ucLightAtten/**Light::ATTEN*/;
    }

    uchar t;

    uchar sunlight;
    uchar light;
    uchar selflight;

    bool open;

private :

    static std::array<color,256>                                        make_hue_table_();
    static std::array<std::array<uv_coordinates,4>,block::TEX_MAX>      make_uv_list_();
    static std::array<std::array<block::texture_id,6>,block::BLOCK_MAX> make_texture_list_();
    static std::array<block_data,block::BLOCK_MAX>                      make_block_data_();
    static std::array<float,256>                                        make_occlusion_table1_();
    static std::array<float,256>                                        make_occlusion_table2_();
};

struct block_face
{
    mutable block* b;
    block::face    face;

    vertex v1, v2, v3, v4;
    uchar  sunlight, light, hue;
};

#endif
