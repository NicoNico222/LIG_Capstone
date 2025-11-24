#include "pch.h"
#include "RULGraphHelper.h"
#include <cmath>

RULGraphHelper::RULGraphHelper()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

RULGraphHelper::~RULGraphHelper()
{
    GdiplusShutdown(m_gdiplusToken);
}

double RULGraphHelper::NormalizeValue(double value, double min_val, double max_val)
{
    if (max_val - min_val < 1e-8)
        return 0.0;
    return 2.0 * (value - min_val) / (max_val - min_val) - 1.0;
}

AxisInfo RULGraphHelper::CalculateNiceAxis(double min_val, double max_val, int max_ticks)
{
    double range = max_val - min_val;
    if (range <= 0) range = 1.0;

    // 1. 대략적인 간격 계산
    double rough_step = range / (max_ticks - 1);

    // 2. 10의 거듭제곱 단위로 정규화 (예: 250 -> 2.5)
    double exponent = floor(log10(rough_step));
    double fraction = rough_step / pow(10.0, exponent);
    double nice_fraction;

    // 3. 깔끔한 단위(1, 2, 5, 10) 선택 로직
    // 파이썬처럼 촘촘하게 보이기 위해 기준을 조금 더 낮게 잡음
    if (fraction < 1.5) nice_fraction = 1.0;
    else if (fraction < 3.0) nice_fraction = 2.0;
    else if (fraction < 7.0) nice_fraction = 5.0;
    else nice_fraction = 10.0;

    double nice_step = nice_fraction * pow(10.0, exponent);

    // 4. 축 범위 재조정
    double nice_min = floor(min_val / nice_step) * nice_step;
    double nice_max = ceil(max_val / nice_step) * nice_step;
    int tick_count = (int)((nice_max - nice_min) / nice_step) + 1;

    // [중요] 눈금이 너무 적으면(예: 0, 1000, 2000, 3000 -> 4개), 
    // 간격을 강제로 반으로 줄여서(0, 500, 1000...) 최소 5개 이상 확보
    if (tick_count < 5)
    {
        nice_step /= 2.0;
        nice_min = floor(min_val / nice_step) * nice_step;
        nice_max = ceil(max_val / nice_step) * nice_step;
        tick_count = (int)((nice_max - nice_min) / nice_step) + 1;
    }

    AxisInfo info;
    info.min_val = nice_min;
    info.max_val = nice_max;
    info.step = nice_step;
    info.tick_count = tick_count;

    return info;
}

