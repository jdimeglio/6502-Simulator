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

// Deasm6502View.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "Deasm6502View.h"
#include "Deasm6502Doc.h"
#include "resource.h"
#include "DeasmGoto.h"
#include "Deasm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFont CDeasm6502View::m_Font;
LOGFONT CDeasm6502View::m_LogFont;
COLORREF CDeasm6502View::m_rgbBkgnd;
COLORREF CDeasm6502View::m_rgbAddress= RGB(127,127,127);
COLORREF CDeasm6502View::m_rgbCode= RGB(191,191,191);
COLORREF CDeasm6502View::m_rgbInstr= RGB(0,0,0);
bool CDeasm6502View::m_bDrawCode= TRUE;

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View

IMPLEMENT_DYNCREATE(CDeasm6502View, CView)


CDeasm6502View::CDeasm6502View()
{
	m_nFontHeight = 0;
	m_nFontWidth = 0;
}


CDeasm6502View::~CDeasm6502View()
{
}


BEGIN_MESSAGE_MAP(CDeasm6502View, CView)
  ON_WM_CONTEXTMENU()
//{{AFX_MSG_MAP(CDeasm6502View)
  ON_WM_VSCROLL()
  ON_WM_KEYDOWN()
  ON_WM_ERASEBKGND()
  ON_COMMAND(ID_DEASM_GOTO, OnDeasmGoto)
  ON_UPDATE_COMMAND_UI(ID_DEASM_GOTO, OnUpdateDeasmGoto)
  ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
  ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebugger)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View drawing

int CDeasm6502View::no_of_lines(RECT &rect)	// obl. iloœci wierszy w oknie
{
	if (m_nFontHeight == 0)
		return 1;	// not yet ready

	int h= rect.bottom - rect.top;
	if (h >= m_nFontHeight)
		return h / m_nFontHeight;		// no of rows in window
	else
		return 1;						// always at least one row
}


void CDeasm6502View::OnDraw(CDC* pDC)	// deasemblowany program - wyœwietlanie instrukcji
{
  CDeasm6502Doc *pDoc = (CDeasm6502Doc*)GetDocument();
  if (pDoc == NULL)
    return;

  RECT rect;
  GetClientRect(&rect);
  int lines= no_of_lines(rect);

  rect.bottom = rect.top + m_nFontHeight;
  RECT mark= rect;			// miejsce na wskaŸniki

  rect.left += m_nFontHeight;		// margines lewy
  if (rect.left >= rect.right)
    rect.right = rect.left;

  CDeasm deasm;
  int ptr= pDoc->m_uStartAddr;

  for (int i=0; i<=lines; i++)
  {
    if (pDC->RectVisible(&mark))	// pola wskaŸnika do odœwie¿enia?
    {
      Breakpoint bp= theApp.m_global.GetBreakpoint(UINT16(ptr));
      if (bp & BPT_MASK)		// jest miejsce przerwania?
        draw_breakpoint(*pDC,mark.left,mark.top,m_nFontHeight,bp & BPT_DISABLED ? FALSE : TRUE);
      if (pDoc->m_nPointerAddr == ptr)	// w tym wierszu strza³ka?
	draw_pointer(*pDC,mark.left,mark.top,m_nFontHeight);
    }
    const CString &str= 
      deasm.DeasmInstr(*pDoc->m_pCtx,CDeasm::DeasmFmt(CDeasm::DF_ADDRESS|CDeasm::DF_CODE_BYTES),ptr);
    if (pDC->RectVisible(&rect))	// wiersz instrukcji do odœwie¿enia?
    {
      pDC->SetTextColor(m_rgbAddress);
      pDC->TextOut(rect.left,rect.top,LPCTSTR(str),4);
      if (m_bDrawCode)
      {
	pDC->SetTextColor(m_rgbCode);
	pDC->TextOut(rect.left+m_nFontWidth*6,rect.top,LPCTSTR(str)+6,8);
      }
      pDC->SetTextColor(m_rgbInstr);
      pDC->TextOut(rect.left+m_nFontWidth*16,rect.top,LPCTSTR(str)+16,str.GetLength()-16);
    }
    rect.top += m_nFontHeight;
    rect.bottom += m_nFontHeight;
    mark.top += m_nFontHeight;
    mark.bottom += m_nFontHeight;
  }

}

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View diagnostics

