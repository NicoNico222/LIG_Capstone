#pragma once
#include <vector>
#include <string>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

struct IMUData
{
    vector<double> XA, YA, ZA, XW, YW, ZW;
    vector<double> R, P, Y, N, E, D;
    int dataSize;

    IMUData() : dataSize(0) {}
};

class CGraphHelper
{
public:
    CGraphHelper();
    ~CGraphHelper();

    bool LoadCSV(const CString& filePath, IMUData& data);

    void DrawGraph(CDC* pDC, CRect rect, const IMUData& data, bool isXData);

private:

    ULONG_PTR m_gdiplusToken;


    void DrawSingleGraph(Graphics& graphics, RectF rect,
        const vector<double>& data,
        const CString& title,
        int maxPoints = 10000);


    void DrawAxis(Graphics& graphics, RectF rect);

    void DrawTitle(Graphics& graphics, RectF rect, const CString& title);
};