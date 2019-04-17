#include "perlin.hpp"

// NOTE : Perlin noise code taken from :
//   http://stackoverflow.com/questions/4753055/perlin-noise-generation-for-terrain
// Credits go to 'gamernb' for the initial version.

/*perlin_noise::perlin_noise() :
    dPersistence(0.0), dFrequency(0.0), uiOctaves(0u), iSeed(0)
{
}*/

double perlin_noise::operator () (double i, double j) const
{
    double t = 0.0;
    double a = 1.0;
    double f = dFrequency;
    double sum = 0.0;

    for (uint k = 0; k < uiOctaves; ++k)
    {
        t += a*get_octave_(j*f + iSeed, i*f + iSeed);
        sum += a;
        a *= dPersistence;
        f *= 2.0;
    }

    return t/sum;
}

double perlin_noise::operator () (double i, double j, double k) const
{
    double t = 0.0;
    double a = 1.0;
    double f = dFrequency;
    double sum = 0.0;

    for (uint l = 0; l < uiOctaves; ++l)
    {
        t += a*get_octave_(k*f + iSeed, j*f + iSeed, i*f + iSeed);
        sum += a;
        a *= dPersistence;
        f *= 2.0;
    }

    return t/sum;
}

double perlin_noise::get_octave_(double x, double y) const
{
    int Xint = x;
    int Yint = y;

    double dx = x - Xint;
    double dy = y - Yint;

    const uint size = 3;
    const int half = size/2;
    double array[size+1][size+1];

    for (int j = -half; j <= half+1; ++j)
    for (int i = -half; i <= half+1; ++i)
        array[i+half][j+half] = noise_(Xint+i, Yint+j);

    /*static const double coef[size][size] = {
        {1.0, 2.0, 1.0},
        {2.0, 3.0, 2.0},
        {1.0, 2.0, 1.0}
    };*/
    static const double coef[size][size] = {
        {0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0}
    };

    double sum = 0.0;
    for (int j = -half; j <= half; ++j)
    for (int i = -half; i <= half; ++i)
        sum += coef[i+half][j+half];

    double corner[2][2];
    for (uint j = 0; j < 2; ++j)
    for (uint i = 0; i < 2; ++i)
    {
        corner[i][j] = 0.0;

        for (int l = -half; l <= half; ++l)
        for (int k = -half; k <= half; ++k)
            corner[i][j] += coef[k+half][l+half]*array[i+k+half][j+l+half]/sum;
    }

    double sx1 = interpolate_(corner[0][0], corner[1][0], dx);
    double sx2 = interpolate_(corner[0][1], corner[1][1], dx);
    double res = interpolate_(sx1, sx2, dy);

    return res;
}

double perlin_noise::get_octave_(double x, double y, double z) const
{
    return noise_(x, y, z);
}

double perlin_noise::interpolate_(double x, double y, double a) const
{
    double negA = 1.0 - a;
    double negASqr = negA * negA;
    double fac1 = 3.0 * (negASqr) - 2.0 * (negASqr * negA);
    double aSqr = a * a;
    double fac2 = 3.0 * aSqr - 2.0 * (aSqr * a);

    return x * fac1 + y * fac2; //add the weighted factors
}

double perlin_noise::noise_(int x, int y) const
{
    // NOTE : This noise function returns garbage for large numbers
    int n = x + y*57;
    n = (n << 13) ^ n;
    n = (n*(n*n*15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - double(n)*0.931322574615478515625e-9;

    // Alternative (still untested) :
    /*
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int nn = (n*(n*n*60493 + 19990303) + 1376312589) & 0x7fffffff;
    return 1.0 - ((double)nn/1073741824.0);
    */
}

double perlin_noise::noise_(int x, int y, int z) const
{
	int n = x + y*57 + z*131;
	n = (n << 13)^n;
	n = (n*(n*n*15731 + 789221) + 1376312589) & 0x7fffffff;
	return 1.0 - double(n)*0.931322574615478515625e-9;
}
