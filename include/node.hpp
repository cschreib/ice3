#ifndef NODE_HPP
#define NODE_HPP

#include <lxgui/utils.hpp>
#include "movable.hpp"

class node : public movable
{
public :

    node(world& mWorld);
    node(world& mWorld, const vector3f& mPosition);
    ~node() = default;

private :

};

#endif
