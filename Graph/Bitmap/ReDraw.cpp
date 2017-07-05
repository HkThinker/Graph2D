#include "StdAfx.h"
#include "ReDraw.h"
#include "Line.h"
#include "Rectangle.h"
#include "Polygon.h"
#include "Text.h"

CReDraw * CReDraw::s_pThis = 0;
BYTE *CReDraw::s_pBuffer = 0;
int CReDraw::s_Bpp = 2;
int CReDraw::s_nPitch = 0;
SIZE CReDraw::s_szBmp = { 0 };
RECT CReDraw::s_rtDraw = { 0 };

CReDraw * CReDraw::GetInstance()
{
	if (!s_pThis)
	{
		s_pThis = new CReDraw();
	}

	return s_pThis;
}

CReDraw::CReDraw()
{
	m_hWnd = 0;
	hDCWnd = 0;
	m_hBmp = 0;
	m_pLine = 0;
	m_pRect = 0;
	m_pPoly = 0;
	m_pText = 0;
}

CReDraw::~CReDraw()
{
	Release();
}

bool CReDraw::Initial(HWND hWnd,int nWidth, int nHeight)
{
	Reset();

	m_hWnd = hWnd;

	hDCWnd = ::GetDC(m_hWnd);

	s_szBmp.cx = nWidth;
	s_szBmp.cy = nHeight;
	s_nPitch = ((s_szBmp.cx * 2 ) + 3) & 0xfffc;
	m_hMemDC = ::CreateCompatibleDC(hDCWnd);
	
	BITMAPINFO  dibInfo;
	dibInfo.bmiHeader.biBitCount = 16;
	dibInfo.bmiHeader.biClrImportant = 0;
	dibInfo.bmiHeader.biClrUsed = 0;
	dibInfo.bmiHeader.biCompression = 0;
	dibInfo.bmiHeader.biHeight = s_szBmp.cy;
	dibInfo.bmiHeader.biPlanes = 1;
	dibInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	dibInfo.bmiHeader.biSizeImage = s_szBmp.cx * s_szBmp.cy * s_Bpp;
	dibInfo.bmiHeader.biWidth = s_szBmp.cx;
	dibInfo.bmiHeader.biXPelsPerMeter = 0;
	dibInfo.bmiHeader.biYPelsPerMeter = 0;
	dibInfo.bmiColors[0].rgbBlue = 0;
	dibInfo.bmiColors[0].rgbGreen = 0;
	dibInfo.bmiColors[0].rgbRed = 0;
	dibInfo.bmiColors[0].rgbReserved = 0;

	m_hBmp = CreateDIBSection(hDCWnd,
							  (const BITMAPINFO*)&dibInfo,
							  DIB_RGB_COLORS,
							  (void**)&s_pBuffer,
							  NULL, 0);
	
	m_hOldBmp = ::SelectObject(m_hMemDC, m_hBmp);
	RECT rt;
	rt.left = 0;
	rt.top  = 0;
	rt.right  = s_szBmp.cx;
	rt.bottom = s_szBmp.cy;
	SetDrawRect(&rt);

	if (!m_pText)
	{
		return InitialDrawTool();
	}
	return true;
}

bool CReDraw::InitialDrawTool()
{
	m_pLine = new CLine;
	m_pRect = new CRectangle;
	m_pPoly = new CPolygon;

	m_pText = new CText;

	return true;
}

void CReDraw::Reset()
{
	if (m_hBmp)
	{
		::SelectObject(m_hMemDC, m_hOldBmp);
		::DeleteObject(m_hBmp);
		m_hBmp = 0;
	}

	if (m_hMemDC)
	{
		::ReleaseDC(m_hWnd, m_hMemDC);
		m_hMemDC = 0;
	}

	if (hDCWnd)
	{
		::ReleaseDC(NULL, hDCWnd);
		hDCWnd = 0;
	}
}

