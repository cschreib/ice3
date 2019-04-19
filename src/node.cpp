#include "node.hpp"

node::node(world& mWorld) :
    movable(mWorld)
{
}

node::node(world& mWorld, const vector3f& mPosition) :
    movable(mWorld, mPosition)
{
}