#ifdef _DEBUG
void CDeasm6502View::AssertValid() const
{
  CView::AssertValid();
}

void CDeasm6502View::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View message handlers

void CDeasm6502View::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
  if (pInfo || pDC->IsPrinting())
    return;

  pDC->SetBkMode(OPAQUE);
  pDC->SelectObject(&m_Font);
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  m_nFontHeight = (int)tm.tmHeight + (int)tm.tmExternalLeading;
  m_nFontWidth = tm.tmAveCharWidth;
  pDC->SetBkColor(m_rgbBkgnd);

  CView::OnPrepareDC(pDC, pInfo);
}


BOOL CDeasm6502View::PreCreateWindow(CREATESTRUCT& cs)
{
  bool ret= CView::PreCreateWindow(cs);
  cs.style |= WS_VSCROLL;
  return ret;
}


//=============================================================================

void CDeasm6502View::ScrollToLine(UINT16 addr)
{
  CDeasm6502Doc *pDoc= (CDeasm6502Doc *)GetDocument();

  if (pDoc == NULL)
    return;

  if (pDoc->m_uStartAddr == addr)	// ¿¹dany adres jest aktualnym pocz¹tkiem?
    return;

  if (pDoc->m_uStartAddr > addr)	// ¿¹dany adres przed aktualnym pocz¹tkiem
  {
    RECT rect;
    GetClientRect(&rect);
    int lines= no_of_lines(rect);
    CDeasm deasm;
    UINT16 start= addr;
    bool redraw= TRUE;
    for (int i=0; i<lines; i++)	// idziemy od 'addr' w dó³, a¿ spotkamy 'm_uStartAddr'
    {				// lub skoñczy siê iloœæ dostêpnych wierszy
      if (deasm.FindNextAddr(start,*pDoc->m_pCtx) == 0)
        break;		// "przewiniêcie siê" adresu
      if (start > pDoc->m_uStartAddr)
	break;		// rozkazy "nie trafiaj¹" w siebie, przerysowujemy wszystko
      if (start == pDoc->m_uStartAddr)
      {
	RECT rect;
	get_view_rect(rect);
	int y= (i+1) * m_nFontHeight;
/*
	if ( (rect.bottom -= y) <= 0 )
	{
	  ASSERT(FALSE);
	  break;
	}
*/
	UpdateWindow();		// dla unikniêcia problemów z odœwie¿aniem
	pDoc->m_uStartAddr = addr;
        ScrollWindow(0,y,&rect,&rect);
	UpdateWindow();
	redraw = FALSE;
	break;
      }
    }
    if (redraw)
    {
      pDoc->m_uStartAddr = addr;
      InvalidateRect(NULL);
      UpdateWindow();
    }
  }
  else				// ¿¹dany adres jest za aktualnym pocz¹tkiem
  {
    RECT rect;
    GetClientRect(&rect);
    int lines= no_of_lines(rect);
    CDeasm deasm;
    UINT16 start= pDoc->m_uStartAddr;
    bool redraw= TRUE;

	int i;
    for (i= 1; i<lines; i++)	// idziemy od 'm_uStartAddr' w dó³, a¿ spotkamy 'addr'
    {				// lub skoñczy siê iloœæ dostêpnych wierszy
      if (deasm.FindNextAddr(start,*pDoc->m_pCtx) == 0)
        break;		// "przewiniêcie siê" adresu
      if (start > addr)
	break;		// rozkazy "nie trafiaj¹" w siebie, przerysowujemy wszystko
      if (start == addr)
	return;		// strza³ka mieœci siê w oknie
    }
    if (i==lines)	// poprzednia pêtla zakoñczona normalnie (nie przez 'break') ?
    {
      for (int i=0; i<lines; i++)	// idziemy od 'start' w dó³, a¿ spotkamy 'addr'
      {				// lub skoñczy siê iloœæ dostêpnych wierszy
	if (deasm.FindNextAddr(start,*pDoc->m_pCtx) == 0)
	  break;		// "przewiniêcie siê" adresu
	if (start > addr)
	  break;		// rozkazy "nie trafiaj¹" w siebie, przerysowujemy wszystko
	if (start == addr)
	{
	  RECT rect;
	  get_view_rect(rect);
	  int y= (i+1) * m_nFontHeight;
/*
	  if ( (rect.top += y) >= rect.bottom )
	  {
	    ASSERT(FALSE);
	    break;
	  }
*/
	  UpdateWindow();		// dla unikniêcia problemów z odœwie¿aniem
	  for (int j=0; j<=i; j++)	// wyznaczenie nowego adresu pocz¹tku okna
	    deasm.FindNextAddr(pDoc->m_uStartAddr,*pDoc->m_pCtx);
	  ScrollWindow(0,-y,&rect,&rect);
	  UpdateWindow();
	  redraw = FALSE;
	  break;
	}
      }
    }
    if (redraw)
    {
      pDoc->m_uStartAddr = addr;
      InvalidateRect(NULL);
      UpdateWindow();
    }
  }

  return;
}


