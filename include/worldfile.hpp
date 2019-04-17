#ifndef WORLDFILE_HPP
#define WORLDFILE_HPP

namespace region_file {
namespace v0001 {
struct header
{
    char  sFileType[3];
    char  sVersion[4];
};
struct chunk
{
    vector3i mPos;
    uint     uiNumPlainBlock;
    uchar    ucDataCompression;
    uint     uiDataSize;
};
struct chunk_data_header
{
    uint uiNumUnit;
    uint uiNumLight;
};
struct block
{
    uchar ucType;
    uchar ucSunLight;
    uchar ucLight;
};
struct light
{
    vector3i mPos;
    uchar    ucIntensity;
};
struct item
{
    uint  uiType;
    uchar ucQuality;
};
struct unit
{
    static const size_t MAX_NAME_LENGTH = 16;
    uint     sName[MAX_NAME_LENGTH];
    uchar    ucType;
    vector3i mPos;
    item     lItemSlots[8];
    uint     uiInventorySize;
};
}
using namespace v0001;
}

namespace world_file {
namespace v0001 {
struct header
{
    char  sFileType[4];
    char  sVersion[4];
    uchar ucChunkSize;
    uchar ucLightAtten;
    uint  pUnitOffset;
    uint  uiNumUnit;
};
struct unit
{
    static const size_t MAX_NAME_LENGTH = 16;
    uint     sName[MAX_NAME_LENGTH];
    vector3i mChunkPos;
};
}
using namespace v0001;
}

#endif
