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

    for (int i = 0; i <= 8; i++) // [수정] 0부터 8까지 (총 9개 눈금)
    {
        float yPos = graphRect.Y + graphRect.Height * i / 8.0f; // [수정] 8.0f로 나눠서 위치 계산
        double value = 1.0 - (2.0 * i / 8.0);                  // [수정] 2.0 / 8.0 = 0.25 간격

        // 그리드 선
        graphics.DrawLine(&gridPen, PointF(graphRect.X, yPos), PointF(graphRect.X + graphRect.Width, yPos));

        // 레이블
        CString label;
        // 0.25 간격(0.75, 0.25, -0.25, -0.75)을 정확히 표시하기 위해 %.2f를 사용
        label.Format(_T("%.2f"), value); // [수정] %.1f 대신 %.2f 사용
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

void RULGraphHelper::DrawLegend(CDC* pDC, CRect rect, int ci)
{
    Graphics graphics(pDC->m_hDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // 1. 배경 및 테두리
    SolidBrush bgBrush(Color(255, 255, 255, 255));
    graphics.FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());

    Pen borderPen(Color(255, 200, 200, 200), 1.0f);
    graphics.DrawRectangle(&borderPen, rect.left, rect.top, rect.Width() - 1, rect.Height() - 1);

    // 2. 폰트 설정
    Gdiplus::Font font(L"맑은 고딕", 10, FontStyleRegular); // 글자 크기 약간 줄임
    SolidBrush textBrush(Color(255, 0, 0, 0));
    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentCenter);

    // 3. 항목 정의
    struct LegendItem {
        int type; // 0:Line, 1:Dot, 2:Rect, 3:Dash
        Color color;
        CString text;
    };

    CString ciText;
    ciText.Format(_T("%d%% CI Region"), ci);

    LegendItem items[] = {
        // 왼쪽 열 (4개)
        { 0, Color(255, 31, 119, 180), _T("A train ~ B predict Regression ") },
        { 1, Color(255, 44, 160, 44), _T("B Predictive Mean") },
        { 1, Color(255, 255, 127, 14), _T("A Input") },
        { 1, Color(255, 255, 0, 0),    _T("B Target") },
        // 오른쪽 열 (3개)
        { 2, Color(80, 174, 199, 232), ciText },
        { 3, Color(255, 255, 0, 0),    _T("Threshold +") },
        { 1, Color(255, 0, 0, 255),    _T("Faulty Point") }
    };

    // 4. 2단 레이아웃 배치 계산
    float startX = (float)rect.left + 10.0f;
    float startY = (float)rect.top + 22.0f;
    float lineHeight = 25.0f;
    float iconWidth = 15.0f;

    // 두 번째 열이 시작될 X 위치 (전체 너비의 약 55% 지점)
    float col2Offset = (float)rect.Width() * 0.65f;

    for (int i = 0; i < 7; i++)
    {
        // 열(Column)과 행(Row) 결정
        // 0~3번 인덱스: 0열 / 4~6번 인덱스: 1열
        int col = (i < 4) ? 0 : 1;
        int row = (i < 4) ? i : (i - 4);

        float currentX = startX + (col * col2Offset);
        float currentY = startY + (row * lineHeight);

        // 텍스트 영역
        RectF textRect(currentX + iconWidth + 5, currentY - 15, col2Offset - iconWidth - 5, 25);

        // 아이콘 좌표
        float iconMidY = currentY;
        float iconLeft = currentX;
        float iconRight = currentX + iconWidth;

        // 타입별 그리기
        if (items[i].type == 0) // 실선
        {
            Pen linePen(items[i].color, 2.0f);
            graphics.DrawLine(&linePen, PointF(iconLeft, iconMidY), PointF(iconRight, iconMidY));
        }
        else if (items[i].type == 1) // 점
        {
            SolidBrush dotBrush(items[i].color);
            float dotSize = 6.0f;
            graphics.FillEllipse(&dotBrush, (iconLeft + iconRight) / 2 - dotSize / 2, iconMidY - dotSize / 2, dotSize, dotSize);
        }
        else if (items[i].type == 2) // CI 사각형
        {
            SolidBrush rectBrush(items[i].color);
            float rectH = 8.0f;
            graphics.FillRectangle(&rectBrush, iconLeft, iconMidY - rectH / 2, iconWidth, rectH);
        }
        else if (items[i].type == 3) // 점선
        {
            Pen dashPen(items[i].color, 1.5f);
            dashPen.SetDashStyle(DashStyleDash);
            graphics.DrawLine(&dashPen, PointF(iconLeft, iconMidY), PointF(iconRight, iconMidY));
        }

        graphics.DrawString(items[i].text, -1, &font, textRect, &format, &textBrush);
    }
}

