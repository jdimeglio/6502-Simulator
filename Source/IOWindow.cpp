/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

// IOWindow.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "IOWindow.h"
#include <memory.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
bool CIOWindow::m_bRegistered= FALSE;
bool CIOWindow::m_bHidden;
CString CIOWindow::m_strClass;
CFont CIOWindow::m_Font;
LOGFONT CIOWindow::m_LogFont=
{
  13,	// LONG lfHeight;
  0,	// LONG lfWidth;
  0,	// LONG lfEscapement;
  0,	// LONG lfOrientation;
  0,	// LONG lfWeight;
  0,	// BYTE lfItalic;
  0,	// BYTE lfUnderline;
  0,	// BYTE lfStrikeOut;
  0,	// BYTE lfCharSet;
  0,	// BYTE lfOutPrecision;
  0,	// BYTE lfClipPrecision;
  0,	// BYTE lfQuality;
  FIXED_PITCH,	// BYTE lfPitchAndFamily;
  "Courier"	// CHAR lfFaceName[LF_FACESIZE];
};

CPoint CIOWindow::m_WndPos= CPoint(0,0);	// po³o¿enie okna
int CIOWindow::m_nInitW= 40;
int CIOWindow::m_nInitH= 25;
COLORREF CIOWindow::m_rgbTextColor= RGB(0,0,0);
COLORREF CIOWindow::m_rgbBackgndColor= RGB(255,255,255);

//-----------------------------------------------------------------------------
// Rejestracja klasy okien

void CIOWindow::RegisterWndClass()
{
  ASSERT(!m_bRegistered);
  if (m_bRegistered)
    return;
  m_strClass = AfxRegisterWndClass(0/*CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS*/,
    ::LoadCursor(NULL,IDC_ARROW), 0, AfxGetApp()->LoadIcon(IDI_IO_WINDOW));
  m_bRegistered = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CIOWindow

IMPLEMENT_DYNCREATE(CIOWindow, CMiniFrameWnd)

CIOWindow::CIOWindow()
{
  m_hWnd = 0;
  m_pData = NULL;
  m_nWidth = m_nHeight = 0;

  m_nPosX = m_nPosY = 0;	// po³o¿enie znaku do wypisania (i kursora)
  m_nCursorCount = 0;		// licznik ukryæ kursora

  if (!m_bRegistered)
    RegisterWndClass();

  m_bHidden = FALSE;

  m_bCursorOn = false;
  m_bCursorVisible = false;

  m_uTimer = 0;
}

CIOWindow::~CIOWindow()
{
  if (m_pData)
    delete []m_pData;
}

//-----------------------------------------------------------------------------
// Nowe okno

bool CIOWindow::Create()
{
  ASSERT(m_hWnd==0);
  CString title;
  title.LoadString(IDS_IO_WINDOW);

  RECT rect= {0,0,100,100};
  rect.left = m_WndPos.x;
  rect.top = m_WndPos.y;
  if (!CMiniFrameWnd::CreateEx(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE, m_strClass, title,
    WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_MOVEFRAME /*| MFS_SYNCACTIVE*/,
    rect, AfxGetMainWnd(), 0))
    return FALSE;

  ModifyStyleEx(0,WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE);
//  SetFont(m_Font,FALSE);
//  CalcFontSize();
  SetSize(m_nInitW,m_nInitH);

  m_uTimer = SetTimer(101, 250, 0);

  SetFocus();

  return TRUE;
}


void CIOWindow::CalcFontSize()		// obliczenie rozmiaru znaków
{
  ASSERT(m_hWnd);
  CClientDC dc(this);
  dc.SelectObject(&m_Font);
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);
  m_nCharH = (int)tm.tmHeight + (int)tm.tmExternalLeading;
  m_nCharW = (int)tm.tmAveCharWidth;
}

//-----------------------------------------------------------------------------
// Ustawienie wymiarów okna