bool CReDraw::Flip()
{
	::BitBlt(hDCWnd, 0, 0, s_szBmp.cx, s_szBmp.cy, m_hMemDC, 0, 0, SRCCOPY);
	return S_OK;
}

bool CReDraw::Flip(LPRECT rt)
{
#ifdef WINCE
	UINT32 nWidth = rt->right - rt->left;
	UINT32 nHeight = rt->bottom - rt->top;
	::BitBlt(hDCWnd, rt->left, rt->top, nWidth, nHeight,  m_hMemDC, rt->left, rt->top, SRCCOPY);

	return true;
#else
	return Flip();
#endif

}

void CReDraw::Release()
{
	::DeleteObject(m_hBmp);
	::DeleteDC(m_hMemDC);

	m_pText->UnInitial();

	delete m_pLine;
	delete m_pRect;
	delete m_pPoly;
	delete m_pText;
}

void CReDraw::SetDrawRect(LPRECT lpRect)
{
	s_rtDraw.left   =  max(0, lpRect->left);
	s_rtDraw.right  =  min(s_szBmp.cx, lpRect->right);
	s_rtDraw.top    =  max(0, lpRect->top);
	s_rtDraw.bottom =  min(s_szBmp.cx, lpRect->bottom);
}

void CReDraw::Clear(COLORREF col)
{
	CRect rtView(0, 0, s_szBmp.cx, s_szBmp.cy);
	FillRect(&rtView, col);
}

void CReDraw::DrawLine(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col)
{
	m_pLine->DrawLine(nStartX, nStartY, nEndX, nEndY, col);
}

void CReDraw::DrawLineTrans(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col)
{
	m_pLine->DrawLineTrans(nStartX, nStartY, nEndX, nEndY, col);
}

void CReDraw::DrawDotLine(int nStartX, int nStartY, int nEndX, int nEndY, int nStep, COLORREF col)
{
	m_pLine->DrawDotLine(nStartX, nStartY, nEndX, nEndY, nStep , col);
}

void CReDraw::DrawAntiRoadBoundary(int nStartX, int nStartY, int nEndX, int nEndY, const vector<COLORREF>& vColBK, COLORREF col)
{
	m_pLine->DrawAntiBoundaryLine(nStartX, nStartY, nEndX, nEndY, vColBK, col);
}

void CReDraw::DrawAntiAliasLine(int nStartX, int nStartY, int nEndX, int nEndY, COLORREF col)
{
	m_pLine->DrawAntiAliasLine(nStartX, nStartY, nEndX, nEndY, col);
}

void CReDraw::DrawAntiAliasEllipse(LPRECT lpRect, COLORREF col)
{
	m_pRect->DrawAntiAliasEllipse(lpRect->left, lpRect->right, lpRect->top, lpRect->bottom, col);
}

void CReDraw::DrawRect(LPRECT lpRect, COLORREF col)
{
	m_pRect->DrawRect(lpRect, col);
}

void CReDraw::FillRect(LPRECT lpRect, COLORREF col)
{
	m_pRect->FillRect(lpRect, col);
}

void CReDraw::FillVerticalGradient(LPRECT lpRect, COLORREF ColStart, COLORREF ColEnd)
{
	m_pRect->FillVerticalGradient(lpRect, ColStart, ColEnd);
}

void CReDraw::TransparentRect(LPRECT lpRect, COLORREF col, float fAlpha)
{
	m_pRect->TransparentRect(lpRect, col, fAlpha);
}

void CReDraw::DrawPolyLine(LPPOINT lpPoints, int nCount, COLORREF col)
{
	m_pPoly->DrawPolyLine(lpPoints, nCount, col);
}

void CReDraw::DrawSwitchColPolyLine(LPPOINT lpPoints, int nCount, int nStep, COLORREF col1, COLORREF col2)
{
	m_pPoly->DrawSwitchColPolyLine(lpPoints, nCount, nStep, col1, col2);
}