void RULGraphHelper::DrawPredictionGraph(CDC* pDC, CRect rect, const PredictionGraphData& data)
{
    if (data.samples.empty() || data.mean.size() != 6) return;

    Graphics graphics(pDC->m_hDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    int margin = 15;
    int graphWidth = (rect.Width() - margin * 4) / 3;
    int graphHeight = (rect.Height() - margin * 3) / 2;

    vector<CString> titles = {
        _T("Roll Drift"), _T("Pitch Drift"), _T("Yaw Drift"),
        _T("North Drift"), _T("East Drift"), _T("Down Drift")
    };

    for (int i = 0; i < 6; i++)
    {
        int row = i / 3;
        int col = i % 3;

        int x = rect.left + margin + col * (graphWidth + margin);
        int y = rect.top + margin + row * (graphHeight + margin);

        RectF graphRect((REAL)x, (REAL)y, (REAL)graphWidth, (REAL)graphHeight);

        vector<double> axis_samples;
        for (const auto& sample : data.samples)
        {
            if (sample.size() > i)
                axis_samples.push_back(sample[i]);
        }

        DrawSinglePredictionGraph(graphics, graphRect, axis_samples,
            data.mean[i], data.ci_lower[i], data.ci_upper[i],
            data.true_values[i], data.ci, titles[i]);
    }
}

void RULGraphHelper::DrawSinglePredictionGraph(Graphics& graphics, RectF rect,
    const vector<double>& samples, double mean_val, double ci_lower, double ci_upper,
    double true_val, int ci, const CString& title)
{
    if (samples.empty()) return;

    SolidBrush bgBrush(Color(255, 245, 245, 245));
    graphics.FillRectangle(&bgBrush, rect);

    Pen borderPen(Color(255, 200, 200, 200), 1.0f);
    graphics.DrawRectangle(&borderPen, rect);

    float titleHeight = 30.0f;
    Gdiplus::Font titleFont(L"맑은 고딕", 10, FontStyleBold);
    SolidBrush titleBrush(Color(255, 0, 0, 0));
    StringFormat titleFormat;
    titleFormat.SetAlignment(StringAlignmentCenter);
    titleFormat.SetLineAlignment(StringAlignmentCenter);
    RectF titleRect(rect.X, rect.Y + 5, rect.Width, titleHeight);
    graphics.DrawString(title, -1, &titleFont, titleRect, &titleFormat, &titleBrush);

    // [수정 1] 하단 여백을 줄여 그래프 크기 확보 (범례공간 + 축 텍스트 공간)
    float bottomMargin = 75.0f;

    RectF plotRect(rect.X + 50, rect.Y + titleHeight + 10,
        rect.Width - 75, rect.Height - titleHeight - bottomMargin);

    double rawMin = *std::min_element(samples.begin(), samples.end());
    double rawMax = *std::max_element(samples.begin(), samples.end());

    rawMin = min(rawMin, min(mean_val, min(ci_lower, min(ci_upper, true_val))));
    rawMax = max(rawMax, max(mean_val, max(ci_lower, max(ci_upper, true_val))));

    AxisInfo xAxis = CalculateNiceAxis(rawMin, rawMax, 8);
    double xMin = xAxis.min_val;
    double xMax = xAxis.max_val;
    double xRange = xMax - xMin;
    if (xRange == 0) xRange = 1.0;

    Pen axisPen(Color(255, 100, 100, 100), 1.0f);
    graphics.DrawLine(&axisPen,
        PointF(plotRect.X, plotRect.Y + plotRect.Height),
        PointF(plotRect.X + plotRect.Width, plotRect.Y + plotRect.Height));
    graphics.DrawLine(&axisPen,
        PointF(plotRect.X, plotRect.Y),
        PointF(plotRect.X, plotRect.Y + plotRect.Height));

    Pen gridPen(Color(100, 200, 200, 200), 0.5f);
    gridPen.SetDashStyle(DashStyleDot);

    Gdiplus::Font axisFont(L"맑은 고딕", 8);
    SolidBrush axisBrush(Color(255, 0, 0, 0));
    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);

    for (int i = 0; i < xAxis.tick_count; i++)
    {
        double currentVal = xAxis.min_val + i * xAxis.step;

        if (currentVal < xMin - 1e-9 || currentVal > xMax + 1e-9) continue;

        float x_norm_ratio = (float)((currentVal - xMin) / xRange);
        float xPos = plotRect.X + x_norm_ratio * plotRect.Width;

        if (i > 0 && i < xAxis.tick_count - 1)
            graphics.DrawLine(&gridPen, PointF(xPos, plotRect.Y), PointF(xPos, plotRect.Y + plotRect.Height));

        CString label;
        if (abs(currentVal - round(currentVal)) < 1e-5) label.Format(_T("%.0f"), currentVal);
        else label.Format(_T("%.2f"), currentVal);

        RectF labelRect(xPos - 30, plotRect.Y + plotRect.Height + 5, 60, 20);
        graphics.DrawString(label, -1, &axisFont, labelRect, &centerFormat, &axisBrush);
    }

    double maxDensity = 2.5;
    DrawKDE(graphics, plotRect, samples, xMin, xMax, maxDensity);

    Pen meanPen(Color(255, 255, 0, 0), 2.0f);
    meanPen.SetDashStyle(DashStyleDash);

    auto GetXPos = [&](double val, double minV, double maxV, float width) -> float {
        if (maxV - minV == 0) return plotRect.X;
        return plotRect.X + width * (float)((val - minV) / (maxV - minV));
        };

    float meanX = GetXPos(mean_val, xMin, xMax, plotRect.Width);
    graphics.DrawLine(&meanPen, PointF(meanX, plotRect.Y), PointF(meanX, plotRect.Y + plotRect.Height));

    Pen ciPen(Color(255, 255, 165, 0), 1.5f);
    ciPen.SetDashStyle(DashStyleDot);

    float ciLowerX = GetXPos(ci_lower, xMin, xMax, plotRect.Width);
    float ciUpperX = GetXPos(ci_upper, xMin, xMax, plotRect.Width);

    graphics.DrawLine(&ciPen, PointF(ciLowerX, plotRect.Y), PointF(ciLowerX, plotRect.Y + plotRect.Height));
    graphics.DrawLine(&ciPen, PointF(ciUpperX, plotRect.Y), PointF(ciUpperX, plotRect.Y + plotRect.Height));

    Pen truePen(Color(255, 0, 128, 0), 2.0f);
    float trueX = GetXPos(true_val, xMin, xMax, plotRect.Width);
    graphics.DrawLine(&truePen, PointF(trueX, plotRect.Y), PointF(trueX, plotRect.Y + plotRect.Height));

    CString xlabel = _T("Drift Value");
    RectF xlabelRect(plotRect.X, plotRect.Y + plotRect.Height + 25, plotRect.Width, 20);
    graphics.DrawString(xlabel, -1, &axisFont, xlabelRect, &centerFormat, &axisBrush);

    StringFormat leftFormat;
    leftFormat.SetAlignment(StringAlignmentFar);
    leftFormat.SetLineAlignment(StringAlignmentCenter);

    for (int i = 0; i <= 4; i++)
    {
        double densityVal = i * 0.5;
        float y = plotRect.Y + plotRect.Height - (plotRect.Height * (float)(densityVal / maxDensity));

        if (i > 0)
            graphics.DrawLine(&gridPen, PointF(plotRect.X, y), PointF(plotRect.X + plotRect.Width, y));

        CString label;
        label.Format(_T("%.1f"), densityVal);
        RectF labelRect(plotRect.X - 50, y - 10, 45, 20);
        graphics.DrawString(label, -1, &axisFont, labelRect, &leftFormat, &axisBrush);
    }

    GraphicsState state = graphics.Save();
    graphics.TranslateTransform(plotRect.X - 40, plotRect.Y + plotRect.Height / 2);
    graphics.RotateTransform(-90);

    StringFormat rotFormat;
    rotFormat.SetAlignment(StringAlignmentCenter);
    rotFormat.SetLineAlignment(StringAlignmentCenter);

    CString ylabel = _T("Density");
    RectF ylabelRect(-plotRect.Height / 2, -7, plotRect.Height, 15);
    graphics.DrawString(ylabel, -1, &axisFont, ylabelRect, &rotFormat, &axisBrush);

    graphics.Restore(state);

    // [수정 2] 범례 위치 조정 (화면 최하단 기준 -40 위치)
    float legendY = rect.Y + rect.Height - 23.0f;
    float legendX = rect.X + 10;
    float col2X = legendX + 165.0f; // 2열 시작 위치
    float lineLen = 30.0f;
    float spacing = 15.0f;

    Gdiplus::Font legendFont(L"맑은 고딕", 10);
    Pen kdePen(Color(255, 100, 149, 237), 2.0f);

    // 1행
    graphics.DrawLine(&kdePen, PointF(legendX, legendY), PointF(legendX + lineLen, legendY));
    graphics.DrawString(_T("Predicted Distribution"), -1, &legendFont,
        PointF(legendX + lineLen + 5, legendY - 7), &axisBrush);

    CString ciLabel;
    ciLabel.Format(_T("%d%% CI"), ci);
    graphics.DrawLine(&ciPen, PointF(col2X, legendY), PointF(col2X + lineLen, legendY));
    graphics.DrawString(ciLabel, -1, &legendFont,
        PointF(col2X + lineLen + 5, legendY - 7), &axisBrush);

    // 2행
    legendY += spacing;

    graphics.DrawLine(&meanPen, PointF(legendX, legendY), PointF(legendX + lineLen, legendY));
    graphics.DrawString(_T("Mean"), -1, &legendFont,
        PointF(legendX + lineLen + 5, legendY - 7), &axisBrush);

    graphics.DrawLine(&truePen, PointF(col2X, legendY), PointF(col2X + lineLen, legendY));
    graphics.DrawString(_T("True Value"), -1, &legendFont,
        PointF(col2X + lineLen + 5, legendY - 7), &axisBrush);
}