BOOL CDeasm6502View::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
  if (bDoScroll)
    return TRUE;

  return CView::OnScroll(nScrollCode, nPos, bDoScroll);
}


BOOL CDeasm6502View::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
  return CView::OnScrollBy(sizeScroll, bDoScroll);
}


void CDeasm6502View::OnInitialUpdate()
{
  CView::OnInitialUpdate();

//  SetScrollRange(SB_VERT,-0x8000,0x7FFF);
  set_scroll_range();

  CDeasm6502Doc *pDoc = (CDeasm6502Doc*)GetDocument();
  if (pDoc)
    SetScrollPos(SB_VERT,(int)pDoc->m_uStartAddr-0x8000);

  m_Font.DeleteObject();
  m_Font.CreateFontIndirect(&m_LogFont);
}


//-----------------------------------------------------------------------------
// przesuniêcie zawartoœci okna deasemblacji
//
void CDeasm6502View::scroll(UINT nSBCode, int nPos, int nRepeat)
{
  CDeasm6502Doc *pDoc = (CDeasm6502Doc*)GetDocument();
  if (pDoc == NULL)
    return;
//  UINT8 cmd;
  CDeasm deasm;

  switch (nSBCode)
  {
    case SB_ENDSCROLL:	// End scroll
      break;

    case SB_LINEDOWN:	// Scroll one line down
      switch (deasm.FindNextAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx))
      {
        case 0:
	  break;	// dalej ju¿ siê nie da
	case 1:
	  RECT rect;
          GetClientRect(&rect);
//	  get_view_rect(rect);
//	  UpdateWindow();	// dla unikniêcia problemów z odœwie¿aniem
	  ScrollWindow(0, -m_nFontHeight, &rect, &rect);
	  UpdateWindow();
	  break;
      }
/*
      cmd = pDoc->m_pCtx->mem[pDoc->m_uStartAddr];	// pierwszy rozkaz w oknie
      pDoc->m_uStartAddr = (pDoc->m_uStartAddr + mode_to_len[code_to_mode[cmd]]) & pDoc->m_pCtx->mem_mask;
      RECT rect;
      get_view_rect(rect);
      UpdateWindow();	// dla unikniêcia problemów z odœwie¿aniem
      ScrollWindow(0,-m_nFontHeight,&rect,&rect);
*/
      break;
    case SB_LINEUP:	// Scroll one line up
      switch (deasm.FindPrevAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx))
      {
        case 0:
	  break;	// jesteœmy ju¿ na pocz¹tku
	case 1:
	  RECT rect;
          GetClientRect(&rect);
//	  get_view_rect(rect);
//	  if ( (rect.bottom -= m_nFontHeight) <= 0)
//	    break;
//	  UpdateWindow();	// dla unikniêcia problemów z odœwie¿aniem
          ScrollWindow(0,m_nFontHeight, &rect, &rect);
	  UpdateWindow();
	  break;
	case -1:
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna - zmieni³o siê kilka rozkazów
	  break;
	default:
	  ASSERT(FALSE);
	  break;
      }
      break;

    case SB_PAGEDOWN:	// Scroll one page down
    {
      RECT rect;
      get_view_rect(rect);
      switch (deasm.FindNextAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx, no_of_lines(rect)))
      {
        case 0:
	  break;	// dalej ju¿ siê nie da
	case 1:
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
	  break;
      }
      break;
    }

    case SB_PAGEUP:	// Scroll one page up
    {
      RECT rect;
      get_view_rect(rect);
      switch (deasm.FindPrevAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx, no_of_lines(rect)))
      {
        case 0:
	  break;	// jesteœmy ju¿ na pocz¹tku
	case 1:
	case -1:
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna - zmieni³o siê kilka rozkazów
	  break;
	default:
	  ASSERT(FALSE);
	  break;
      }
      break;
    }

    case SB_TOP:	// Scroll to top
    {
      RECT rect;
      get_view_rect(rect);
      int dy= no_of_lines(rect);	// iloœæ wierszy w oknie
      int lines= deasm.FindDelta(pDoc->m_uStartAddr, 0, *pDoc->m_pCtx, dy);
      if (lines < 0)
	InvalidateRect(NULL);	// przerysowanie ca³ego okna
      else if (lines > 0)
      {
	if (lines >= dy)
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
	else
//	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
          ScrollWindow(0,lines * m_nFontHeight,&rect,&rect);
      }
      break;
    }

    case SB_BOTTOM:	// Scroll to bottom
    {
      RECT rect;
      get_view_rect(rect);
      int dy= no_of_lines(rect);	// iloœæ wierszy w oknie
      int lines= deasm.FindDelta(pDoc->m_uStartAddr, 0xFFFF, *pDoc->m_pCtx, dy);
      if (lines < 0)
	InvalidateRect(NULL);	// przerysowanie ca³ego okna
      else if (lines > 0)
      {
	if (lines >= dy)
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
	else
//	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
          ScrollWindow(0, -lines * m_nFontHeight, &rect, &rect);
      }
      break;
    }

    case SB_THUMBPOSITION:   // Scroll to the absolute position. The current position is provided in nPos
      break;
    case SB_THUMBTRACK:	// Drag scroll box to specified position. The current position is provided in nPos
    {
      RECT rect;
      get_view_rect(rect);
      int dy= no_of_lines(rect);	// iloœæ wierszy w oknie
      int lines= deasm.FindDelta(pDoc->m_uStartAddr,UINT16(nPos+0x8000),*pDoc->m_pCtx,dy);
      if (lines < 0)
	InvalidateRect(NULL);	// przerysowanie ca³ego okna
      else if (lines > 0)
      {
	if (lines >= dy)
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
	else
	  InvalidateRect(NULL);	// przerysowanie ca³ego okna
//          ScrollWindow(0,dir * lines * m_nFontHeight,&rect,&rect);
      }
      break;
    }
    default:
      break;
  }
  SetScrollPos(SB_VERT,(int)pDoc->m_uStartAddr - 0x8000);
}


