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
    int mode8LineCount = 0;  // IMU_MODE=8인 행의 카운터

    while (std::getline(file, line))
    {
        // 헤더 스킵
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

        // 최소 15개 컬럼 필요
        if (values.size() >= 15)
        {
            // ========== Python과 동일: IMU_MODE 확인 ==========
            int imu_mode = (int)values[0];

            // IMU_MODE가 8이 아니면 스킵
            if (imu_mode != 8)
                continue;

            mode8LineCount++;

            // ========== Python과 동일: 200~10200 행만 선택 ==========
            if (mode8LineCount < 200)
                continue;

            if (mode8LineCount > 10200)
                break;

            // ========== Python과 동일: 단위 변환 ==========
            // 가속도: 0.001g -> m/s²
            // X_DEL_VEL * 1000 / 9.8
            double xa = values[3] * 1000.0 / 9.8;
            double ya = values[4] * 1000.0 / 9.8;
            double za = values[5] * 1000.0 / 9.8;

            // 각속도: 0.01rad/s -> deg/s  
            // X_DEL_ANG * 18000 / π
            const double PI = 3.14159265358979323846;
            double xw = values[6] * 18000.0 / PI;
            double yw = values[7] * 18000.0 / PI;
            double zw = values[8] * 18000.0 / PI;

            // X 데이터 (단위 변환 완료)
            data.XA.push_back(xa);
            data.YA.push_back(ya);
            data.ZA.push_back(za);
            data.XW.push_back(xw);
            data.YW.push_back(yw);
            data.ZW.push_back(zw);

            // Y 데이터 (원본 값 그대로)
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
    else  // Y 데이터
    {
        for (int i = 0; i < 3; i++)
        {
            // 왼쪽: Roll, Pitch, Yaw
            int leftX = rect.left + margin;
            int leftY = rect.top + margin + i * (graphHeight + margin);
            RectF leftRect((REAL)leftX, (REAL)leftY, (REAL)graphWidth, (REAL)graphHeight);

            if (i == 0) DrawSingleGraph(graphics, leftRect, data.R, _T("Roll"));
            else if (i == 1) DrawSingleGraph(graphics, leftRect, data.P, _T("Pitch"));
            else DrawSingleGraph(graphics, leftRect, data.Y, _T("Yaw"));

            // 오른쪽: V_North, V_East, V_Down
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

    // 범위 계산
    double range = max_val - min_val;
    if (range < 1e-10) range = 1.0;

    // Nice 단계 크기 계산
    double rough_step = range / (max_ticks - 1);
    double magnitude = pow(10, floor(log10(rough_step)));

    // Nice 숫자로 반올림 (1, 2, 5, 10의 배수)
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

    // Nice min/max 계산
    info.min_val = floor(min_val / nice_step) * nice_step;
    info.max_val = ceil(max_val / nice_step) * nice_step;
    info.step = nice_step;
    info.tick_count = (int)round((info.max_val - info.min_val) / nice_step);

    // 최소 눈금 개수 보장
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

    // 배경
    SolidBrush bgBrush(Color(255, 245, 245, 245));
    graphics.FillRectangle(&bgBrush, rect);

    Pen borderPen(Color(255, 200, 200, 200), 1.0f);
    graphics.DrawRectangle(&borderPen, rect);

    // 타이틀
    float titleHeight = 25.0f;
    RectF titleRect(rect.X, rect.Y, rect.Width, titleHeight);
    DrawTitle(graphics, titleRect, title);

    // 그래프 영역
    RectF graphRect(rect.X + 60, rect.Y + titleHeight + 5,
        rect.Width - 75, rect.Height - titleHeight - 40);

    int dataSize = (int)data.size();
    int displaySize = min(dataSize, maxPoints);

    // 원시값의 min/max 계산
    double minVal = *std::min_element(data.begin(), data.begin() + displaySize);
    double maxVal = *std::max_element(data.begin(), data.begin() + displaySize);

    // ========== 수정: CalculateNiceAxis로 균등한 Y축 생성 (눈금 5~6개) ==========
    AxisInfo axisInfo = CalculateNiceAxis(minVal, maxVal, 6);

    double yMin = axisInfo.min_val;
    double yMax = axisInfo.max_val;
    double yStep = axisInfo.step;
    int yTickCount = axisInfo.tick_count;

    double range = yMax - yMin;
    if (range < 1e-8) range = 1.0;

    // 축 그리기
    Pen axisPen(Color(255, 0, 0, 0), 1.0f);
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y),
        PointF(graphRect.X, graphRect.Y + graphRect.Height));
    graphics.DrawLine(&axisPen,
        PointF(graphRect.X, graphRect.Y + graphRect.Height),
        PointF(graphRect.X + graphRect.Width, graphRect.Y + graphRect.Height));

    // 그리드 (CalculateNiceAxis 기반)
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

    // X축 그리드 (5등분)
    for (int i = 1; i <= 4; i++)
    {
        float x = graphRect.X + graphRect.Width * i / 5.0f;
        graphics.DrawLine(&gridPen,
            PointF(x, graphRect.Y),
            PointF(x, graphRect.Y + graphRect.Height));
    }

    // 데이터 선 그리기
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

    // ========== 수정: Y축 레이블 (적응형 포맷) ==========
    Gdiplus::Font font(L"맑은 고딕", 8);
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

        // ========== 0 값 특별 처리 ==========
        if (fabs(tickValue) < 1e-10)
        {
            // 0인 경우: step에 맞춰 포맷 결정 (최대 3자리)
            double absStep = fabs(yStep);

            if (absStep >= 1.0)
                label = _T("0");
            else if (absStep >= 0.1)
                label = _T("0.0");
            else if (absStep >= 0.01)
                label = _T("0.00");
            else  // 0.001 이하도 0.000으로 표시
                label = _T("0.000");
        }
        else
        {
            // 0이 아닌 경우: 스마트 포맷 적용 (최대 3자리)
            double absValue = fabs(tickValue);
            double absStep = fabs(yStep);

            if (absValue < 0.0001 || absValue > 100000)
            {
                // 과학적 표기법 (극단적으로 작거나 큰 값만)
                label.Format(_T("%.2e"), tickValue);
            }
            else if (absStep >= 1.0)
            {
                // 정수 또는 1자리 소수
                if (absStep >= 10.0)
                    label.Format(_T("%.0f"), tickValue);
                else
                    label.Format(_T("%.1f"), tickValue);
            }
            else if (absStep >= 0.01)
            {
                // 2자리 소수
                label.Format(_T("%.2f"), tickValue);

                // -0.00 방지
                if (label == _T("-0.00"))
                    label = _T("0.00");
            }
            else  // absStep < 0.01 (0.001 이하 포함)
            {
                // ========== 수정: 무조건 3자리 소수 ==========
                label.Format(_T("%.3f"), tickValue);

                // -0.000 방지
                if (label == _T("-0.000"))
                    label = _T("0.000");
            }
        }

        RectF labelRect(graphRect.X - 55, yPos - 8, 50, 16);
        graphics.DrawString(label, -1, &font, labelRect, &format, &textBrush);
    }

    // X축 레이블 (0, 2000, 4000, 6000, 8000, 10000)
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

    // X축 라벨
    CString xlabel = _T("time index");
    RectF xlabelRect(graphRect.X, graphRect.Y + graphRect.Height + 22,
        graphRect.Width, 16);
    graphics.DrawString(xlabel, -1, &font, xlabelRect, &format, &textBrush);

    // Y축 라벨
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