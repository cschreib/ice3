#ifndef NODE_HPP
#define NODE_HPP

#include <lxgui/utils.hpp>
#include "movable.hpp"

class node : public movable
{
public :

    node(utils::wptr<world> pWorld);
    node(utils::wptr<world> pWorld, const vector3f& mPosition);
    ~node() = default;

private :

};

#endif