void CIOWindow::SetSize(int w, int h, int resize/* =1*/)
{
  ASSERT(w > 0);
  ASSERT(h > 0);
  m_nInitW = w;
  m_nInitH = h;
  if (m_hWnd==0)	// nie ma jeszcze okna?
    return;
  int new_size= w * h;
  int old_size= m_nWidth * m_nHeight;
  bool change=  m_nWidth != w || m_nHeight != h;
  m_nWidth = w;
  m_nHeight = h;
  if (m_pData==NULL || new_size!=old_size)
  {
    if (m_pData)
      delete []m_pData;
    m_pData = new UINT8[new_size];
  }

  if (resize == 0)	// bez zmiany wymiarów okna?
    return;
  if (resize == -1 && !change)
    return;
  Resize();
}


void CIOWindow::Resize()
{
  CalcFontSize();
  CRect size(0,0,m_nCharW*m_nWidth, m_nCharH*m_nHeight);
  CalcWindowRect(&size,CWnd::adjustOutside);
  SetWindowPos(NULL,0,0,size.Width(),size.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
  Cls();
}

  // odczyt wymiarów okna
void CIOWindow::GetSize(int &w, int &h)
{
  w = m_hWnd ? m_nWidth : m_nInitW;
  h = m_hWnd ? m_nHeight : m_nInitH;
}

  // ustawienie po³o¿enia okna
void CIOWindow::SetWndPos(const POINT &p)
{
  m_WndPos = p;
  if (m_hWnd==0)	// nie ma jeszcze okna?
    return;
  SetWindowPos(NULL,p.x,p.y,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
}

  // odczyt po³o¿enia okna
CPoint CIOWindow::GetWndPos()
{
  if (m_hWnd==0)	// nie ma jeszcze okna?
    return m_WndPos;
  CRect rect;
  GetWindowRect(rect);
  return rect.TopLeft();
}


//-----------------------------------------------------------------------------

int CIOWindow::put(char chr, int x, int y)
{
  if (m_pData==NULL)
    return -1;
  if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
    return -2;
  m_pData[x + y * m_nWidth] = (UINT8)chr;
  return 0;
}

/*
int CIOWindow::puts(const char *str, int len, int x, int y)
{
  if (m_pData==NULL)
    return -1;
  if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
    return -2;

  return 0;
}
*/

int CIOWindow::invalidate(int x, int y)	// obszar znaku pod (x,y) do przerysowania
{
  RECT rect;
//  GetClientRect(rect);
  ASSERT(m_nCharH > 0 && m_nCharW > 0);
  rect.left = x * m_nCharW;
  rect.top = y * m_nCharH;
  rect.right = rect.left + m_nCharW;
  rect.bottom = rect.top + m_nCharH;
//  ASSERT(rect.bottom >= rect.top);
  InvalidateRect(&rect);
  return 0;
}


int CIOWindow::scroll(int dy)		// przesuniêcie napisów o 'dy' wierszy
{
  if (m_pData==NULL)
    return -1;
  if (dy > m_nHeight || dy < 0)
    return -2;
  // przesuniêcie wierszy
  memmove(m_pData, m_pData + dy * m_nWidth, (m_nHeight - dy) * m_nWidth);
  // na odkryte miejsce zera
  memset(m_pData + (m_nHeight - dy) * m_nWidth, 0, dy * m_nWidth);
  // ca³e okno do przerysowania
  Invalidate();
  return 0;
}

//-----------------------------------------------------------------------------
int CIOWindow::PutH(int chr)			// wydrukowanie liczby hex (8 bitow)
{
	int h1= (chr >> 4) & 0x0f;
	int h2= chr & 0x0f;
	char szBuf[4];
	szBuf[0] = h1 > 9 ? h1 + 'A' - 10 : h1 + '0';
	szBuf[1] = h2 > 9 ? h2 + 'A' - 10 : h2 + '0';
	szBuf[2] = '\0';
	return PutS(szBuf);
}


int CIOWindow::PutC(int chr)			// wydrukowanie znaku
{
	HideCursor();

  if (chr == 0x0a) // line feed?
  {
    if (++m_nPosY >= m_nHeight)
    {
      ASSERT(m_nPosY == m_nHeight);
      m_nPosY--;
      scroll(1);		// przesuniêcie napisów o jeden wiersz
    }
  }
  else if (chr == 0x0d) // carriage return?
    m_nPosX = 0;
  else if (chr == 0x08) // backspace?
  {
    if (--m_nPosX < 0)
    {
      m_nPosX = m_nWidth - 1;
      if (--m_nPosY < 0)
	  {
        m_nPosY = 0;
		return 0;
	  }
    }
    if (put(' ', m_nPosX, m_nPosY) < 0)
      return -1;
    invalidate(m_nPosX, m_nPosY);	// obszar pod znakiem do przerysowania
  }
  else
    return PutChr(chr);
  return 0;
}


int CIOWindow::PutChr(int chr)			// wydrukowanie znaku (verbatim)
{
	HideCursor();
  if (put(chr,m_nPosX,m_nPosY) < 0)
    return -1;
  invalidate(m_nPosX,m_nPosY);	// obszar pod znakiem do przerysowania
  if (++m_nPosX >= m_nWidth)
  {
    m_nPosX = 0;
    if (++m_nPosY >= m_nHeight)
    {
      ASSERT(m_nPosY == m_nHeight);
      m_nPosY--;
      scroll(1);		// przesuniêcie napisów o jeden wiersz
    }
  }
  return 0;
}


int CIOWindow::PutS(const char *str, int len/*= -1*/)	// ci¹g znaków do wydrukowania
{
  for (int i=0; i<len || len==-1; i++)
  {
    if (str[i] == '\0')
      break;
    if (PutC(str[i]) < 0)
      return -1;
  }
  return 0;
}


bool CIOWindow::SetPosition(int x, int y)	// ustawienie po³o¿enia dla tekstu
{
  if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
    return FALSE;
  m_nPosX = x;
  m_nPosY = y;
  return TRUE;
}


void CIOWindow::GetPosition(int &x, int &y)	// odczyt po³o¿enia
{
  x = m_nPosX;
  y = m_nPosY;
}


bool CIOWindow::Cls()			// wyczyszczenie okna
{
  if (m_pData==NULL)
    return FALSE;
  memset(m_pData,0,m_nHeight*m_nWidth);	// wyzerowanie
  m_nPosX = m_nPosY = 0;
  Invalidate();				// ca³e okno do przerysowania
  return TRUE;
}


int CIOWindow::Input()			// input
{
	m_bCursorOn = true;
//	SetFocus();

	return m_InputBuffer.GetChar();		// returns available char or 0 if buffer is empty
}

/*
int CIOWindow::Input()			// input
{
  if (theApp.m_global.GetSimulator()->IsBroken())	// execution broken?
    return -1;

  m_bCursorOn = true;
  m_bCursorVisible = true;
  m_uTimer = SetTimer(1, 250, 0);
  DrawCursor();

  SetFocus();

  RunModalLoop();

  KillTimer(m_uTimer);
  m_uTimer = 0;
  if (m_bCursorVisible)
  {
    m_bCursorVisible = false;
    DrawCursor();
  }
  m_bCursorOn = false;

  if (theApp.m_global.GetSimulator()->IsBroken())	// execution broken?
    AfxGetMainWnd()->SetFocus();

  return m_nModalResult;
}
*/

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CIOWindow, CMiniFrameWnd)
  //{{AFX_MSG_MAP(CIOWindow)
  ON_WM_PAINT()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_CHAR()
  ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PASTE, OnPaste)
	//}}AFX_MSG_MAP
  ON_MESSAGE(CIOWindow::CMD_CLS, OnCls)
  ON_MESSAGE(CIOWindow::CMD_PUTC, OnPutC)
  ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
  ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
  ON_MESSAGE(CIOWindow::CMD_IN, OnInput)
  ON_MESSAGE(CIOWindow::CMD_POSITION, OnPosition)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIOWindow message handlers

