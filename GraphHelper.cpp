#include "pch.h"
#include "GraphHelper.h"
#include <fstream>
#include <sstream>
#include <algorithm>

CGraphHelper::CGraphHelper()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CGraphHelper::~CGraphHelper()
{
    GdiplusShutdown(m_gdiplusToken);
}

bool CGraphHelper::LoadCSV(const CString& filePath, IMUData& data)
{
    data.XA.clear(); data.YA.clear(); data.ZA.clear();
    data.XW.clear(); data.YW.clear(); data.ZW.clear();
    data.R.clear(); data.P.clear(); data.Y.clear();
    data.N.clear(); data.E.clear(); data.D.clear();

    std::ifstream file(filePath);
    if (!file.is_open())
        return false;

    std::string line;
    bool isFirstLine = true;
    int lineCount = 0;
    int mode8LineCount = 0;

    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        std::stringstream ss(line);
        std::string cell;
        vector<double> values;

        while (std::getline(ss, cell, ','))
        {
            try {
                values.push_back(std::stod(cell));
            }
            catch (...) {
                values.push_back(0.0);
            }
        }

        if (values.size() >= 15)
        {
            int imu_mode = (int)values[0];

            if (imu_mode != 8)
                continue;

            mode8LineCount++;

            if (mode8LineCount < 200)
                continue;

            if (mode8LineCount > 10200)
                break;

            double xa = values[3] * 1000.0 / 9.8;
            double ya = values[4] * 1000.0 / 9.8;
            double za = values[5] * 1000.0 / 9.8;

            const double PI = 3.14159265358979323846;
            double xw = values[6] * 18000.0 / PI;
            double yw = values[7] * 18000.0 / PI;
            double zw = values[8] * 18000.0 / PI;

            data.XA.push_back(xa);
            data.YA.push_back(ya);
            data.ZA.push_back(za);
            data.XW.push_back(xw);
            data.YW.push_back(yw);
            data.ZW.push_back(zw);

            data.N.push_back(values[9]);
            data.E.push_back(values[10]);
            data.D.push_back(values[11]);
            data.R.push_back(values[12]);
            data.P.push_back(values[13]);
            data.Y.push_back(values[14]);

            lineCount++;
        }
    }

    file.close();
    data.dataSize = lineCount;

    return lineCount > 0;
}

