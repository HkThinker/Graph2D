#include "StdAfx.h"
#include "ReDraw.h"
#include "Line.h"
#include "Rectangle.h"
#include "Polygon.h"
#include "Text.h"
#include "ReBitmap.h"
#include "ddraw.h"
#include "DebugTool.h"

CReDraw * CReDraw::s_pThis = 0;
BYTE * CReDraw::s_pBuf = 0;
int CReDraw::s_nBpp = 2;
int CReDraw::s_nPitch = 0;
RECT CReDraw::s_rtDraw = { 0 };
RECT CReDraw::s_rtDrawPos = { 0 };
PanelInfo CReDraw::s_panelInfo = { 0 };
CRITICAL_SECTION CReDraw::s_cs; 

LPDIRECTDRAWSURFACE CReDraw::s_pScreen = 0;
LPDIRECTDRAWSURFACE CReDraw::s_pBackScreen = 0;

CReDraw * CReDraw::GetInstance()
{
	if (!s_pThis)
	{
		s_pThis = new CReDraw;
	}
	return s_pThis;
}

CReDraw::CReDraw()
{
	m_hWnd = 0;
	m_pDraw = 0;
	s_pScreen = 0;
	m_pLine = 0;
	m_pRect = 0;
	m_pPoly = 0;
	m_pText = 0;
	m_ptViewOffset.x = 0;
	m_ptViewOffset.y = 0;

	m_bInit = false;

	InitializeCriticalSection(&s_cs); 
}

CReDraw::~CReDraw()
{
	DeleteCriticalSection(&s_cs);
}

bool CReDraw::Initial(HWND hWnd, int nWidth, int nHeight)
{	
	if (m_bInit)
	{
		return true;
	}

	m_hWnd = hWnd;
	HRESULT hr = ::DirectDrawCreate(NULL, &m_pDraw, NULL);
	if (FAILED(hr))
	{
		TRACE(_T("Create DirectDraw Failed"));
		return false;
	}

	hr = m_pDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (FAILED(hr))
	{
		TRACE(_T("DirectDraw SetCooperativeLevel Failed"));
		return false;
	}

	RECT rtView;
	::GetWindowRect(hWnd, &rtView);

	DDSURFACEDESC Ddsd;
	Ddsd.dwSize = sizeof(DDSURFACEDESC);
	Ddsd.dwFlags =  DDSD_CAPS;
	Ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hr = m_pDraw->CreateSurface(&Ddsd, &s_pScreen, NULL); 
	if (FAILED(hr))
	{
		TRACE(_T("DirectDraw CreateSurface Failed"));
		return false;
	}

//	DDSURFACEDESC SurfInfo;
	memset(&m_SurfInfo, 0x00, sizeof(DDSURFACEDESC));
	m_SurfInfo.dwSize = sizeof(DDSURFACEDESC);	
	hr = s_pScreen->GetSurfaceDesc(&m_SurfInfo);
	if (FAILED(hr))
	{
		TRACE(_T("DirectDraw GetSurfaceDesc Failed"));
		return false;
	}

	DDSURFACEDESC Ddsd2;
	Ddsd2.dwSize = sizeof(DDSURFACEDESC);
	Ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	Ddsd2.dwWidth = nWidth;
	Ddsd2.dwHeight = nHeight;
    hr = m_pDraw->CreateSurface(&Ddsd2, &s_pBackScreen, NULL); 
	if (FAILED(hr))
	{
		TRACE(_T("DirectDraw CreateSurface Failed"));
		return false;
	}

	s_panelInfo.nWidth = m_SurfInfo.dwWidth;
	s_panelInfo.nHeight = m_SurfInfo.dwHeight;

	s_nPitch = nWidth * 2;//m_SurfInfo.lPitch;
	s_rtDraw.left = 0;
	s_rtDraw.top = 0;
	s_rtDraw.right = nWidth;
	s_rtDraw.bottom = nHeight;
 
	memset(&m_pixelFormat, 0x00, sizeof(DDPIXELFORMAT));
	m_pixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	hr = s_pScreen->GetPixelFormat(&m_pixelFormat);
	if (FAILED(hr))
	{
		TRACE(_T("DirectDraw GetPixelFormat Failed"));
		return false;
	}

	if (m_pixelFormat.dwRGBBitCount != 16)
	{
		TRACE(_T("Not support this PixelFormat"));
		AfxMessageBox(_T("Not support this PixelFormat"));
		return false;
	}

	DDSURFACEDESC desc;
	memset(&desc, 0x00, sizeof(DDSURFACEDESC));
	desc.dwSize = sizeof(DDSURFACEDESC);
	hr = s_pBackScreen->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
    if (FAILED(hr))
	{
		return false;
	}

	s_pBuf = (BYTE *) desc.lpSurface;

	::GetWindowRect(m_hWnd, &m_rtView);

	CPoint ptViewLT(m_rtView.left, m_rtView.top);
	CPoint ptViewRB(m_rtView.right, m_rtView.bottom);
	::ClientToScreen(m_hWnd, &ptViewLT);
	::ClientToScreen(m_hWnd, &ptViewRB);
	m_rtDest.left = ptViewLT.x; 
	m_rtDest.top  = ptViewLT.y;
	m_rtDest.right = ptViewRB.x;
	m_rtDest.bottom = ptViewRB.y;

	m_bInit = true;
	return InitialTool();
}

