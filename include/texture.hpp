#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vector>
#include <lxgui/utils.hpp>
#include "color.hpp"

class texture
{
public:

    class color
    {
    public :

        typedef uchar chanel;

        color() {}
        color(chanel nr, chanel ng, chanel nb, chanel na = 255) :
            r(nr), g(ng), b(nb), a(na) {}
        explicit color(const ::color& c) :
            r(c.r*255), g(c.g*255), b(c.b*255), a(c.a*255) {}

        chanel r = 0, g = 0, b = 0, a = 0;

        static const color WHITE;
        static const color BLACK;
        static const color RED;
        static const color GREEN;
        static const color BLUE;
        static const color GREY;
    };

    enum wrap
    {
        REPEAT,
        CLAMP
    };

    enum filter
    {
        NONE,
        LINEAR
    };

    texture();
    texture(uint uiWidth, uint uiHeight, wrap mWrap = REPEAT, filter mFilter = NONE);
    ~texture();

    texture(const texture& tex) = delete;
    texture& operator = (const texture& tex) = delete;

    void set_wrap(wrap mWrap);
    void set_filter(filter mFilter);

    void bind() const;

    const std::vector<texture::color>& get_data() const;
    std::vector<texture::color>&       get_data();
    void                               set_pixel(uint x, uint y, const texture::color& mColor);
    const texture::color&              get_pixel(uint x, uint y) const;
    texture::color&                    get_pixel(uint x, uint y);
    uint                               get_width() const;
    uint                               get_height() const;

    void update_texture();

    void save_to_file(const std::string& sFile);

private:

    uint uiWidth_ = 0;
    uint uiHeight_ = 0;
    uint uiTextureHandle_ = 0;

    std::vector<texture::color> pData_;
};

#endif
