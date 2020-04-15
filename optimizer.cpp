#include "optimizer.h"

#include "stack_allocator.h"

double getM(double r, std::vector<point> &points) {
    double mBig = 0;
    for (int i = 1; i < points.size(); i++) {
        double tmp = std::abs(points[i].y - points[i-1].y) / (points[i].x - points[i - 1].x);
        mBig = std::max(mBig, tmp);
    }

    return mBig > 0 ? r * mBig : 1;
}

OptResult optimize(const std::function<double(double)> &eval,
              const std::function<double(double, double, double, double, double)> &measure,
              const std::function<double(double, double, double, double, double)> &next,
              double r, double epsilon, double x_min, double x_max) {
    std::vector<point> points;
    points.reserve(205);

    points.emplace_back(x_min, eval(x_min));
    points.emplace_back(x_max, eval(x_max));

    double measuredX = points[1].x;
    double measuredY = points[1].y;
    int iters = 0;

    for (int i = 0; i < 200; i++) {
        std::sort(points.begin(), points.end(), [](const auto &a, const auto &b){
            return a.x < b.x;
        });

        double m = getM(r, points);

        std::vector<double, stack_allocator<double, 200>> measures;
        measures.reserve(points.size());
        for (int j = 1; j < points.size(); j++) {
            auto curPoint = points[j];
            auto prevPoint = points[j-1];
            measures.emplace_back(measure(curPoint.x, curPoint.y, prevPoint.x, prevPoint.y, m));
        }

        const auto maxMeasureElt = std::max_element(measures.begin(), measures.end());
        auto maxRegion = std::distance(measures.begin(), maxMeasureElt) + 1;

        auto curPoint = points[maxRegion];
        auto prevPoint = points[maxRegion - 1];

        if (curPoint.x - prevPoint.x < epsilon) {
            measuredX = curPoint.x;
            measuredY = curPoint.y;
            break;
        }

        double nextX = next(curPoint.x, curPoint.y, prevPoint.x, prevPoint.y, m);
        double nextY = eval(nextX);

        iters = i;

//        points.insert(points.begin() + maxRegion, point(nextX, nextY));
        points.emplace_back(nextX, nextY);
    }

    return OptResult{points, point(measuredX, measuredY), iters};
}
