#ifndef COMPRESS_HPP
#define COMPRESS_HPP

#include <lxgui/utils.hpp>

namespace compress
{
    namespace zlib
    {
        bool compress(uchar* pIn, uint uiSizeIn, uchar*& pOut, uint& uiSizeOut);
        bool decompress(uchar* pIn, uint uiSizeIn, uchar*& pOut, uint& uiSizeOut);
    };

    using namespace zlib;
};

#endif