void CIOWindow::OnPaint()
{
  CPaintDC dc(this);	// device context for painting

  if (m_pData==NULL)
    return;

  dc.SelectObject(&m_Font);
  dc.SetBkMode(OPAQUE);
  dc.SetTextColor(m_rgbTextColor);
  dc.SetBkColor(m_rgbBackgndColor);

  CString line;
  UINT8 *src= m_pData;
  for (int y=0,pos_y=0; y<m_nHeight; y++,pos_y+=m_nCharH)
  {
    TCHAR *dst= line.GetBuffer(m_nWidth);
    for (int i=0; i<m_nWidth; i++)	// znaki jednego wiersza do bufora 'line'
      if ( (*dst++ = TCHAR(*src++)) == 0 )
        dst[-1] = ' ';			// zast¹pienie znaku '\0' przez ' '
    line.ReleaseBuffer(m_nWidth);
    dc.TextOut(0,pos_y,line);		// wydrukowanie wiersza
  }

  DrawCursor();
}

//=============================================================================

void CIOWindow::OnDestroy()
{
  CRect rect;
  GetWindowRect(&rect);
  m_WndPos = rect.TopLeft();	// zapamiêtanie po³o¿enia okna

  if (m_uTimer)
    KillTimer(m_uTimer);
  m_uTimer = 0;

  CMiniFrameWnd::OnDestroy();
}

