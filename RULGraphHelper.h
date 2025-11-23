#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;
using namespace std;

struct RULGraphData
{
    vector<double> threshold;
    vector<double> slope_mean;
    vector<vector<double>> y_line_mean_list;
    vector<vector<double>> y_line_0_list;
    vector<vector<double>> y_line_1_list;
    double input_x;
    vector<double> input_y;
    double target_x;
    vector<double> target_y;
    vector<double> gap_mean_y_mean;
    vector<vector<double>> x_line_list;
    double gap_mean;
    double p2_x_mean;
    vector<double> rul_mean_list;
    int ci;
};

class RULGraphHelper
{
public:
    RULGraphHelper();
    ~RULGraphHelper();

    void DrawRULGraph(CDC* pDC, CRect rect, const RULGraphData& data);

private:
    ULONG_PTR m_gdiplusToken;

    void DrawSingleRULGraph(Graphics& graphics, RectF rect,
        double threshold_val,
        double slope_val,
        const vector<double>& y_line_mean,
        const vector<double>& y_line_0,
        const vector<double>& y_line_1,
        double input_x,
        double input_y_val,
        double target_x,
        double target_y_val,
        double gap_mean_y_val,
        const vector<double>& x_line,
        double gap_mean,
        double p2_x_mean,
        double rul_mean,
        int ci,
        const CString& title);

    double NormalizeValue(double value, double min_val, double max_val);
};