void CDeasm6502View::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  scroll(nSBCode, nPos);

// int SetScrollPos( int nBar, int nPos);

//  CView::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CDeasm6502View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  switch (nChar)
  {
    case VK_DOWN:
      scroll(SB_LINEDOWN,0,nRepCnt);
      break;
    case VK_UP:
      scroll(SB_LINEUP,0,nRepCnt);
      break;
    case VK_NEXT:
      scroll(SB_PAGEDOWN,0,nRepCnt);
      break;
    case VK_PRIOR:
      scroll(SB_PAGEUP,0,nRepCnt);
      break;
    case VK_HOME:
      scroll(SB_TOP,0,nRepCnt);
      break;
    case VK_END:
      scroll(SB_BOTTOM,0,nRepCnt);
      break;
    default:
      CView::OnKeyDown(nChar, nRepCnt, nFlags);
  }
}

//=============================================================================

void CDeasm6502View::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  switch (LOWORD(lHint))
  {
    case 1:	// przerysowaæ wskaŸniki?
    {
      RECT rect;
      GetClientRect(&rect);
      rect.right = m_nFontHeight;		// wielkoœæ lewego marginesu
      InvalidateRect(&rect);
      UpdateWindow();
      break;
    }
    case 2:	// przesun¹æ zawartoœæ okna?
      ScrollToLine(UINT16(HIWORD(lHint)));
      break;
    case 0:	// przerysowaæ zawartoœæ okna?
      InvalidateRect(NULL);
      break;
  }
}