//=============================================================================

void CIOWindow::PostNcDestroy()
{
//	CMiniFrameWnd::PostNcDestroy();
  m_hWnd = NULL;
}

//=============================================================================

void CIOWindow::OnGetMinMaxInfo(MINMAXINFO* pMMI)
{
  CMiniFrameWnd::OnGetMinMaxInfo(pMMI);

  CRect size(0,0,m_nCharW*m_nWidth,m_nCharH*m_nHeight);
  CalcWindowRect(&size,CWnd::adjustOutside);
//TRACE("vert %d \thorz %d\n",!!(GetStyle() & WS_VSCROLL), !!(GetStyle() & WS_HSCROLL) );
  int w= size.Width();
  if (GetStyle() & WS_VSCROLL)
    w += ::GetSystemMetrics(SM_CXVSCROLL);
//  if (client.Width() < max_size.Width())	// jest suwak?
  int h= size.Height();
  if (GetStyle() & WS_HSCROLL)
    h += ::GetSystemMetrics(SM_CYHSCROLL);
//  if (client.Height() < max_size.Height())	// jest suwak?
/*
  SCROLLINFO si;
  if (GetScrollInfo(SB_HORZ,&si,SIF_PAGE|SIF_RANGE) && si.nMax-si.nMin > (int)si.nPage)
    w += ::GetSystemMetrics(SM_CXVSCROLL);
  if (GetScrollInfo(SB_VERT,&si,SIF_PAGE|SIF_RANGE) && si.nMax-si.nMin > (int)si.nPage)
    h += ::GetSystemMetrics(SM_CYHSCROLL);
*/
  pMMI->ptMaxSize.x = w;
  pMMI->ptMaxSize.y = h;
  pMMI->ptMaxTrackSize.x = w;
  pMMI->ptMaxTrackSize.y = h;
//  pMMI->ptMinTrackSize.x = 1;
//  pMMI->ptMinTrackSize.y = 1;
//  pMMI->ptMaxPosition.x = 0;
//  pMMI->ptMaxPosition.y = 0;

//TRACE("mx %d \tmy %d \ttx %d \ty %d\n", pMMI->ptMaxSize.x, pMMI->ptMaxSize.y, pMMI->ptMaxTrackSize.x, pMMI->ptMaxTrackSize.y);

//  CMiniFrameWnd::OnGetMinMaxInfo(pMMI);
}

//=============================================================================

void CIOWindow::OnSize(UINT nType, int cx, int cy) 
{
  CMiniFrameWnd::OnSize(nType,cx,cy);

  if (nType == SIZE_RESTORED)
  {
    int w= (GetStyle() & WS_VSCROLL) ? ::GetSystemMetrics(SM_CXVSCROLL) : 0;
    int h= (GetStyle() & WS_HSCROLL) ? ::GetSystemMetrics(SM_CYHSCROLL) : 0;
    CRect rect(0,0,m_nCharW*m_nWidth, m_nCharH*m_nHeight);
    CSize size(rect.Width(),rect.Height());
    bool remove= ( cx+w >= size.cx && cy+h >= size.cy );
    SCROLLINFO si_horz=
    {
      sizeof si_horz,
      SIF_PAGE | SIF_RANGE,
      0, size.cx-1,		// min i max
      remove ? size.cx : cx,
      0, 0
    };
    SCROLLINFO si_vert=
    {
      sizeof si_vert,
      SIF_PAGE | SIF_RANGE,
      0, size.cy-1,		// min i max
      remove ? size.cy : cy,
      0, 0
    };
    SetScrollInfo(SB_HORZ,&si_horz);
    SetScrollInfo(SB_VERT,&si_vert);
  }
  else if (nType == SIZE_MAXIMIZED)
    cx = 0;
}