void RULGraphHelper::DrawRULGraph(CDC* pDC, CRect rect, const RULGraphData& data)
{
    Graphics graphics(pDC->m_hDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    int margin = 15;
    int graphWidth = (rect.Width() - margin * 4) / 3;
    int graphHeight = (rect.Height() - margin * 3) / 2;

    CString titles[] = {
        _T("Roll Drift"),
        _T("Pitch Drift"),
        _T("Yaw Drift"),
        _T("V_North Drift"),
        _T("V_East Drift"),
        _T("V_Down Drift")
    };

    for (int i = 0; i < 6; i++)
    {
        int col = i % 3;
        int row = i / 3;

        int graphX = rect.left + margin + col * (graphWidth + margin);
        int graphY = rect.top + margin + row * (graphHeight + margin);

        RectF graphRect((REAL)graphX, (REAL)graphY, (REAL)graphWidth, (REAL)graphHeight);

        DrawSingleRULGraph(
            graphics,
            graphRect,
            data.threshold[i],
            data.slope_mean[i],
            data.y_line_mean_list[i],
            data.y_line_0_list[i],
            data.y_line_1_list[i],
            data.input_x,
            data.input_y[i],
            data.target_x,
            data.target_y[i],
            data.gap_mean_y_mean[i],
            data.x_line_list[i],
            data.gap_mean,
            data.p2_x_mean,
            data.rul_mean_list[i],
            data.ci,
            titles[i]
        );
    }
}

void RULGraphHelper::DrawSingleRULGraph(
    Graphics& graphics,
    RectF rect,
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
    const CString& title)
{
    SolidBrush bgBrush(Color(255, 245, 245, 245));
    graphics.FillRectangle(&bgBrush, rect);

    Pen borderPen(Color(255, 200, 200, 200), 1.0f);
    graphics.DrawRectangle(&borderPen, rect);

    float titleHeight = 25.0f;
    RectF titleRect(rect.X, rect.Y, rect.Width, titleHeight);

    CString fullTitle = title + _T(" (Normalized)");

    Gdiplus::Font titleFont(L"맑은 고딕", 9, FontStyleBold);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    StringFormat titleFormat;
    titleFormat.SetAlignment(StringAlignmentCenter);
    titleFormat.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(fullTitle, -1, &titleFont, titleRect, &titleFormat, &textBrush);

    float leftMargin = 45.0f;
    float bottomMargin = 25.0f;
    RectF graphRect(rect.X + leftMargin, rect.Y + titleHeight + 5,
        rect.Width - leftMargin - 15, rect.Height - titleHeight - bottomMargin - 5);

    double raw_x_min = *min_element(x_line.begin(), x_line.end());
    double raw_x_max = *max_element(x_line.begin(), x_line.end());

    if (input_x < raw_x_min) raw_x_min = input_x;
    if (input_x > raw_x_max) raw_x_max = input_x;
    if (target_x > raw_x_max) raw_x_max = target_x;
    if (rul_mean > raw_x_max) raw_x_max = rul_mean;

    // [수정] max_ticks를 5 -> 10으로 늘림 (더 촘촘하게 요청)
    AxisInfo xAxis = CalculateNiceAxis(raw_x_min, raw_x_max, 8);
    double x_min = xAxis.min_val;
    double x_max = xAxis.max_val;

    double thr_raw = (slope_val > 0) ? threshold_val : -threshold_val;

    vector<double> all_y_values;
    all_y_values.insert(all_y_values.end(), y_line_mean.begin(), y_line_mean.end());
    all_y_values.insert(all_y_values.end(), y_line_0.begin(), y_line_0.end());
    all_y_values.insert(all_y_values.end(), y_line_1.begin(), y_line_1.end());
    all_y_values.push_back(input_y_val);
    all_y_values.push_back(target_y_val);
    all_y_values.push_back(gap_mean_y_val);
    all_y_values.push_back(thr_raw);

    double y_min_raw = *min_element(all_y_values.begin(), all_y_values.end());
    double y_max_raw = *max_element(all_y_values.begin(), all_y_values.end());

    auto normalize = [y_min_raw, y_max_raw](double y) -> double {
        if (abs(y_max_raw - y_min_raw) < 1e-10) return 0.0;
        return 2.0 * (y - y_min_raw) / (y_max_raw - y_min_raw) - 1.0;
        };

    double y_min = -1.0;
    double y_max = 1.0;

    Pen axisPen(Color(255, 0, 0, 0), 1.0f);
    graphics.DrawLine(&axisPen, PointF(graphRect.X, graphRect.Y), PointF(graphRect.X, graphRect.Y + graphRect.Height));
    graphics.DrawLine(&axisPen, PointF(graphRect.X, graphRect.Y + graphRect.Height), PointF(graphRect.X + graphRect.Width, graphRect.Y + graphRect.Height));

    Pen gridPen(Color(100, 200, 200, 200), 0.5f);
    gridPen.SetDashStyle(DashStyleDot);

    Gdiplus::Font axisFont(L"맑은 고딕", 8);
    StringFormat yFormat;
    yFormat.SetAlignment(StringAlignmentFar);
    yFormat.SetLineAlignment(StringAlignmentCenter);

    for (int i = 0; i <= 5; i++)
    {
        float yPos = graphRect.Y + graphRect.Height * i / 5.0f;
        double value = 1.0 - (2.0 * i / 5.0);

        graphics.DrawLine(&gridPen, PointF(graphRect.X, yPos), PointF(graphRect.X + graphRect.Width, yPos));

        CString label;
        label.Format(_T("%.1f"), value);
        RectF labelRect(graphRect.X - 40, yPos - 8, 35, 16);
        graphics.DrawString(label, -1, &axisFont, labelRect, &yFormat, &textBrush);
    }

    StringFormat xFormat;
    xFormat.SetAlignment(StringAlignmentCenter);
    xFormat.SetLineAlignment(StringAlignmentNear);

    for (int i = 0; i < xAxis.tick_count; i++)
    {
        double currentVal = xAxis.min_val + i * xAxis.step;
        float x_norm_ratio = (float)((currentVal - x_min) / (x_max - x_min));
        float xPos = graphRect.X + x_norm_ratio * graphRect.Width;

        if (i > 0)
            graphics.DrawLine(&gridPen, PointF(xPos, graphRect.Y), PointF(xPos, graphRect.Y + graphRect.Height));

        CString label;
        if (abs(currentVal - (int)currentVal) < 1e-5)
            label.Format(_T("%d"), (int)currentVal);
        else
            label.Format(_T("%.1f"), currentVal);

        RectF labelRect(xPos - 25, graphRect.Y + graphRect.Height + 5, 50, 20);
        graphics.DrawString(label, -1, &axisFont, labelRect, &xFormat, &textBrush);
    }

    auto GetXPos = [&](double val) -> float {
        return graphRect.X + (float)((val - x_min) / (x_max - x_min)) * graphRect.Width;
        };

    auto GetYPos = [&](double val_norm_y) -> float {
        return graphRect.Y + graphRect.Height * (1.0f - (float)((val_norm_y - y_min) / (y_max - y_min)));
        };

    SolidBrush ciBrush(Color(80, 174, 199, 232));
    vector<PointF> ciPoints;
    for (size_t i = 0; i < x_line.size(); i++)
    {
        float x = GetXPos(x_line[i]);
        float y = GetYPos(normalize(y_line_0[i]));
        ciPoints.push_back(PointF(x, y));
    }
    for (int i = (int)x_line.size() - 1; i >= 0; i--)
    {
        float x = GetXPos(x_line[i]);
        float y = GetYPos(normalize(y_line_1[i]));
        ciPoints.push_back(PointF(x, y));
    }
    if (ciPoints.size() > 2)
        graphics.FillPolygon(&ciBrush, &ciPoints[0], (int)ciPoints.size());

    Pen linePen(Color(255, 31, 119, 180), 2.0f);
    vector<PointF> linePoints;
    for (size_t i = 0; i < x_line.size(); i++)
    {
        float x = GetXPos(x_line[i]);
        float y = GetYPos(normalize(y_line_mean[i]));
        linePoints.push_back(PointF(x, y));
    }
    if (linePoints.size() > 1)
        graphics.DrawLines(&linePen, &linePoints[0], (int)linePoints.size());

    SolidBrush inputBrush(Color(255, 255, 127, 14));
    graphics.FillEllipse(&inputBrush, GetXPos(input_x) - 4, GetYPos(normalize(input_y_val)) - 4, 8.0f, 8.0f);

    SolidBrush targetBrush(Color(255, 255, 0, 0));
    graphics.FillEllipse(&targetBrush, GetXPos(target_x) - 4, GetYPos(normalize(target_y_val)) - 4, 8.0f, 8.0f);

    SolidBrush gapBrush(Color(255, 44, 160, 44));
    float gap_x_pos = (float)(gap_mean + p2_x_mean);
    graphics.FillEllipse(&gapBrush, GetXPos(gap_x_pos) - 4, GetYPos(normalize(gap_mean_y_val)) - 4, 8.0f, 8.0f);

    double thr_normalized = normalize(thr_raw);
    float thr_y = GetYPos(thr_normalized);
    Pen thresholdPen(Color(255, 255, 0, 0), 1.5f);
    thresholdPen.SetDashStyle(DashStyleDash);
    graphics.DrawLine(&thresholdPen, PointF(graphRect.X, thr_y), PointF(graphRect.X + graphRect.Width, thr_y));

    float rul_x = GetXPos(rul_mean);
    Pen rulPen(Color(200, 0, 0, 255), 1.5f);
    rulPen.SetDashStyle(DashStyleDot);

    if (rul_mean >= x_min && rul_mean <= x_max)
    {
        graphics.DrawLine(&rulPen, PointF(rul_x, graphRect.Y), PointF(rul_x, graphRect.Y + graphRect.Height));

        SolidBrush rulBrush(Color(255, 0, 0, 255));
        graphics.FillEllipse(&rulBrush, rul_x - 4, thr_y - 4, 8.0f, 8.0f);

        CString rulText;
        rulText.Format(_T("%.1f Month"), rul_mean);
        Gdiplus::Font rulFont(L"맑은 고딕", 9, FontStyleBold);
        SolidBrush rulTextBrush(Color(255, 0, 0, 255));

        float textX = rul_x + 8;
        if (textX + 80 > graphRect.X + graphRect.Width)
            textX = rul_x - 88;

        RectF rulTextRect(textX, thr_y - 18, 90, 25);
        graphics.DrawString(rulText, -1, &rulFont, rulTextRect, NULL, &rulTextBrush);
    }
}