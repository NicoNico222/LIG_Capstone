#include "pch.h"
#include "GraphHelper.h"
#include <fstream>
#include <sstream>
#include <algorithm>

CGraphHelper::CGraphHelper()
{
    // GDI+ 초기화
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CGraphHelper::~CGraphHelper()
{
    // GDI+ 종료
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
            data.XA.push_back(values[3]);
            data.YA.push_back(values[4]);
            data.ZA.push_back(values[5]);
            data.XW.push_back(values[6]);
            data.YW.push_back(values[7]);
            data.ZW.push_back(values[8]);
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

    // 배경 흰색으로
    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    // 3x2 그리드 계산
    int margin = 10;
    int graphWidth = (rect.Width() - margin * 4) / 2;
    int graphHeight = (rect.Height() - margin * 4) / 3;

    if (isXData)
    {
        // X 데이터 - Python과 동일한 순서
        // for i in range(3):
        //     axes[i,0].plot(test_X[:, i])      -> XA, YA, ZA (왼쪽)
        //     axes[i,1].plot(test_X[:, i+3])    -> XW, YW, ZW (오른쪽)

        for (int i = 0; i < 3; i++)
        {
            // 왼쪽: XA, YA, ZA (i = 0, 1, 2)
            int leftX = rect.left + margin;
            int leftY = rect.top + margin + i * (graphHeight + margin);
            RectF leftRect((REAL)leftX, (REAL)leftY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, leftRect, data.XA, _T("XA"));
            else if (i == 1) DrawSingleGraph(graphics, leftRect, data.YA, _T("YA"));
            else DrawSingleGraph(graphics, leftRect, data.ZA, _T("ZA"));

            // 오른쪽: XW, YW, ZW (i+3 = 3, 4, 5)
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
        // Y 데이터 - Python과 동일한 순서
        // for i in range(3):
        //     axes[i,0].plot(test_Y[:, i])      -> R, P, Y (왼쪽)
        //     axes[i,1].plot(test_Y[:, i+3])    -> N, E, D (오른쪽)

        for (int i = 0; i < 3; i++)
        {
            // 왼쪽: R DIFF, P DIFF, Y DIFF
            int leftX = rect.left + margin;
            int leftY = rect.top + margin + i * (graphHeight + margin);
            RectF leftRect((REAL)leftX, (REAL)leftY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, leftRect, data.R, _T("R DIFF"));
            else if (i == 1) DrawSingleGraph(graphics, leftRect, data.P, _T("P DIFF"));
            else DrawSingleGraph(graphics, leftRect, data.Y, _T("Y DIFF"));

            // 오른쪽: N DIFF, E DIFF, D DIFF
            int rightX = rect.left + margin + graphWidth + margin;
            int rightY = rect.top + margin + i * (graphHeight + margin);
            RectF rightRect((REAL)rightX, (REAL)rightY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, rightRect, data.N, _T("N DIFF"));
            else if (i == 1) DrawSingleGraph(graphics, rightRect, data.E, _T("E DIFF"));
            else DrawSingleGraph(graphics, rightRect, data.D, _T("D DIFF"));
        }
    }
}

void CGraphHelper::DrawSingleGraph(Graphics& graphics, RectF rect,
    const vector<double>& data,
    const CString& title,
    int maxPoints)
{
    if (data.empty()) return;

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
    double range = maxVal - minVal;
    if (range < 1e-10) range = 1.0;

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
        float y_norm = (float)((data[i] - minVal) / range);

        float x = graphRect.X + x_norm * graphRect.Width;
        float y = graphRect.Y + graphRect.Height * (1.0f - y_norm);

        points.push_back(PointF(x, y));
    }

    if (points.size() > 1)
    {
        graphics.DrawLines(&dataPen, &points[0], (int)points.size());
    }

    Gdiplus::Font font(L"맑은 고딕", 8);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);
    format.SetLineAlignment(StringAlignmentCenter);

    for (int i = 0; i <= 5; i++)
    {
        float yPos = graphRect.Y + graphRect.Height * i / 5.0f;
        double value = 1.0 - (i / 5.0);

        CString label;
        label.Format(_T("%.1f"), value);

        RectF labelRect(graphRect.X - 50, yPos - 8, 45, 16);
        graphics.DrawString(label, -1, &font, labelRect, &format, &textBrush);
    }

    format.SetAlignment(StringAlignmentCenter);
    for (int i = 0; i <= 5; i++)
    {
        float xPos = graphRect.X + graphRect.Width * i / 5.0f;
        double value = i / 5.0;

        CString label;
        label.Format(_T("%.1f"), value);

        RectF labelRect(xPos - 20, graphRect.Y + graphRect.Height + 5, 40, 16);
        graphics.DrawString(label, -1, &font, labelRect, &format, &textBrush);
    }

    CString xlabel = _T("normalized time");
    RectF xlabelRect(graphRect.X, graphRect.Y + graphRect.Height + 22,
        graphRect.Width, 16);
    graphics.DrawString(xlabel, -1, &font, xlabelRect, &format, &textBrush);

    StringFormat vformat;
    vformat.SetAlignment(StringAlignmentCenter);
    vformat.SetLineAlignment(StringAlignmentCenter);
    vformat.SetFormatFlags(StringFormatFlagsDirectionVertical);

    CString ylabel = _T("normalized value");
    RectF ylabelRect(graphRect.X - 55, graphRect.Y, 16, graphRect.Height);
    graphics.DrawString(ylabel, -1, &font, ylabelRect, &vformat, &textBrush);
}

void CGraphHelper::DrawAxis(Graphics& graphics, RectF rect)
{
    Pen axisPen(Color(255, 0, 0, 0), 1.0f);

    // Y축
    graphics.DrawLine(&axisPen,
        PointF(rect.X, rect.Y),
        PointF(rect.X, rect.Y + rect.Height));

    // X축
    graphics.DrawLine(&axisPen,
        PointF(rect.X, rect.Y + rect.Height),
        PointF(rect.X + rect.Width, rect.Y + rect.Height));

    // 그리드
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
    Gdiplus::Font font(L"맑은 고딕", 10, FontStyleBold);
    SolidBrush brush(Color(255, 0, 0, 0));
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    graphics.DrawString(title, -1, &font, rect, &format, &brush);
}