void RULGraphHelper::DrawKDE(Graphics& graphics, RectF rect, const vector<double>& samples,
    double min_val, double max_val, double max_density)
{
    if (samples.empty()) return;

    int numPoints = 500;

    double mean = 0.0;
    for (double s : samples) mean += s;
    mean /= samples.size();

    double variance = 0.0;
    for (double s : samples) variance += (s - mean) * (s - mean);
    variance /= samples.size();
    double std_dev = sqrt(variance);

    double bandwidth = 1.06 * std_dev * pow(samples.size(), -0.2);

    vector<PointF> points;

    for (int i = 0; i < numPoints; i++)
    {
        double x = min_val + (max_val - min_val) * i / (numPoints - 1);
        double density = 0.0;

        for (double sample : samples)
        {
            double diff = (x - sample) / bandwidth;
            density += exp(-0.5 * diff * diff);
        }
        density /= (samples.size() * bandwidth * sqrt(2.0 * 3.14159265359));

        points.push_back(PointF((float)x, (float)density));
    }

    vector<PointF> scaledPoints;
    for (const auto& pt : points)
    {
        float x = rect.X + rect.Width * (pt.X - (float)min_val) / (float)(max_val - min_val);
        float y = rect.Y + rect.Height * (1.0f - pt.Y / (float)max_density);
        scaledPoints.push_back(PointF(x, y));
    }

    if (scaledPoints.size() > 1)
    {
        Pen kdePen(Color(180, 100, 149, 237), 2.0f);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.DrawLines(&kdePen, &scaledPoints[0], (int)scaledPoints.size());
    }
}