#pragma once

#include "ReBitmap.h"
#include "Text.h"
#include "FontDefine.h"

class CLine;
class CRectangle;
class CPolygon;
class CRect;
class __declspec(dllexport) CReDraw 
{
public:
	static CReDraw * GetInstance();
	virtual ~CReDraw();

	bool Initial(HWND , int nWidth, int nHeight);
	
	bool Flip();
	bool Flip(LPRECT rt);
	void Release();

	void SetDrawRect(LPRECT lpRect);
	void Clear(COLORREF col);

	void DrawLine(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col);
	void DrawLineTrans(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col);
	void DrawDotLine(int nStartX, int nStartY, int nEndX, int nEndY, int nStep, COLORREF col);
	void DrawAntiAliasLine(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col);
	void DrawAntiRoadBoundary(int nStartX, int nStartY, int nEndX, int nEndY, const vector<COLORREF>& vColBK, COLORREF col);

	void DrawAntiAliasEllipse(LPRECT lpRect, COLORREF col);
	void DrawRect(LPRECT lpRect, COLORREF col);
	void FillRect(LPRECT lpRect, COLORREF col);
	void FillVerticalGradient(LPRECT lpRect, COLORREF ColStart, COLORREF ColEnd);
    void TransparentRect(LPRECT lpRect, COLORREF col, float fAlpha);

	void DrawPolyLine(LPPOINT lpPoints, int nCount, COLORREF col);
	void DrawSwitchColPolyLine(LPPOINT lpPoints, int nCount, int nStep, COLORREF col1, COLORREF col2);
	void DrawPolygon(LPPOINT lpPoints, int nCount, COLORREF col);
	void FillAnyPolygon(LPPOINT lpPoints, int nCount, COLORREF col);
	void FillConvexPolygon(LPPOINT lpPoints, int nCount, COLORREF col);

	void TextInitial(CLocalCode *pCode);
    void SetLocalCode(CLocalCode *pCode);
    void SetTextColor(const COLORREF& colText);
    void SetBoundaryColor(const COLORREF& colBound);
    RECT GetTextRect(const char *pText);	
    void DrawStr(const char *pText,int nX, int nY, HFONT hFont);
	void DrawStr(const char *pText,Font_Size1 nFontType, int nX, int nY,Text_Type tType = ONE_COLOR, bool bAdjust = false);
    void DrawRectStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay, Font_Weight nFontWeight = Font_Thin, Text_Type tType = ONE_COLOR);
    void DrawRectHighStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay,const char *pHighText,Text_Type tType = ONE_COLOR);	

	bool IsText(const char *pStr);
	bool IsText(const char *pStr,const int nIndex);

	SIZE GetFontSize(Font_Size1 type);
	SIZE GetNumFontSize(Font_Size1 type);
	SIZE GetStrSize(const char *pText, int nFontSize);
	SIZE GetStrSize(const char *pText, Font_Size1 type);

	char* ReverseStr(const char *pStr);
        char* GetLowerCaseString(const char *pStr);
	int GetStrokeOfText(const char *pStr);
	int GetStrokeOfTextCount();
	int GetStrLength(const char *pStr);

	void Smooth() {};

	CReBitmap * GetNewBitmap();
	static HDC GetMemoryDC();

	static BYTE * GetBuffer();
	static SIZE * GetBufSize();
	static RECT * GetDrawRect();
	static int GetPitch();

	static int    GetBpp();
	
private:
	CReDraw();
	void Reset();
	bool InitialDrawTool();
	void DrawScaleStr(const char *pText,Font_Size1 nFontType, int nX, int nY,int nFontSize, Text_Type tType = ONE_COLOR);


public:
	static int s_Bpp;
	static int s_nPitch;
	static BYTE *s_pBuffer;

private:
	static CReDraw * s_pThis;
	static SIZE s_szBmp;
	static RECT s_rtDraw;
	HDC m_hMemDC;

	HWND m_hWnd;
	HDC hDCWnd;
	HBITMAP m_hBmp;
	CLine * m_pLine;
	CRectangle * m_pRect;
	CPolygon *m_pPoly;
	CText * m_pText;
	HGDIOBJ m_hOldBmp;

};

