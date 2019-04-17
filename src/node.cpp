#include "node.hpp"

node::node(utils::wptr<world> pWorld) :
    movable(pWorld)
{
}

node::node(utils::wptr<world> pWorld, const vector3f& mPosition) :
    movable(pWorld, mPosition)
{
}
