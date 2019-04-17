#include "vertexbufferobject.hpp"
#include "glew.hpp"

#include <utils_exception.hpp>
#include <utils_string.hpp>
#include <cstring>

const vertex_type vbo_vertex_1::TYPE = T2_C3_V3;
const vertex_type vbo_vertex_2::TYPE = T23_C3_V3;
const vertex_type vbo_vertex_3::TYPE = T4_C3_V3;

vbo_data::vbo_data(uint size, vertex_type type) :
    mType(type), uiNum(size)
{
    switch (mType)
    {
    case T2_C3_V3 :
        pData = new vbo_vertex_1[size];
        return;
    case T23_C3_V3 :
        pData = new vbo_vertex_2[size];
        return;
    case T4_C3_V3 :
        pData = new vbo_vertex_3[size];
        return;
    }

    throw utils::exception("vbo_data", "unhandled vertex type : "+utils::to_string((int)mType));
}

vbo_data::~vbo_data()
{
    switch (mType)
    {
    case T2_C3_V3 :
        delete[] reinterpret_cast<vbo_vertex_1*>(pData);
        return;
    case T23_C3_V3 :
        delete[] reinterpret_cast<vbo_vertex_2*>(pData);
        return;
    case T4_C3_V3 :
        delete[] reinterpret_cast<vbo_vertex_3*>(pData);
        return;
    }

    throw utils::exception("vbo_data", "unhandled vertex type : "+utils::to_string((int)mType));
}

void vertex_buffer_object::init_()
{
    glew::initialize();

    glGenBuffers(1, &uiVBOHandle_);
}

void vertex_buffer_object::set_data_(const void* pData, uint uiNumVertex, vertex_type mType)
{
    uint uiSize = 0;

    switch (mType)
    {
    case T2_C3_V3 :
        uiSize = sizeof(vbo_vertex_1);
        break;
    case T23_C3_V3 :
        uiSize = sizeof(vbo_vertex_2);
        break;
    case T4_C3_V3 :
        uiSize = sizeof(vbo_vertex_3);
        break;
    }

    if (uiSize == 0)
        utils::exception("vbo_data", "unhandled vertex type : "+utils::to_string((int)mType));

    if (uiNumVertex == 0)
    {
        if (uiVBOHandle_ != 0)
            glDeleteBuffers(1, &uiVBOHandle_);

        uiNumVertex_ = 0;
    }
    else
    {
        if (uiVBOHandle_ == 0)
            glGenBuffers(1, &uiVBOHandle_);

        uiNumVertex_ = uiNumVertex;

        glBindBuffer(GL_ARRAY_BUFFER, uiVBOHandle_);
        glBufferData(GL_ARRAY_BUFFER, uiNumVertex_*uiSize, pData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        mVertexType_ = mType;
    }
}

vertex_buffer_object::vertex_buffer_object(const vbo_data& mData)
{
    init_();
    set_data_(mData.pData, mData.uiNum, mData.mType);
}

vertex_buffer_object::~vertex_buffer_object()
{
    if (uiVBOHandle_ != 0)
        glDeleteBuffers(1, &uiVBOHandle_);
}

void vertex_buffer_object::set_data(const vbo_data& mData)
{
    set_data_(mData.pData, mData.uiNum, mData.mType);
}

uint vertex_buffer_object::get_size() const
{
    return uiNumVertex_;
}

void vertex_buffer_object::render()
{
    if (uiVBOHandle_ == 0)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, uiVBOHandle_);

    switch (mVertexType_)
    {
        case T2_C3_V3 :

            glEnableClientState(GL_VERTEX_ARRAY);
            glInterleavedArrays(GL_T2F_C3F_V3F, 0, 0);

            glDrawArrays(GL_QUADS, 0, uiNumVertex_);

            glDisableClientState(GL_VERTEX_ARRAY);
            break;

        case T23_C3_V3 :

            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(vbo_vertex_2), (void*)(0*sizeof(float)));

            glClientActiveTexture(GL_TEXTURE1);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(3, GL_FLOAT, sizeof(vbo_vertex_2), (void*)(2*sizeof(float)));

            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            glColorPointer   (3, GL_FLOAT, sizeof(vbo_vertex_2), (void*)(5*sizeof(float)));
            glVertexPointer  (3, GL_FLOAT, sizeof(vbo_vertex_2), (void*)(8*sizeof(float)));

            glDrawArrays(GL_QUADS, 0, uiNumVertex_);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);

            glClientActiveTexture(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glClientActiveTexture(GL_TEXTURE0);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            break;

        case T4_C3_V3 :

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);

            glTexCoordPointer(4, GL_FLOAT, sizeof(vbo_vertex_3), (void*)(0*sizeof(float)));
            glColorPointer   (3, GL_FLOAT, sizeof(vbo_vertex_3), (void*)(4*sizeof(float)));
            glVertexPointer  (3, GL_FLOAT, sizeof(vbo_vertex_3), (void*)(7*sizeof(float)));

            glDrawArrays(GL_QUADS, 0, uiNumVertex_);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            break;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool is_extension_supported(const char* szTargetExtension)
{
    // Code taken from :
    // http://nehe.gamedev.net/tutorial/vertex_buffer_objects/22002/

    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;

    // Extension names should not have spaces
    pszWhere = (unsigned char*)strchr(szTargetExtension, ' ');
    if (pszWhere || *szTargetExtension == '\0')
        return false;

    // Get Extensions String
    pszExtensions = glGetString(GL_EXTENSIONS);

    // Search The Extensions String For An Exact Copy
    pszStart = pszExtensions;

    for (;;)
    {
        pszWhere = (unsigned char*)strstr((const char*)pszStart, szTargetExtension);
        if (!pszWhere)
            break;

        pszTerminator = pszWhere + strlen(szTargetExtension);

        if ((pszWhere == pszStart || *(pszWhere - 1) == ' ') &&
            (*pszTerminator == ' ' || *pszTerminator == '\0'))
            return true;

        pszStart = pszTerminator;
    }

    return false;
}

bool vertex_buffer_object::is_supported()
{
    glew::initialize();
    return is_extension_supported("GL_ARB_vertex_buffer_object");
}