void CReDraw::DrawPolygon(LPPOINT lpPoints, int nCount, COLORREF col)
{
	m_pPoly->DrawPolygon(lpPoints, nCount, col);
}

void CReDraw::FillAnyPolygon(LPPOINT lpPoints, int nCount, COLORREF col)
{
	m_pPoly->FillAnyPolygon(lpPoints, nCount, col);
}

void CReDraw::FillConvexPolygon(LPPOINT lpPoints, int nCount, COLORREF col)
{
	m_pPoly->FillConvexPolygon(lpPoints, nCount, col);
}

void CReDraw::TextInitial(CLocalCode *pCode)
{
	m_pText->SetLocalCode(pCode);
	m_pText->Initial();
}

void CReDraw::DrawStr(const char *pText,Font_Size1 nFontType, int nX, int nY ,Text_Type tType, bool bAdjust)
{
	m_pText->DrawStr(pText, nFontType, nX, nY, tType, bAdjust);
}

RECT CReDraw::GetTextRect(const char *pText)
{
	return m_pText->GetTextRect(pText);
}

void CReDraw::DrawStr(const char *pText, int nX, int nY, HFONT hFont)
{
	m_pText->DrawStr(pText, nX, nY, hFont);
}

void CReDraw::DrawRectStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay,Font_Weight nFontWeight, Text_Type tType)
{
	m_pText->DrawRectStr(pText, nFontType,rStrRect, tShowWay ,nFontWeight, tType);
}

void CReDraw::DrawRectHighStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay,const char *pHighText,Text_Type tType)
{
	m_pText->DrawRectHighStr(pText,nFontType,rStrRect,tShowWay,pHighText,tType);
}

void CReDraw::SetTextColor(const COLORREF& colText)
{
	m_pText->SetTextColor(colText);
}

void CReDraw::SetLocalCode(CLocalCode *pCode)
{
	m_pText->SetLocalCode(pCode);
}

int CReDraw::GetStrokeOfText(const char *pStr)
{
	return m_pText->GetStrokeOfText(pStr);
}

int CReDraw::GetStrokeOfTextCount()
{
	return m_pText->GetStrokeOfTextCount();
}

void CReDraw::SetBoundaryColor(const COLORREF& colBound)
{
	m_pText->SetBoundaryColor(colBound);
}

bool CReDraw::IsText(const char *pStr)
{
	return m_pText->IsText(pStr);
}

bool CReDraw::IsText(const char *pStr,const int nIndex)
{
	return m_pText->IsText(pStr,nIndex);
}


SIZE CReDraw::GetStrSize(const char *pText, Font_Size1 type)
{
	return m_pText->GetStrSize(pText, type);
}

SIZE CReDraw::GetFontSize(Font_Size1 type)
{
	return m_pText->GetFontSize(type);
}

SIZE CReDraw::GetNumFontSize(Font_Size1 type)
{
	return m_pText->GetNumFontSize(type);
}

char* CReDraw::ReverseStr(const char *pStr)
{
	return m_pText->ReverseStr(pStr);
}

int CReDraw::GetStrLength(const char *pStr)
{
	return m_pText->GetStrLength(pStr);
}

CReBitmap * CReDraw::GetNewBitmap()
{
	return new CReBitmap();
}

BYTE * CReDraw::GetBuffer()
{ 
	return s_pBuffer; 
}

SIZE * CReDraw::GetBufSize()
{
	return &s_szBmp;
}

RECT * CReDraw::GetDrawRect()
{
	return &s_rtDraw;
}

int CReDraw::GetBpp()
{
	return s_Bpp;
}

int CReDraw::GetPitch() 
{ 
	return s_nPitch; 
}

char *  CReDraw::GetLowerCaseString(const char *pStr)
{
	return m_pText->GetLowerCaseString(pStr);
}

HDC CReDraw::GetMemoryDC()
{
	return s_pThis->m_hMemDC;
}