//=============================================================================

afx_msg LRESULT CDeasm6502View::OnExitDebugger(WPARAM /* wParam */, LPARAM /* lParam */)
{
  GetDocument()->OnCloseDocument();
  return 0;
}

//-----------------------------------------------------------------------------

BOOL CDeasm6502View::OnEraseBkgnd(CDC* pDC)
{
  RECT rect;
  GetClientRect(&rect);
  pDC->FillSolidRect(&rect,m_rgbBkgnd);
  return TRUE;
// return CView::OnEraseBkgnd(pDC);
}

//-----------------------------------------------------------------------------

void CDeasm6502View::OnDeasmGoto()
{
  static UINT addr= 0;
  CDeasmGoto dlg;
  dlg.m_uAddress = addr;

  if (dlg.DoModal() == IDOK)
  {
    addr = dlg.m_uAddress;
    scroll(SB_THUMBTRACK,dlg.m_uAddress-0x8000,1);
  }
}

void CDeasm6502View::OnUpdateDeasmGoto(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
}

//-----------------------------------------------------------------------------
// Popup menu

void CDeasm6502View::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  CMenu menu;
  VERIFY(menu.LoadMenu(IDR_POPUP_DEASM6502));

  CMenu* pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);
/*
  CWnd* pWndPopupOwner = this;
  while (pWndPopupOwner->GetStyle() & WS_CHILD)
    pWndPopupOwner = pWndPopupOwner->GetParent();
*/
  if (point.x == -1 && point.y == -1)     // menu wywo³ane przy pomocy klawiatury?
  {
    CRect rect;
    GetClientRect(rect);

    point = rect.TopLeft();
    CPoint ptTopLeft(0, 0);
    ClientToScreen(&ptTopLeft);
    point.x = ptTopLeft.x + rect.Width() / 2;   // ustawiamy siê na œrodku okna
    point.y = ptTopLeft.y + rect.Height() / 2;
  }

  pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
//    pWndPopupOwner);
}


void CDeasm6502View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	set_scroll_range();
}


void CDeasm6502View::set_scroll_range()
{
  RECT rect;
  GetClientRect(&rect);
  int lines= no_of_lines(rect);

  SCROLLINFO si;
  si.cbSize = sizeof si;
  si.fMask = SIF_RANGE | SIF_PAGE;
  si.nMin = -0x8000;
  si.nMax = 0x7fff;
  si.nPage = lines; // estimate only
  SetScrollInfo(SB_VERT, &si, FALSE);
}