void CReDraw::ResetData()
{
	if (s_pBackScreen)
	{
		s_pBackScreen->Release();
	}
	if (s_pScreen)
	{
		s_pScreen->Release();
	}
	if (m_pDraw)
	{
		m_pDraw->Release();
	}
}


bool CReDraw::Flip()
{	
	s_pBackScreen->Unlock(0);

	CRect rtOffset(m_rtView);
	rtOffset.left += m_ptViewOffset.x;
	rtOffset.top += m_ptViewOffset.y;
	rtOffset.right += m_ptViewOffset.x;
	rtOffset.bottom += m_ptViewOffset.y;

	HRESULT hr = s_pScreen->Blt(&rtOffset, s_pBackScreen, &m_rtView, DDBLT_WAIT,NULL);
	if (FAILED(hr))
	{
		ASSERT(0);
		return false;
	}

	DDSURFACEDESC desc;
	memset(&desc, 0x00, sizeof(DDSURFACEDESC));
	desc.dwSize = sizeof(DDSURFACEDESC);
	hr = s_pBackScreen->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
    if (FAILED(hr))
	{
		return false;
	}

	s_pBuf = (BYTE *) desc.lpSurface;

	return true;
}

bool CReDraw::Flip(LPRECT rt)
{
	s_pBackScreen->Unlock(0);

	CRect rtOffset(rt);

	rtOffset.left += m_ptViewOffset.x;
	rtOffset.top += m_ptViewOffset.y;
	rtOffset.right += m_ptViewOffset.x;
	rtOffset.bottom += m_ptViewOffset.y;

	HRESULT hr = s_pScreen->Blt(&rtOffset, s_pBackScreen, rt, DDBLT_WAIT,NULL);
	if (FAILED(hr))
	{
		ASSERT(0);
		return false;
	}

	DDSURFACEDESC desc;
	memset(&desc, 0x00, sizeof(DDSURFACEDESC));
	desc.dwSize = sizeof(DDSURFACEDESC);
	hr = s_pBackScreen->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
    if (FAILED(hr))
	{
		return false;
	}

	s_pBuf = (BYTE *) desc.lpSurface;

	return true;
}


void CReDraw::Release()
{
	ResetData();

	delete m_pLine;
	delete m_pRect;
	delete m_pPoly;
	
	m_pText->UnInitial();
	delete m_pText;
	delete s_pThis;
}

void CReDraw::SetDrawRect(LPRECT lpRect)
{
	s_rtDraw.left   =  max(0, lpRect->left);
	s_rtDraw.right  =  min((int) s_panelInfo.nWidth, lpRect->right);
	s_rtDraw.top    =  max(0, lpRect->top);
	s_rtDraw.bottom =  min((int) s_panelInfo.nHeight, lpRect->bottom);
}