void CGraphHelper::DrawGraph(CDC* pDC, CRect rect, const IMUData& data, bool isXData)
{
    if (data.dataSize == 0) return;

    Graphics graphics(pDC->m_hDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    int margin = 10;
    int graphWidth = (rect.Width() - margin * 4) / 2;
    int graphHeight = (rect.Height() - margin * 4) / 3;

    if (isXData)
    {
        for (int i = 0; i < 3; i++)
        {
            int leftX = rect.left + margin;
            int leftY = rect.top + margin + i * (graphHeight + margin);
            RectF leftRect((REAL)leftX, (REAL)leftY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, leftRect, data.XA, _T("XA"));
            else if (i == 1) DrawSingleGraph(graphics, leftRect, data.YA, _T("YA"));
            else DrawSingleGraph(graphics, leftRect, data.ZA, _T("ZA"));

            int rightX = rect.left + margin + graphWidth + margin;
            int rightY = rect.top + margin + i * (graphHeight + margin);
            RectF rightRect((REAL)rightX, (REAL)rightY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, rightRect, data.XW, _T("XW"));
            else if (i == 1) DrawSingleGraph(graphics, rightRect, data.YW, _T("YW"));
            else DrawSingleGraph(graphics, rightRect, data.ZW, _T("ZW"));
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            int leftX = rect.left + margin;
            int leftY = rect.top + margin + i * (graphHeight + margin);
            RectF leftRect((REAL)leftX, (REAL)leftY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, leftRect, data.R, _T("Roll"));
            else if (i == 1) DrawSingleGraph(graphics, leftRect, data.P, _T("Pitch"));
            else DrawSingleGraph(graphics, leftRect, data.Y, _T("Yaw"));

            int rightX = rect.left + margin + graphWidth + margin;
            int rightY = rect.top + margin + i * (graphHeight + margin);
            RectF rightRect((REAL)rightX, (REAL)rightY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, rightRect, data.N, _T("V_North"));
            else if (i == 1) DrawSingleGraph(graphics, rightRect, data.E, _T("V_East"));
            else DrawSingleGraph(graphics, rightRect, data.D, _T("V_Down"));
        }
    }
}

AxisInfo CGraphHelper::CalculateNiceAxis(double min_val, double max_val, int max_ticks)
{
    AxisInfo info;

    double range = max_val - min_val;
    if (range < 1e-10) range = 1.0;

    double rough_step = range / (max_ticks - 1);
    double magnitude = pow(10, floor(log10(rough_step)));

    double nice_step;
    double residual = rough_step / magnitude;

    if (residual <= 1.0)
        nice_step = 1.0 * magnitude;
    else if (residual <= 2.0)
        nice_step = 2.0 * magnitude;
    else if (residual <= 5.0)
        nice_step = 5.0 * magnitude;
    else
        nice_step = 10.0 * magnitude;

    info.min_val = floor(min_val / nice_step) * nice_step;
    info.max_val = ceil(max_val / nice_step) * nice_step;
    info.step = nice_step;
    info.tick_count = (int)round((info.max_val - info.min_val) / nice_step);

    if (info.tick_count < 2) {
        info.tick_count = 2;
        info.step = (info.max_val - info.min_val) / info.tick_count;
    }

    return info;
}

void CGraphHelper::DrawSingleGraph(Graphics& graphics, RectF rect,
    const vector<double>& data,
    const CString& title,
    int maxPoints)
{
    if (data.empty()) return;

    GraphicsState state = graphics.Save();

    SolidBrush bgBrush(Color(255, 245, 245, 245));
    graphics.FillRectangle(&bgBrush, rect);

    Pen borderPen(Color(255, 200, 200, 200), 1.0f);
    graphics.DrawRectangle(&borderPen, rect);

    float titleHeight = 25.0f;
    RectF titleRect(rect.X, rect.Y, rect.Width, titleHeight);
    DrawTitle(graphics, titleRect, title);

    RectF graphRect(rect.X + 60, rect.Y + titleHeight + 5,
        rect.Width - 75, rect.Height - titleHeight - 40);

    int dataSize = (int)data.size();
    int displaySize = min(dataSize, maxPoints);

    double minVal = *std::min_element(data.begin(), data.begin() + displaySize);
    double maxVal = *std::max_element(data.begin(), data.begin() + displaySize);

    AxisInfo axisInfo = CalculateNiceAxis(minVal, maxVal, 6);

    double yMin = axisInfo.min_val;
    double yMax = axisInfo.max_val;
    double yStep = axisInfo.step;
    int yTickCount = axisInfo.tick_count;

    double range = yMax - yMin;
    if (range < 1e-8) range = 1.0;

    Pen axisPen(Color(255, 0, 0, 0), 1.0f);
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y),
        PointF(graphRect.X, graphRect.Y + graphRect.Height));
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y + graphRect.Height),
        PointF(graphRect.X + graphRect.Width, graphRect.Y + graphRect.Height));

    Pen gridPen(Color(100, 200, 200, 200), 0.5f);
    gridPen.SetDashStyle(DashStyleDot);

    for (int i = 0; i <= yTickCount; i++)
    {
        double tickValue = yMin + i * yStep;
        float y_norm = (float)((yMax - tickValue) / range);
        float y = graphRect.Y + graphRect.Height * y_norm;

        graphics.DrawLine(&gridPen,
            PointF(graphRect.X, y),
            PointF(graphRect.X + graphRect.Width, y));
    }

    for (int i = 1; i <= 4; i++)
    {
        float x = graphRect.X + graphRect.Width * i / 5.0f;
        graphics.DrawLine(&gridPen,
            PointF(x, graphRect.Y),
            PointF(x, graphRect.Y + graphRect.Height));
    }

    Pen dataPen(Color(255, 31, 119, 180), 1.5f);

    vector<PointF> points;
    for (int i = 0; i < displaySize; i++)
    {
        float x_norm = (float)i / (displaySize - 1);
        float y_norm = (float)((yMax - data[i]) / range);

        float x = graphRect.X + x_norm * graphRect.Width;
        float y = graphRect.Y + graphRect.Height * y_norm;
        points.push_back(PointF(x, y));
    }

    if (points.size() > 1)
    {
        graphics.DrawLines(&dataPen, &points[0], (int)points.size());
    }

    Gdiplus::Font font(L"¸¼Àº °íµñ", 8);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);
    format.SetLineAlignment(StringAlignmentCenter);

    for (int i = 0; i <= yTickCount; i++)
    {
        double tickValue = yMin + i * yStep;
        float y_norm = (float)((yMax - tickValue) / range);
        float yPos = graphRect.Y + graphRect.Height * y_norm;

        CString label;

        if (fabs(tickValue) < 1e-10)
        {
            double absStep = fabs(yStep);

            if (absStep >= 1.0)
                label = _T("0");
            else if (absStep >= 0.1)
                label = _T("0.0");
            else if (absStep >= 0.01)
                label = _T("0.00");
            else
                label = _T("0.000");
        }
        else
        {
            double absValue = fabs(tickValue);
            double absStep = fabs(yStep);

            if (absValue < 0.0001 || absValue > 100000)
            {
                label.Format(_T("%.2e"), tickValue);
            }
            else if (absStep >= 1.0)
            {
                if (absStep >= 10.0)
                    label.Format(_T("%.0f"), tickValue);
                else
                    label.Format(_T("%.1f"), tickValue);
            }
            else if (absStep >= 0.01)
            {
                label.Format(_T("%.2f"), tickValue);

                if (label == _T("-0.00"))
                    label = _T("0.00");
            }
            else
            {
                label.Format(_T("%.3f"), tickValue);

                if (label == _T("-0.000"))
                    label = _T("0.000");
            }
        }

        RectF labelRect(graphRect.X - 55, yPos - 8, 50, 16);
        graphics.DrawString(label, -1, &font, labelRect, &format, &textBrush);
    }

    format.SetAlignment(StringAlignmentCenter);
    int xLabels[] = { 0, 2000, 4000, 6000, 8000, 10000 };

    for (int i = 0; i <= 5; i++)
    {
        float xPos = graphRect.X + graphRect.Width * i / 5.0f;
        CString label;
        label.Format(_T("%d"), xLabels[i]);

        RectF labelRect(xPos - 25, graphRect.Y + graphRect.Height + 5, 50, 16);
        graphics.DrawString(label, -1, &font, labelRect, &format, &textBrush);
    }

    CString xlabel = _T("time index");
    RectF xlabelRect(graphRect.X, graphRect.Y + graphRect.Height + 22,
        graphRect.Width, 16);
    graphics.DrawString(xlabel, -1, &font, xlabelRect, &format, &textBrush);

    GraphicsState state2 = graphics.Save();
    graphics.TranslateTransform(graphRect.X - 45, graphRect.Y + graphRect.Height / 2);
    graphics.RotateTransform(-90);

    CString ylabel;
    if (title == _T("XA") || title == _T("YA") || title == _T("ZA"))
        ylabel = _T("acc");
    else if (title == _T("XW") || title == _T("YW") || title == _T("ZW"))
        ylabel = _T("gyro");
    else if (title == _T("Roll") || title == _T("Pitch") || title == _T("Yaw"))
        ylabel = _T("attitude");
    else if (title == _T("V_North") || title == _T("V_East") || title == _T("V_Down"))
        ylabel = _T("velocity");
    else
        ylabel = _T("value");

    RectF ylabelRect(-graphRect.Height / 2, -8, graphRect.Height, 16);
    graphics.DrawString(ylabel, -1, &font, ylabelRect, &format, &textBrush);

    graphics.Restore(state2);
    graphics.Restore(state);
}

void CGraphHelper::DrawAxis(Graphics& graphics, RectF rect)
{
    Pen axisPen(Color(255, 0, 0, 0), 1.0f);

    graphics.DrawLine(&axisPen,
        PointF(rect.X, rect.Y),
        PointF(rect.X, rect.Y + rect.Height));

    graphics.DrawLine(&axisPen,
        PointF(rect.X, rect.Y + rect.Height),
        PointF(rect.X + rect.Width, rect.Y + rect.Height));

    Pen gridPen(Color(100, 200, 200, 200), 0.5f);
    gridPen.SetDashStyle(DashStyleDot);

    for (int i = 1; i < 4; i++)
    {
        float y = rect.Y + rect.Height * i / 4.0f;
        graphics.DrawLine(&gridPen,
            PointF(rect.X, y),
            PointF(rect.X + rect.Width, y));
    }
}

void CGraphHelper::DrawTitle(Graphics& graphics, RectF rect, const CString& title)
{
    Gdiplus::Font font(L"¸¼Àº °íµñ", 10, FontStyleBold);
    SolidBrush brush(Color(255, 0, 0, 0));
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    graphics.DrawString(title, -1, &font, rect, &format, &brush);
}