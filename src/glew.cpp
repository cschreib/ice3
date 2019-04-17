#include "glew.hpp"

void glew::initialize()
{
    static bool bGlewInit = false;
    if (!bGlewInit)
    {
        glewInit();
        bGlewInit = true;
    }
}
