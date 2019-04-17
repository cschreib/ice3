#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <utils.hpp>
#include "movable.hpp"
#include <memory>
#include <array>

class block_chunk;
class block;

class light : public movable
{
public :

    light(const std::string& sName, utils::wptr<world> pWorld, std::weak_ptr<block_chunk> pChunk, block* pBlock);
    ~light();

    void set_self(utils::wptr<movable> pMovable) override;

    const std::string& get_name() const { return sName_; }

    std::weak_ptr<const block_chunk> get_chunk() const { return pChunk_; }
    std::weak_ptr<block_chunk>       get_chunk() { return pChunk_; }
    const block*                     get_block() const { return pBlock_; }
    block*                           get_block() { return pBlock_; }

    void  set_intensity(uchar ucIntensity);
    uchar get_intensity() const;

    static const uchar                 ATTEN;
    static const std::array<uint,256>  RANGE_TABLE;
    static const std::array<float,256> INTENSITY_TABLE;

private :

    virtual void on_moved_(movable::movement_type mType);

    utils::wptr<light> pLightSelf_;

    std::string                sName_;
    std::weak_ptr<block_chunk> pChunk_;
    block*                     pBlock_;

    uchar ucIntensity_;

};

#endif