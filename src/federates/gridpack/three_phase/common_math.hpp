#pragma once

#include <complex>
#include <cmath>

namespace three_phase
{

std::complex<double> RotationToRadians(double magnitude, double degrees)
{
    return std::polar(magnitude, (degrees * M_PI / 180.0));
}

} // namespace three_phase