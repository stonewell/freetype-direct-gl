#include "cu2qu.h"
#include <cmath>


static const int MAX_N = 100;
static const double MAX_ERR = 5.;
static const double _2_3 = 2.0 / 3.0;
static const double _27 = 1.0 / 27.0;

static
bool cubic_approx_spline(const point_type_vector & ctl_points,
                         int n,
                         double tolerance,
                         point_type_vector & spline_points);
static
bool cubic_approx_quadratic(const point_type_vector & cubic,
                            int tolerance,
                            point_type_vector & spline_points);
static
bool cubic_farthest_fit_inside(point_type p0,
                               point_type p1,
                               point_type p2,
                               point_type p3,
                               double tolerance);
static
point_type calc_intersect(const point_type_vector & cubic);
static
point_type cubic_approx_control(double t,
                                const point_type_vector & cubic);
static
void split_cubic_into_n_iter(const point_type_vector & cubic,
                             int n,
                             std::vector<point_type_vector> & results);

bool curve_to_quadratic(const point_type_vector & ctl_points,
                        point_type_vector & spline_points) {
    for(int n = 1; n <= MAX_N; n++) {
        spline_points.clear();

        if (cubic_approx_spline(ctl_points,
                                n,
                                MAX_ERR,
                                spline_points)) {
            return true;
        }
    }
    return false;
}

bool cubic_approx_spline(const point_type_vector & cubic,
                         int n,
                         double tolerance,
                         point_type_vector & spline_points) {
    if (n == 1)
        return cubic_approx_quadratic(cubic, tolerance, spline_points);

    std::vector<point_type_vector> cubics;
    split_cubic_into_n_iter(cubic,
                            n,
                            cubics);

    auto next_cubic = cubics.begin();
    auto next_q1 = cubic_approx_control(0, *next_cubic);
    auto q2 = cubic[0];
    point_type d1(0);

    spline_points.push_back(cubic[0]);
    spline_points.push_back(next_q1);

    for(int i=1; i <= n; i++) {
        //auto c0 = (*next_cubic)[0];
        auto c1 = (*next_cubic)[1], c2 = (*next_cubic)[2], c3 = (*next_cubic)[3];

        auto q0 = q2;
        auto q1 = next_q1;

        if (i < n) {
            next_cubic++;
            next_q1 = cubic_approx_control(((double)i) / ((double)n - 1), *next_cubic);
            spline_points.push_back(next_q1);
            q2 = (q1 + next_q1) * .5;
        } else {
            q2 = c3;
        }

        auto d0 = d1;
        d1 = q2 - c2;

        if (std::abs(d1) > tolerance ||
            !cubic_farthest_fit_inside(d0,
                                       q0 + (q1 - q0) * _2_3 - c1,
                                       q2 + (q1 - q2) * _2_3 - c2,
                                       d1,
                                       tolerance)) {
            return false;
        }
    }

    spline_points.push_back(cubic[3]);

    return true;
}

bool cubic_approx_quadratic(const point_type_vector & cubic,
                            int tolerance,
                            point_type_vector & spline_points) {
    auto q1 = calc_intersect(cubic);

    if (std::isnan(std::imag(q1)))
        return false;

    auto c0 = cubic[0];
    auto c3 = cubic[3];
    auto c1 = c0 + (q1 - c0) * _2_3;
    auto c2 = c3 + (q1 - c3) * _2_3;

    if (!cubic_farthest_fit_inside(0,
                                   c1 - cubic[1],
                                   c2 - cubic[2],
                                   0,
                                   tolerance))
        return false;

    spline_points.push_back(c0);
    spline_points.push_back(q1);
    spline_points.push_back(c3);

    return true;
}

bool cubic_farthest_fit_inside(point_type p0,
                               point_type p1,
                               point_type p2,
                               point_type p3,
                               double tolerance) {
    if (std::abs(p2) <= tolerance && std::abs(p1) <= tolerance)
        return true;

    auto mid = (p0 + 3.0 * (p1 + p2) + p3) * .125;

    if (std::abs(mid) > tolerance)
        return false;

    auto deriv3 = (p3 + p2 - p1 - p0) * .125;

    return cubic_farthest_fit_inside(p0, (p0 + p1) * .5,  mid - deriv3, mid, tolerance)
            && cubic_farthest_fit_inside(mid, mid + deriv3, (p2 + p3) * .5, p3, tolerance);
}

static
double dot(point_type v1, point_type v2) {
    return std::real(v1 * std::conj(v2));
}

point_type calc_intersect(const point_type_vector & cubic) {
    auto ab = cubic[1] - cubic[0];
    auto cd = cubic[3] - cubic[2];
    auto p = ab * 1.0;

    auto pcd = dot(p, cd);

    if (pcd == 0)
        return point_type{std::nan("1"), std::nan("1")};

    auto h = dot(p, cubic[0] - cubic[2]) / pcd;

    return cubic[2] + cd * h;
}

