#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include <gdiplus.h>
#include <cmath>

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
     bool has_target;
};

struct PredictionGraphData
{
    vector<vector<double>> samples;
    vector<double> mean;
    vector<double> ci_lower;
    vector<double> ci_upper;
    vector<double> true_values;
    int ci;
    bool has_true_values;
};

struct AxisInfo
{
    double min_val;
    double max_val;
    double step;
    int tick_count;
};

class RULGraphHelper
{
public:
    RULGraphHelper();
    ~RULGraphHelper();

    void DrawRULGraph(CDC* pDC, CRect rect, const RULGraphData& data);
    void DrawPredictionGraph(CDC* pDC, CRect rect, const PredictionGraphData& data);
    void DrawLegend(CDC* pDC, CRect rect, int ci);

private:
    ULONG_PTR m_gdiplusToken;

    AxisInfo CalculateNiceAxis(double min_val, double max_val, int max_ticks);

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

    void DrawSinglePredictionGraph(Graphics& graphics, RectF rect,
        const vector<double>& samples,
        double mean_val,
        double ci_lower,
        double ci_upper,
        double true_val,
        int ci,
        const CString& title);

    void DrawKDE(Graphics& graphics, RectF rect, const vector<double>& samples,
        double min_val, double max_val, double max_density);

    double NormalizeValue(double value, double min_val, double max_val);
};