//-----------------------------------------------------------------------------

void CIOWindow::SetColors(COLORREF text, COLORREF backgnd)	// ust. kolorów
{
  m_rgbBackgndColor = backgnd;
  m_rgbTextColor = text;
  if (m_hWnd)
    Invalidate();
}

void CIOWindow::GetColors(COLORREF &text, COLORREF &backgnd)	// odczyt kolorów
{
  text = m_rgbTextColor;
  backgnd = m_rgbBackgndColor;
}

//=============================================================================

afx_msg LRESULT CIOWindow::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
  VERIFY( Cls() );
  if (!m_bHidden)		// okno by³o widoczne?
    if (m_hWnd)
      ShowWindow(SW_NORMAL);
    else
      Create();
  return 1;
}


afx_msg LRESULT CIOWindow::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
  if (m_hWnd && (GetStyle() & WS_VISIBLE))	// okno aktualnie wyœwietlone?
  {
    m_bHidden = FALSE;				// info - okno by³o wyœwietlane
    ShowWindow(SW_HIDE);			// ukrycie okna
  }
  else
    m_bHidden = TRUE;				// info - okno by³o ukryte
  return 1;
}

//=============================================================================

afx_msg LRESULT CIOWindow::OnCls(WPARAM /*wParam*/, LPARAM /* lParam */)
{
  VERIFY( Cls() );
  return 1;
}


afx_msg LRESULT CIOWindow::OnPutC(WPARAM wParam, LPARAM lParam)
{
  if (lParam == 0)
    VERIFY( PutC(int(UINT8(wParam))) == 0 );
  else if (lParam == 1)
    VERIFY( PutChr(int(UINT8(wParam))) == 0 );
  else if (lParam == 2)
    VERIFY( PutH(int(UINT8(wParam))) == 0 );
  else
  { ASSERT(false); }
  return 1;
}


afx_msg LRESULT CIOWindow::OnInput(WPARAM /*wParam*/, LPARAM /* lParam */)
{
  return Input();
}


afx_msg LRESULT CIOWindow::OnPosition(WPARAM wParam, LPARAM lParam)
{
	bool bXPos= !!(wParam & 1);

	if (wParam & 2)	// get pos?
	{
		return bXPos ? m_nPosX : m_nPosY;
	}
	else				// set pos
	{
		int x= m_nPosX;
		int y= m_nPosY;

		if (bXPos)
			x = lParam;
		else
			y = lParam;

		if (x >= m_nWidth)
			x = m_nWidth - 1;
		if (y >= m_nHeight)
			y = m_nHeight - 1;

		if (x != m_nPosX || y != m_nPosY)
		{
			if (m_bCursorVisible && m_bCursorOn)
				DrawCursor(m_nPosX, m_nPosY, false);

			m_nPosX = x;
			m_nPosY = y;

			if (m_bCursorVisible && m_bCursorOn)
				DrawCursor(m_nPosX, m_nPosY, true);
		}
	}

  return 0;
}

//=============================================================================

void CIOWindow::HideCursor()
{
	if (m_bCursorVisible)
	{
		DrawCursor(m_nPosX, m_nPosY, false);
		m_bCursorVisible = false;
	}
	m_bCursorOn = false;
}

// draw cursor
//
void CIOWindow::DrawCursor()
{
  if (m_bCursorOn)
    DrawCursor(m_nPosX, m_nPosY, m_bCursorVisible);
}


