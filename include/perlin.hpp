#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <lxgui/utils.hpp>

struct perlin_noise
{
    double operator () (double x, double y) const;
    double operator () (double x, double y, double z) const;

    double dPersistence = 0.0;
    double dFrequency = 0.0;
    uint   uiOctaves = 0;
    int    iSeed = 0;

private :

    double get_octave_(double x, double y) const;
    double get_octave_(double x, double y, double z) const;
    double interpolate_(double x, double y, double a) const;
    double noise_(int x, int y) const;
    double noise_(int x, int y, int z) const;
};

#endif
