#pragma once

#include <complex>
#include <vector>

using point_type = std::complex<double>;
using point_type_vector = std::vector<point_type>;

bool curve_to_quadratic(const point_type_vector & ctl_points,
                        point_type_vector & spline_points);
