#include "chaiscript/chaiscript.hpp"
#include "imgui.h"

#include "gui.hpp"
#include "optimizer.h"

#include <imgui_plot.hpp>

char buf[256] = "0.2*sin(x) + 3 * cos(x)";

float m = 0.15;
float minx = -1;
float maxx = 1;
float epsilon = 0.0001;
int method;

float x[1000];
float y[1000];

float y_min = 0, y_max = 0;

std::vector<point> minPoints;

float measuredX = 0, measuredY = 0;
int iters = 0;

std::unique_ptr<chaiscript::ChaiScript> chai;

double cot(double i) {
    return cos(i) / sin(i);
}

void init() {
    chai = std::make_unique<chaiscript::ChaiScript>();
    chai->add(chaiscript::fun(&sin), "sin");
    chai->add(chaiscript::fun(&cos), "cos");
    chai->add(chaiscript::fun(&tan), "tan");
    chai->add(chaiscript::fun(&cot), "cot");
}

void calculateFunction(const std::string &function) {
    float cur = minx;
    float step = std::fabs(minx - maxx) / 1000;
    std::string prog = std::string(buf);
    for (int i = 0; i < 1000; i++, cur += step) {
        chai->set_global(chaiscript::var(cur), "x");
        auto res = chai->eval<double>(prog);
        x[i] = cur;
        y[i] = res;
        y_max = std::max(y_max, (float)res);
        y_min = std::min(y_min, (float)res);
    }
}

auto bruteforceMeasure = [](double curX, double curY, double prevX, double prevY, double m) {
    return curX - prevX;
};

auto bruteforceNext = [](double curX, double curY, double prevX, double prevY, double m) {
    return (prevX + curX) / 2;
};

auto piyavskyMeasure = [](double curX, double curY, double prevX, double prevY, double m) {
   return m * (curX - prevX) / 2 - (curY + prevY) / 2;
};

auto piyavskyNext = [](double curX, double curY, double prevX, double prevY, double m) {
    //return (curX + prevX) / 2 - (curY - prevY) / (2*m);
  return (curX - prevX) / 2 + (curY - prevY) / (2*m);
};

auto stronginMeasure = [](double curX, double curY, double prevX, double prevY, double m) {
    return m * (curX - prevX) + std::pow(curY - prevY, 2) / (m*(curX - prevX)) - 2 * (curY+prevY);
};

auto stronginNext = [](double curX, double curY, double prevX, double prevY, double m) {
  //return (curX + prevX) / 2 - (curY - prevY) / (2*m);
  return (curX - prevX) / 2 + (curY - prevY) / (2*m);
};

void drawGUI(ImGuiIO &io) {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y));

    ImGui::Begin("Controls", nullptr, window_flags);
    ImGui::InputText("f(x)", buf, 256);
    ImGui::InputFloat("m", &m, 0.01, 0.05);
    ImGui::InputFloat("epsilon", &epsilon, 0.001, 0.005);
    ImGui::InputFloat("min x", &minx, 0.01, 1);
    ImGui::InputFloat("max x", &maxx, 0.01, 1);
    ImGui::RadioButton("Bruteforce", &method, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Piyavsky", &method, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Strongin", &method, 2);
    if (ImGui::Button("Calculate")) {
        calculateFunction(buf);

        std::string func(buf);
        chaiscript::ChaiScript &c = *chai;
        auto eval = [&c, &func](double x) {
          c.set_global(chaiscript::var(x), "x");
          return c.eval<double>(func);
        };

        std::function<double(double, double, double, double, double)> measure;
        std::function<double(double, double, double, double, double)> next;

        switch (method) {
            case 0:
                measure = bruteforceMeasure;
                next = bruteforceNext;
                break;
            case 1:
                measure = piyavskyMeasure;
                next = piyavskyNext;
                break;
            case 2:
                measure = stronginMeasure;
                next = stronginNext;
                break;
            default:
                std::terminate();
        }

        auto optRes = optimize(eval, measure, next, m, epsilon, minx, maxx);
        minPoints = optRes.points;
        measuredX = (float)optRes.min.x;
        measuredY = (float)optRes.min.y;
        iters = optRes.iterations;
    }
    ImGui::Text("Min X: %.2g", measuredX);
    ImGui::Text("Min Y: %.2g", measuredY);
    ImGui::Text("Iterations: %i", iters);
    ImGui::End();
    // create and configure persistent PlotInterface and PlotItems upfront
    ImGui::PlotInterface plot;
    std::vector<ImGui::PlotItem> items(2);

    plot.title = "Plot";
    plot.x_axis.minimum = minx;
    plot.x_axis.maximum = maxx;
    plot.x_axis.label = "x";
    plot.y_axis.maximum = y_max;
    plot.y_axis.minimum = y_min;

    items[0].label = "Target function";
    items[0].type = ImGui::PlotItem::Line;
    items[0].color = ImVec4(1, 1, 0, 1);
    items[0].data = std::vector<ImVec2>(1000);
    for (int i = 0; i < 1000; i++) {
        items[0].data[i] = ImVec2(x[i], y[i]);
    }

    items[1].label = "Probes";
    items[1].type = ImGui::PlotItem::Scatter;
    items[1].color = ImVec4(1, 0, 1, 1);
    items[1].data = std::vector<ImVec2>();
    items[1].size = 6;

    if (!minPoints.empty()) {
        items[1].data.reserve(minPoints.size());
        std::transform(minPoints.begin(), minPoints.end(), std::back_inserter(items[1].data),
                       [](auto &point) {
                           return ImVec2((float)point.x, y_min + 0.002f);
                       });
    }

    ImGui::SetNextWindowPos(ImVec2(300, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 300, io.DisplaySize.y));
    ImGui::Begin("Plot", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Plot("Plot", plot, items);
    ImGui::End();
}