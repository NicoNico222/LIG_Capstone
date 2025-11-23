#include "pch.h"
#include "RULGraphHelper.h"

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

    Gdiplus::Font titleFont(L"¸¼Àº °íµñ", 9, FontStyleBold);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    StringFormat titleFormat;
    titleFormat.SetAlignment(StringAlignmentCenter);
    titleFormat.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(fullTitle, -1, &titleFont, titleRect, &titleFormat, &textBrush);

    RectF graphRect(rect.X + 50, rect.Y + titleHeight + 5,
        rect.Width - 65, rect.Height - titleHeight - 35);

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

    double x_min = *min_element(x_line.begin(), x_line.end());
    double x_max = *max_element(x_line.begin(), x_line.end());
    if (rul_mean > x_max) x_max = rul_mean;

    Pen axisPen(Color(255, 0, 0, 0), 1.0f);
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y),
        PointF(graphRect.X, graphRect.Y + graphRect.Height));
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y + graphRect.Height),
        PointF(graphRect.X + graphRect.Width, graphRect.Y + graphRect.Height));

    Pen gridPen(Color(100, 200, 200, 200), 0.5f);
    gridPen.SetDashStyle(DashStyleDot);
    for (int i = 1; i <= 4; i++)
    {
        float y = graphRect.Y + graphRect.Height * i / 5.0f;
        graphics.DrawLine(&gridPen, PointF(graphRect.X, y),
            PointF(graphRect.X + graphRect.Width, y));
    }

    SolidBrush ciBrush(Color(80, 174, 199, 232));
    vector<PointF> ciPoints;
    for (size_t i = 0; i < x_line.size(); i++)
    {
        float x_norm = (float)((x_line[i] - x_min) / (x_max - x_min));
        double y0_normalized = normalize(y_line_0[i]);
        float y0_val = (float)((y0_normalized - y_min) / (y_max - y_min));
        float x = graphRect.X + x_norm * graphRect.Width;
        float y = graphRect.Y + graphRect.Height * (1.0f - y0_val);
        ciPoints.push_back(PointF(x, y));
    }
    for (int i = (int)x_line.size() - 1; i >= 0; i--)
    {
        float x_norm = (float)((x_line[i] - x_min) / (x_max - x_min));
        double y1_normalized = normalize(y_line_1[i]);
        float y1_val = (float)((y1_normalized - y_min) / (y_max - y_min));
        float x = graphRect.X + x_norm * graphRect.Width;
        float y = graphRect.Y + graphRect.Height * (1.0f - y1_val);
        ciPoints.push_back(PointF(x, y));
    }
    if (ciPoints.size() > 2)
        graphics.FillPolygon(&ciBrush, &ciPoints[0], (int)ciPoints.size());

    Pen linePen(Color(255, 31, 119, 180), 2.0f);
    vector<PointF> linePoints;
    for (size_t i = 0; i < x_line.size(); i++)
    {
        float x_norm = (float)((x_line[i] - x_min) / (x_max - x_min));
        double y_normalized = normalize(y_line_mean[i]);
        float y_val = (float)((y_normalized - y_min) / (y_max - y_min));
        float x = graphRect.X + x_norm * graphRect.Width;
        float y = graphRect.Y + graphRect.Height * (1.0f - y_val);
        linePoints.push_back(PointF(x, y));
    }
    if (linePoints.size() > 1)
        graphics.DrawLines(&linePen, &linePoints[0], (int)linePoints.size());

    float input_x_norm = (float)((input_x - x_min) / (x_max - x_min));
    double input_y_normalized = normalize(input_y_val);
    float input_y_norm = (float)((input_y_normalized - y_min) / (y_max - y_min));
    float input_px = graphRect.X + input_x_norm * graphRect.Width;
    float input_py = graphRect.Y + graphRect.Height * (1.0f - input_y_norm);
    SolidBrush inputBrush(Color(255, 255, 127, 14));
    graphics.FillEllipse(&inputBrush, (REAL)(input_px - 5), (REAL)(input_py - 5), (REAL)10, (REAL)10);

    float target_x_norm = (float)((target_x - x_min) / (x_max - x_min));
    double target_y_normalized = normalize(target_y_val);
    float target_y_norm = (float)((target_y_normalized - y_min) / (y_max - y_min));
    float target_px = graphRect.X + target_x_norm * graphRect.Width;
    float target_py = graphRect.Y + graphRect.Height * (1.0f - target_y_norm);
    SolidBrush targetBrush(Color(255, 255, 0, 0));
    graphics.FillEllipse(&targetBrush, (REAL)(target_px - 5), (REAL)(target_py - 5), (REAL)10, (REAL)10);

    float gap_x_norm = (float)(((gap_mean + p2_x_mean) - x_min) / (x_max - x_min));
    double gap_y_normalized = normalize(gap_mean_y_val);
    float gap_y_norm = (float)((gap_y_normalized - y_min) / (y_max - y_min));
    float gap_px = graphRect.X + gap_x_norm * graphRect.Width;
    float gap_py = graphRect.Y + graphRect.Height * (1.0f - gap_y_norm);
    SolidBrush gapBrush(Color(255, 44, 160, 44));
    graphics.FillEllipse(&gapBrush, (REAL)(gap_px - 5), (REAL)(gap_py - 5), (REAL)10, (REAL)10);

    double thr_normalized = normalize(thr_raw);
    float thr_norm = (float)((thr_normalized - y_min) / (y_max - y_min));
    float thr_y = graphRect.Y + graphRect.Height * (1.0f - thr_norm);
    Pen thresholdPen(Color(255, 255, 0, 0), 1.5f);
    thresholdPen.SetDashStyle(DashStyleDash);
    graphics.DrawLine(&thresholdPen, PointF(graphRect.X, thr_y),
        PointF(graphRect.X + graphRect.Width, thr_y));

    float rul_x_norm = (float)((rul_mean - x_min) / (x_max - x_min));
    float rul_x = graphRect.X + rul_x_norm * graphRect.Width;
    Pen rulPen(Color(200, 0, 0, 255), 1.5f);
    rulPen.SetDashStyle(DashStyleDot);
    graphics.DrawLine(&rulPen, PointF(rul_x, graphRect.Y),
        PointF(rul_x, graphRect.Y + graphRect.Height));

    SolidBrush rulBrush(Color(255, 0, 0, 255));
    graphics.FillEllipse(&rulBrush, (REAL)(rul_x - 5), (REAL)(thr_y - 5), (REAL)10, (REAL)10);

    CString rulText;
    rulText.Format(_T("%.1f Month"), rul_mean);
    Gdiplus::Font rulFont(L"¸¼Àº °íµñ", 10, FontStyleBold);
    SolidBrush rulTextBrush(Color(255, 0, 0, 255));
    RectF rulTextRect(rul_x + 8, thr_y - 18, 100, 25);
    graphics.DrawString(rulText, -1, &rulFont, rulTextRect, NULL, &rulTextBrush);

    Gdiplus::Font axisFont(L"¸¼Àº °íµñ", 7);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    for (int i = 0; i <= 5; i++)
    {
        float yPos = graphRect.Y + graphRect.Height * i / 5.0f;
        double value = 1.0 - (2.0 * i / 5.0);

        CString label;
        label.Format(_T("%.1f"), value);

        RectF labelRect(graphRect.X - 45, yPos - 8, 40, 16);
        graphics.DrawString(label, -1, &axisFont, labelRect, &format, &textBrush);
    }

    format.SetAlignment(StringAlignmentCenter);

    int num_ticks = 5;
    double x_range = x_max - x_min;
    double x_step = x_range / num_ticks;

    for (int i = 0; i <= num_ticks; i++)
    {
        float xPos = graphRect.X + graphRect.Width * i / (float)num_ticks;
        double value = x_min + x_step * i;

        CString label;
        if (value >= 1000)
            label.Format(_T("%.0f"), value);
        else if (value >= 100)
            label.Format(_T("%.0f"), value);
        else if (value >= 10)
            label.Format(_T("%.0f"), value);
        else
            label.Format(_T("%.1f"), value);

        RectF labelRect(xPos - 25, graphRect.Y + graphRect.Height + 5, 50, 16);
        graphics.DrawString(label, -1, &axisFont, labelRect, &format, &textBrush);
    }
}