#ifndef VERTEXBUFFEROBJECT_HPP
#define VERTEXBUFFEROBJECT_HPP

#include <lxgui/utils.hpp>
#include "vector3.hpp"
#include <vector>

enum vertex_type
{
    T2_C3_V3,
    T23_C3_V3,
    T4_C3_V3
};

struct vbo_vertex_1
{
    float  u = 0.0, v = 0.0;
    float  r = 0.0, g = 0.0, b = 0.0;
    vector3f pos;

    static const vertex_type TYPE;
};
struct vbo_vertex_2
{
    float  u = 0.0, v = 0.0, s = 0.0, l = 0.0, o = 0.0;
    float  r = 0.0, g = 0.0, b = 0.0;
    vector3f pos;

    static const vertex_type TYPE;
};
struct vbo_vertex_3
{
    float  u = 0.0, v = 0.0, s = 0.0, l = 0.0;
    float  r = 0.0, g = 0.0, b = 0.0;
    vector3f pos;

    static const vertex_type TYPE;
};

struct vbo_data
{
    vbo_data(uint size, vertex_type type);
    ~vbo_data();

    vertex_type mType = T2_C3_V3;
    void*       pData = nullptr;
    uint        uiNum = 0;
};

class vertex_buffer_object
{
public :

    template<class T>
    vertex_buffer_object(const std::vector<T>& lVertexCache, uint uiNumVertex = 0)
    {
        init_();
        set_data(lVertexCache, uiNumVertex);
    }

    vertex_buffer_object(const vbo_data& mData);

    ~vertex_buffer_object();

    template<class T>
    void set_data(const std::vector<T>& lVertexCache, uint uiNumVertex = 0)
    {
        set_data_(&lVertexCache[0], uiNumVertex == 0 ? lVertexCache.size() : uiNumVertex, T::TYPE);
    }

    void set_data(const vbo_data& mData);

    uint get_size() const;

    void render();

    static bool is_supported();

private :

    void init_();
    void set_data_(const void* pData, uint uiNumVertex, vertex_type mType);

    vertex_type mVertexType_ = T2_C3_V3;
    uint        uiVBOHandle_ = 0;
    uint        uiNumVertex_ = 0;
};

#endif
