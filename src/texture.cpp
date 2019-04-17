#include "texture.hpp"
#include "texture_manager.hpp"
#include <GL/gl.h>

texture::texture() : uiWidth_(0u), uiHeight_(0u)
{
}

texture::texture(uint uiWidth, uint uiHeight, wrap mWrap, filter mFilter) :
    uiWidth_(uiWidth), uiHeight_(uiHeight)
{
    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glGenTextures(1, &uiTextureHandle_);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, uiWidth_, uiHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    switch (mWrap)
    {
    case CLAMP :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        break;
    case REPEAT :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }
    switch (mFilter)
    {
    case LINEAR :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case NONE :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);

    pData_.resize(uiWidth*uiHeight);
}

texture::~texture()
{
    glDeleteTextures(1, &uiTextureHandle_);
}

void texture::set_wrap(wrap mWrap)
{
    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);

    switch (mWrap)
    {
    case CLAMP :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        break;
    case REPEAT :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void texture::set_filter(filter mFilter)
{
    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);

    switch (mFilter)
    {
    case LINEAR :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case NONE :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
}

const std::vector<texture::color>& texture::get_data() const
{
    return pData_;
}

std::vector<texture::color>& texture::get_data()
{
    return pData_;
}

void texture::set_pixel(uint x, uint y, const texture::color& mColor)
{
    pData_[x + y*uiWidth_] = mColor;
}

const texture::color& texture::get_pixel(uint x, uint y) const
{
    return pData_[x + y*uiWidth_];
}

texture::color& texture::get_pixel(uint x, uint y)
{
    return pData_[x + y*uiWidth_];
}

uint texture::get_width() const
{
    return uiWidth_;
}

uint texture::get_height() const
{
    return uiHeight_;
}

void texture::update_texture()
{
    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uiWidth_, uiHeight_, GL_RGBA, GL_UNSIGNED_BYTE, pData_.data());

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void texture::save_to_file(const std::string& sFile)
{
    texture_manager::save_texture(*this, sFile);
}