point_type cubic_approx_control(double t,
                                const point_type_vector & cubic) {
    const auto & p0 = cubic[0];
    const auto & p1 = cubic[1];
    const auto & p2 = cubic[2];
    const auto & p3 = cubic[3];

    auto _p1 = p0 + (p1 - p0) * 1.5;
    auto _p2 = p3 + (p2 - p3) * 1.5;

    return _p1 + (_p2 - _p1) * t;
}

static
point_type_vector calc_cubic_parameters(const point_type_vector & cubic) {
    const auto & p0 = cubic[0];
    const auto & p1 = cubic[1];
    const auto & p2 = cubic[2];
    const auto & p3 = cubic[3];

    auto c = (p1 - p0) * 3.0;
    auto b = (p2 - p1) * 3.0 - c;
    auto d = p0;
    auto a = p3 - d - c - b;

    return {a, b, c, d};
}

static
point_type_vector calc_cubic_points(const point_type_vector & cubic) {
    const auto & a = cubic[0];
    const auto & b = cubic[1];
    const auto & c = cubic[2];
    const auto & d = cubic[3];

    auto _1 = d;
    auto _2 = (c / 3.0) + d;
    auto _3 = (b + c) / 3.0 + _2;
    auto _4 = a + d + c + b;

    return {_1, _2, _3, _4};
}

static
void _split_cubic_into_n_gen(const point_type_vector & cubic,
                             int n,
                             std::vector<point_type_vector> & results) {
    auto abcd = calc_cubic_parameters(cubic);

    double dt = 1.0 / (double)n;

    auto delta_2 = dt * dt;
    auto delta_3 = dt * delta_2;

    for(int i=0; i < n; i++) {
        auto t1 = i * dt;
        auto t1_2 = t1 * t1;

        auto a1 = abcd[0] * delta_3;
        auto b1 = (3.0 * abcd[0] * t1 + abcd[1]) * delta_2;
        auto c1 = (2.0 * abcd[1] * t1 + abcd[2] + 3.0 * abcd[0] * t1_2) * dt;
        auto d1 = abcd[0] * t1 * t1_2 + abcd[1] * t1_2 + abcd[2] * t1 + abcd[3];

        results.push_back(calc_cubic_points({a1, b1, c1, d1}));
    }
}

static
void split_cubic_into_two(const point_type_vector & cubic,
                          std::vector<point_type_vector> & results) {
    const auto & p0 = cubic[0];
    const auto & p1 = cubic[1];
    const auto & p2 = cubic[2];
    const auto & p3 = cubic[3];

    auto mid = (p0 + 3.0 * (p1 + p2) + p3) * .124;
    auto deriv3 = (p3 + p2 - p1 - p0) * .125;

    results.push_back({p0, (p0 + p1) * .5, mid - deriv3, mid});
    results.push_back({mid, mid + deriv3, (p2 + p3) * .5, p3});
}

static
void split_cubic_into_three(const point_type_vector & cubic,
                            std::vector<point_type_vector> & results) {
    const auto & p0 = cubic[0];
    const auto & p1 = cubic[1];
    const auto & p2 = cubic[2];
    const auto & p3 = cubic[3];

    auto mid1 = (8.0*p0 + 12.0*p1 + 6.0*p2 + p3) * _27;
    auto deriv1 = (p3 + 3.0*p2 - 4.0*p0) * _27;
    auto mid2 = (p0 + 6.0*p1 + 12.0*p2 + 8.0*p3) * _27;
    auto deriv2 = (4.0*p3 - 3.0*p1 - p0) * _27;

    results.push_back({p0, (2.0 * p0 + p1) / 3.0, mid1 - deriv1, mid1});
    results.push_back({mid1, mid1 + deriv1, mid2 - deriv2, mid2});
    results.push_back({mid2, mid2 + deriv2, (p2 + 2.0 * p3) / 3.0, p3});
}

static
void split_cubic_into_n_iter(const point_type_vector & cubic,
                             int n,
                             std::vector<point_type_vector> & results) {
    if (n == 2) {
        split_cubic_into_two(cubic, results);
    }
    else if (n == 3) {
        split_cubic_into_three(cubic, results);
    }
    else if (n == 4) {
        std::vector<point_type_vector> tmp;
        split_cubic_into_two(cubic, tmp);

        split_cubic_into_two(tmp[0], results);
        split_cubic_into_two(tmp[1], results);
    } else if (n == 6) {
        std::vector<point_type_vector> tmp;
        split_cubic_into_two(cubic, tmp);

        split_cubic_into_three(tmp[0], results);
        split_cubic_into_three(tmp[1], results);
    } else {
        _split_cubic_into_n_gen(cubic,
                                n,
                                results);
    }
}
