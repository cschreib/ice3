#include "compress.hpp"

namespace zlib_impl {
#include <zlib.h>
}

namespace compress
{
bool zlib::compress(uchar* pIn, uint uiSizeIn, uchar*& pOut, uint& uiSizeOut)
{
    using namespace zlib_impl;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = deflateInit(&strm, 1);
    if (ret != Z_OK)
        return false;

    strm.avail_in = uiSizeIn;
    strm.next_in = reinterpret_cast<Bytef*>(pIn);

    uint maxSize = deflateBound(&strm, uiSizeIn);

    pOut = new uchar[maxSize+sizeof(uint)];

    strm.avail_out = maxSize;
    strm.next_out = pOut+sizeof(uint);

    ret = deflate(&strm, Z_FINISH);

    if (ret != Z_STREAM_END)
    {
        delete[] pOut;
        return false;
    }

    *reinterpret_cast<uint*>(pOut) = uiSizeIn;

    uiSizeOut = strm.total_out+sizeof(uint);

    deflateEnd(&strm);

    return true;
}

bool zlib::decompress(uchar* pIn, uint uiSizeIn, uchar*& pOut, uint& uiSizeOut)
{
    using namespace zlib_impl;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = inflateInit(&strm);
    if (ret != Z_OK)
        return false;

    strm.avail_in = uiSizeIn;
    strm.next_in = reinterpret_cast<Bytef*>(pIn+sizeof(uint));

    uiSizeOut = *reinterpret_cast<uint*>(pIn);
    pOut = new uchar[uiSizeOut];

    strm.avail_out = uiSizeOut;
    strm.next_out = pOut;

    ret = inflate(&strm, Z_FINISH);

    if (ret != Z_STREAM_END)
    {
        delete[] pOut;
        return false;
    }

    inflateEnd(&strm);

    return true;
}
}