bool CReDraw::InitialTool()
{
	if (m_pLine)
	{
		return true;
	}
	m_pLine = new CLine;
	m_pRect = new CRectangle;
	m_pPoly = new CPolygon;
	m_pText = new CText;	

	return true;
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
	m_pLine->DrawDotLine(nStartX, nStartY, nEndX, nEndY, nStep, col);
}

void CReDraw::DrawAntiRoadBoundary(int nStartX, int nStartY, int nEndX, int nEndY, const vector<COLORREF>& vColBK, COLORREF col)//2009-8-31
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

void CReDraw::FillVerticalGradient(LPRECT lpRect, COLORREF colStart, COLORREF colEnd)
{
	m_pRect->FillVerticalGradient(lpRect, colStart, colEnd);
}

void CReDraw::TransparentRect(LPRECT lpRect, COLORREF col, float fAlpha)
{
	m_pRect->TransparentRect(lpRect, col, fAlpha);
}

void CReDraw::DrawPolyLine(LPPOINT lpPoints, int nCount, COLORREF col)
{
	m_pPoly->DrawPolyLine(lpPoints, nCount, col);
}

void CReDraw::DrawSwitchColPolyLine(LPPOINT lpPoints, int nCount, int nStep, COLORREF col1, COLORREF col2)//2009-9-1
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

void CReDraw::SetLocalCode(CLocalCode *pCode)
{
	m_pText->SetLocalCode(pCode);
}

void CReDraw::SetTextColor(const COLORREF& colText)
{
	m_pText->SetTextColor(colText);
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

char * CReDraw::ReverseStr(const char *pStr)
{
	return m_pText->ReverseStr(pStr);
}
char *  CReDraw::GetLowerCaseString(const char *pStr)
{
	return m_pText->GetLowerCaseString(pStr);
}
int CReDraw::GetStrokeOfText(const char *pStr)
{
	return m_pText->GetStrokeOfText(pStr);
}
int CReDraw::GetStrokeOfTextCount()
{
	return m_pText->GetStrokeOfTextCount();
}
int CReDraw::GetStrLength(const char *pStr)
{
	return m_pText->GetStrLength(pStr);
}

RECT CReDraw::GetTextRect(const char *pText)
{
	return m_pText->GetTextRect(pText);
}

void CReDraw::DrawStr(const char *pText, int nX, int nY, HFONT hFont)
{
	m_pText->DrawStr(pText, nX, nY, hFont);
}

void CReDraw::DrawStr(const char *pText,Font_Size1 nFontType, int nX, int nY ,Text_Type tType, bool bAdjust)
{
	m_pText->DrawStr(pText, nFontType, nX, nY, tType, bAdjust);
}

void CReDraw::DrawRectStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay,Font_Weight nFontWeight, Text_Type tType)
{
	m_pText->DrawRectStr(pText, nFontType,rStrRect, tShowWay ,nFontWeight, tType);
}

void CReDraw::DrawRectHighStr(const char *pText,Font_Size1 nFontType, RECT& rStrRect,TextShowWay tShowWay,const char *pHighText,Text_Type tType)
{
	m_pText->DrawRectHighStr(pText,nFontType,rStrRect,tShowWay,pHighText,tType);
}

PanelInfo * CReDraw::GetPanelInfo()
{
	return &s_panelInfo;
}

RECT * CReDraw::GetDrawRect()
{ 
	return &s_rtDraw;
}
RECT *  CReDraw::GetStartDrawPos( )
{
	return &s_rtDrawPos;
}
int CReDraw::GetPitch() 
{ 
	return s_nPitch; 
}

int CReDraw::GetBpp()
{
	return s_nBpp;
}

void CReDraw::DumpMemory()
{
	MEMORYSTATUS mem;
	GlobalMemoryStatus(&mem);

	CString strAvail;
	strAvail.Format(_T("VM = %dMB\r\n"), mem.dwAvailVirtual);

	AfxMessageBox(strAvail);
}
