#pragma once
#include <vector>
#include <string>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

// CSV 데이터 구조체
struct IMUData
{
    vector<double> XA, YA, ZA, XW, YW, ZW;
    vector<double> R, P, Y, N, E, D;
    int dataSize;

    IMUData() : dataSize(0) {}
};

// 그래프 그리기 헬퍼 클래스
class CGraphHelper
{
public:
    CGraphHelper();
    ~CGraphHelper();

    // CSV 파일 로드
    bool LoadCSV(const CString& filePath, IMUData& data);

    // 그래프 그리기 (6개 서브플롯)
    void DrawGraph(CDC* pDC, CRect rect, const IMUData& data, bool isXData);

private:
    // GDI+ 초기화
    ULONG_PTR m_gdiplusToken;

    // 단일 그래프 그리기
    void DrawSingleGraph(Graphics& graphics, RectF rect,
        const vector<double>& data,
        const CString& title,
        int maxPoints = 10000);

    // 축 그리기
    void DrawAxis(Graphics& graphics, RectF rect);

    // 제목 그리기
    void DrawTitle(Graphics& graphics, RectF rect, const CString& title);
};