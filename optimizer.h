//
// Created by alex on 13.04.2020.
//

#pragma once

#include <functional>

#include "chaiscript/chaiscript.hpp"

struct point {
    double x;
    double y;
    point(double x, double y) : x(x), y(y) {}
};

struct OptResult {
    std::vector<point> points;
    point min;
    int iterations;
};

OptResult optimize(const std::function<double(double)> &eval,
                   const std::function<double(double, double, double, double, double)> &measure,
                   const std::function<double(double, double, double, double, double)> &next,
                   double m, double epsilon, double x_min, double x_max);