void CIOWindow::DrawCursor(int nX, int nY, bool bVisible)
{
  if (m_pData==NULL)
    return;
  if (nX > m_nWidth || nY > m_nHeight || nX < 0 || nY < 0)
  {
    ASSERT(false);
    return;
  }

  // character under the cursor
  char szBuf[2]= { m_pData[nX + nY * m_nWidth], '\0' };
  if (szBuf[0] == '\0')
    szBuf[0] = ' ';

  CClientDC dc(this);

  dc.SelectObject(&m_Font);
  dc.SetBkMode(OPAQUE);

  if (bVisible)
  {
    dc.SetTextColor(m_rgbBackgndColor);
    dc.SetBkColor(m_rgbTextColor);
  }
  else
  {
    dc.SetTextColor(m_rgbTextColor);
    dc.SetBkColor(m_rgbBackgndColor);
  }

  // cursor pos & size
  CPoint ptPos(nX * m_nCharW, nY * m_nCharH);
  CRect rect(ptPos, CSize(m_nCharW, m_nCharH));

  dc.DrawText(szBuf, 1, rect, DT_TOP | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
}



void CIOWindow::OnTimer(UINT nIDEvent)
{
	m_bCursorVisible = !m_bCursorVisible;

	DrawCursor();

	if (!m_bCursorVisible)
		m_bCursorOn = false;

//  CMiniFrameWnd::OnTimer(nIDEvent);
}


void CIOWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	char c= char(nChar);
	if (c)
		m_InputBuffer.PutChar(c);

	//  EndModalLoop(nChar);
}


BOOL CIOWindow::ContinueModal()
{
  if (theApp.m_global.GetSimulator()->IsBroken())	// execution broken?
    return false;

  return CMiniFrameWnd::ContinueModal();
}


void CIOWindow::OnClose()
{
  if (IsWaiting())
  {
    EndModalLoop(-1);	// break
    return;
  }

	ShowWindow(SW_HIDE);
//  CMiniFrameWnd::OnClose();
}


bool CIOWindow::IsWaiting() const
{
	return false;
//  return (m_nFlags & WF_MODALLOOP) != 0;	// in modal loop waiting for input?
}

void CIOWindow::ExitModalLoop()
{
  if (IsWaiting())
    EndModalLoop(-1);
}


void CIOWindow::Paste()
{
	if (!::IsClipboardFormatAvailable(CF_TEXT))
		return;

	if (!OpenClipboard())
		return;

	if (HANDLE hGlb= ::GetClipboardData(CF_TEXT))
	{
		if (VOID* pStr= ::GlobalLock(hGlb))
		{
			m_InputBuffer.Paste(reinterpret_cast<char*>(pStr));
			GlobalUnlock(hGlb);
		}
	}

	CloseClipboard();
}

///////////////////////////////////////////////////////////////////////////////

char CInputBuffer::GetChar()		// get next available character (returns 0 if there are no chars)
{
	char c= 0;

	if (m_pHead != m_pTail)
	{
		c = *m_pTail++;
		if (m_pTail >= m_vchBuffer + BUF_SIZE)
			m_pTail = m_vchBuffer;
	}

	return c;
}

void CInputBuffer::PutChar(char c)	// places char in the buffer (char is ignored if there is no space)
{
	char* pNext= m_pHead + 1;

	if (pNext >= m_vchBuffer + BUF_SIZE)
		pNext = m_vchBuffer;

	if (pNext != m_pTail)	// is there a place in buffer?
	{
		*m_pHead = c;
		m_pHead = pNext;
	}
}


void CInputBuffer::Paste(const char* pcText)
{
	int nMax= min(strlen(pcText), BUF_SIZE);

	for (int i= 0; i < nMax; ++i)
		PutChar(pcText[i]);
}


void CIOWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_INSERT)
		Paste();
	else
		CMiniFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CIOWindow::PreTranslateMessage(MSG* pMsg)
{
	if (GetFocus() == this)
	{
		if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
		{
			if (pMsg->wParam >= VK_SPACE && pMsg->wParam <= 'Z')
			{
				if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) >= 0)
				{
					// skip the rest of PreTranslateMessage() functions, cause they will
					// eat some of those messages (as accel shortcuts); translate and
					// dispatch them now

					::TranslateMessage(pMsg);
					::DispatchMessage(pMsg);
					return true;
				}
			}
		}
	}

	return CMiniFrameWnd::PreTranslateMessage(pMsg);
}


void CIOWindow::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_POPUP_TERMINAL))
		return;
	CMenu *pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	if (point.x == -1 && point.y == -1)		// menu wywo³ane przy pomocy klawiatury?
	{
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		point = rect.CenterPoint();
	}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}


void CIOWindow::OnPaste()
{
	Paste();
}
