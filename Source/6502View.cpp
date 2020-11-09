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

// 6502View.cpp : implementation of the CSrc6502View class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "6502Doc.h"
#include "6502View.h"
#include "DrawMarks.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CSrc6502View::m_bAutoIndent= TRUE;		// sk³adowa statyczna - automatyczne wciêcia
int CSrc6502View::m_nTabStep= 8;		// krok tabulacji
bool CSrc6502View::m_bAutoSyntax = TRUE;
bool CSrc6502View::m_bAutoUppercase = TRUE;
CFont CSrc6502View::m_Font;
LOGFONT CSrc6502View::m_LogFont;
COLORREF CSrc6502View::m_rgbTextColor;
COLORREF CSrc6502View::m_rgbBkgndColor;
COLORREF CSrc6502View::m_vrgbColorSyntax[6]=
{
	RGB(0, 0, 160),		// instructions
	RGB(128, 0, 128),	// directives
	RGB(128, 128, 128),	// comments
	RGB(0, 0, 255),		// number
	RGB(0, 128, 128),	// string
	RGB(192, 192, 224)	// selection
};
BYTE CSrc6502View::m_vbyFontStyle[6]=
{
	0, 0, 0, 0, 0, 0
};

static CMarks s_LeftMarginMarker;

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View

#ifdef USE_CRYSTAL_EDIT
	IMPLEMENT_DYNCREATE(CSrc6502View, CCrystalEditView)
#else
	IMPLEMENT_DYNCREATE(CSrc6502View, CEditView)
#endif

BEGIN_MESSAGE_MAP(CSrc6502View, CBaseView)
  //{{AFX_MSG_MAP(CSrc6502View)
  ON_WM_CREATE()
  ON_CONTROL_REFLECT(EN_UPDATE, OnEnUpdate)
  ON_WM_CONTEXTMENU()
  ON_WM_CTLCOLOR_REFLECT()
  //}}AFX_MSG_MAP
  // Standard printing commands
  ON_COMMAND(ID_FILE_PRINT, CBaseView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, CBaseView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CBaseView::OnFilePrintPreview)
  ON_MESSAGE(CBroadcast::WM_USER_REMOVE_ERR_MARK, OnRemoveErrMark)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View construction/destruction

CSrc6502View::CSrc6502View()
{
  // TODO: add construction code here
  m_nActualPointerLine = -1;
  m_nActualErrMarkLine = -1;
  m_pMainFrame = 0;
//  m_nBrkIndex = 0;
/*
  memset(&m_logfont, 0, sizeof(m_logfont));
  m_logfont.lfHeight = 9;
  m_logfont.lfPitchAndFamily = FIXED_PITCH;
  strcpy(m_logfont.lfFaceName, "Fixedsys");
*/
//  m_Font.CreateFontIndirect(&m_LogFont);
}

CSrc6502View::~CSrc6502View()
{
}

BOOL CSrc6502View::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  
  bool bPreCreated = CBaseView::PreCreateWindow(cs);
//  cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

  return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View drawing

void CSrc6502View::OnDraw(CDC* pDC)  // overridden to draw this view
{
#ifdef USE_CRYSTAL_EDIT
	CBaseView::OnDraw(pDC);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View printing

BOOL CSrc6502View::OnPreparePrinting(CPrintInfo* pInfo)
{
  // default CBaseView preparation
  return CBaseView::OnPreparePrinting(pInfo);
}

void CSrc6502View::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
  // Default CBaseView begin printing.
  CBaseView::OnBeginPrinting(pDC, pInfo);
}

void CSrc6502View::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
  // Default CBaseView end printing
  CBaseView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View diagnostics

#ifdef _DEBUG
void CSrc6502View::AssertValid() const
{
  CBaseView::AssertValid();
}

void CSrc6502View::Dump(CDumpContext& dc) const
{
  CBaseView::Dump(dc);
}

CSrc6502Doc* CSrc6502View::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSrc6502Doc)));
  return (CSrc6502Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View message handlers

void CSrc6502View::OnInitialUpdate() 
{
  CBaseView::OnInitialUpdate();

  SelectEditFont();
/*
  SetFont(&m_Font,FALSE);
  CEdit &edit= GetEditCtrl();
  DWORD margins= edit.GetMargins();
  edit.SetMargins(16u,UINT(HIWORD(margins)));  // ustawienie lewego marginesu
*/
	SetAutoIndent(CSrc6502View::m_bAutoIndent);
}


int CSrc6502View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CBaseView::OnCreate(lpCreateStruct) == -1)
    return -1;

#ifndef USE_CRYSTAL_EDIT
  m_pfnOldProc = (LRESULT (CALLBACK *)(HWND,UINT,WPARAM,LPARAM)) ::SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)EditWndProc);

  m_wndLeftBar.Create(CWnd::FromHandlePermanent(lpCreateStruct->hwndParent), this);
#endif
  VERIFY(m_pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd()));

  return 0;
}


LRESULT (CALLBACK *CSrc6502View::m_pfnOldProc)(HWND,UINT,WPARAM,LPARAM) = NULL;


void CSrc6502View::check_line(const TCHAR* buf, CAsm::Stat &stat, int &start, int &fin, CString &msg)
{
  CAsm6502 xasm;
  xasm.bProc6502 = theApp.m_global.GetProcType();
  stat= xasm.CheckLine(buf,start,fin);
  if (stat)
    msg = xasm.GetErrMsg(stat);
  else
    msg.Empty();
}

void CSrc6502View::disp_warning(int line, CString &msg)
{
  SetErrMark(line);	// zaznaczenie wiersza zawieraj¹cego b³¹d
  CMainFrame *pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;
  pMain->m_wndStatusBar.SetPaneText(0,msg);
}

//-----------------------------------------------------------------------------

void CSrc6502View::set_position_info(HWND hWnd)
{
  static CString strLine;
  int nStart;
  (*m_pfnOldProc)(hWnd,EM_GETSEL,WPARAM(&nStart),LPARAM(NULL));
  int nLine= (*m_pfnOldProc)(hWnd,EM_LINEFROMCHAR,WPARAM(nStart),LPARAM(0));
  nStart -= (*m_pfnOldProc)(hWnd,EM_LINEINDEX,WPARAM(nLine),LPARAM(0));
  if (nLine < 0 || nStart < 0)
    return;
  TCHAR *pBuf= strLine.GetBuffer(1024+2);
  *((WORD *)pBuf) = 1024;
  int nChars = (*m_pfnOldProc)(hWnd,EM_GETLINE,WPARAM(nLine),LPARAM(pBuf));
  pBuf[nChars] = 0;
  int nColumn= 0;
  for (int i=0; i<nChars && i<nStart; i++)	// obl. numeru kolumny
  {
    if (pBuf[i] == _T('\t'))	// tabulator?
      nColumn += m_nTabStep - nColumn % m_nTabStep;
    else
      nColumn++;
  }
  strLine.ReleaseBuffer(0);

  CMainFrame* pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;
  pMain->SetPositionText(nLine+1,nColumn+1);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK CSrc6502View::EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  CWnd *pWnd= FromHandlePermanent(hWnd);
  ASSERT (pWnd->IsKindOf(RUNTIME_CLASS(CSrc6502View)));
  CSrc6502View *pView= (CSrc6502View *)pWnd;
  bool cr= false;

  switch (msg)
  {
    case WM_CHAR:
      cr = (wParam == 0xD);    // CR?
    case WM_KEYUP:
    case WM_PASTE:
    case WM_COPY:
    case WM_CUT:
    case WM_UNDO:
    {
      LRESULT ret;
      if (cr && m_bAutoIndent)
      {
        ret = (*m_pfnOldProc)(hWnd,msg,wParam,lParam);
	pView->set_position_info(hWnd);
	int line_idx= (*m_pfnOldProc)(hWnd,EM_LINEINDEX,WPARAM(-1),0);
	int line= (*m_pfnOldProc)(hWnd,EM_LINEFROMCHAR,line_idx,0) - 1;	// nr aktualnego wiersza - 1
	line_idx= (*m_pfnOldProc)(hWnd,EM_LINEINDEX,WPARAM(line),0);
	int line_len= (*m_pfnOldProc)(hWnd,EM_LINELENGTH,line_idx,0);
	line= (*m_pfnOldProc)(hWnd,EM_LINEFROMCHAR,line_idx,0);
	TCHAR buf[260];
	const int size= sizeof(buf) / sizeof(TCHAR) - 2;
	*(WORD *)(buf+2) = (WORD)size;
	(*m_pfnOldProc)(hWnd,EM_GETLINE,line,(LPARAM)(buf+2));
	buf[2+min(size-1,line_len)] = 0;
/*
	int line_idx= (*m_pfnOldProc)(hWnd,EM_LINEINDEX,WPARAM(-1),0);
	int line_len= (*m_pfnOldProc)(hWnd,EM_LINELENGTH,line_idx,0);
	int line= (*m_pfnOldProc)(hWnd,EM_LINEFROMCHAR,line_idx,0);	// nr aktualnego wiersza
	TCHAR buf[260];
	const int size= sizeof(buf) / sizeof(TCHAR) - 2;
	*(WORD *)(buf+2) = (WORD)size;
	(*m_pfnOldProc)(hWnd,EM_GETLINE,line,(LPARAM)(buf+2));
	buf[2+min(size-1,line_len)] = 0;
*/
	int start,fin;
	CAsm::Stat stat= CAsm::OK;
	CString strmsg;
	if (m_bAutoSyntax || m_bAutoUppercase)
	  check_line(buf+2,stat,start,fin,strmsg);
	if (m_bAutoUppercase && start>0 && fin>0)	// jest instrukcja do zamiany na du¿e litery?
	{
	  TCHAR instr[32];
	  ASSERT(fin-start < 32);
	  _tcsncpy(instr,buf+2+start,fin-start);
	  instr[fin-start] = 0;
	  _tcsupr(instr);
	  int c_start,c_end;
	  (*m_pfnOldProc)(hWnd,EM_GETSEL,WPARAM(&c_start),LPARAM(&c_end));
	  (*m_pfnOldProc)(hWnd,EM_SETSEL,line_idx+start,line_idx+fin);
	  (*m_pfnOldProc)(hWnd,EM_REPLACESEL,0,(LPARAM)instr);
	  (*m_pfnOldProc)(hWnd,EM_SETSEL,c_start,c_start);
//	  (*m_pfnOldProc)(hWnd,EM_SETSEL,line_idx+line_len,line_idx+line_len);
	}
        int len= _tcsspn(buf+2,_T(" \t"));	// iloœæ spacji i tabulatorów na pocz¹tku wiersza
	if (!(m_bAutoSyntax && stat))		// jeœli nie ma b³êdu (jeœli spr. b³êdów), to  wciêcie
	{
	  if (len)		// jeœli jest wciêcie w wierszu powy¿ej, to kopiujemy je
	  {
	    buf[len+2] = 0;
	    buf[0] = 0xD;
	    buf[1] = 0xA;
	    ret = (*m_pfnOldProc)(hWnd,EM_REPLACESEL,TRUE,(LPARAM)(buf+2));	  // wciêcie tekstu
	  }
	  else
	    ;
//	    ret = (*m_pfnOldProc)(hWnd,msg,wParam,lParam);	  // CR i rozsuniêcie wierszy
	}
	if (m_bAutoSyntax && stat)
	  pView->disp_warning(line,strmsg);
      }
      else
      {
        ret = (*m_pfnOldProc)(hWnd,msg,wParam,lParam);
	pView->set_position_info(hWnd);
      }

      pView->RedrawMarks();
/*
      pView->draw_breakpoints();
      if (pView->m_nActualPointerLine != -1)
        pView->DrawMark(pView->m_nActualPointerLine,MT_POINTER);
      if (pView->m_nActualErrMarkLine != -1)
        pView->DrawMark(pView->m_nActualErrMarkLine,MT_ERROR);
*/
      return ret;
    }

    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    {
      LRESULT ret= (*m_pfnOldProc)(hWnd,msg,wParam,lParam);
      pView->set_position_info(hWnd);
      return ret;
    }

    case WM_PAINT:
    {
      LRESULT ret= (*m_pfnOldProc)(hWnd,msg,wParam,lParam);
      if (ret == 0)
        pView->RedrawMarks();
/*      {
    pView->draw_breakpoints((HDC)wParam);
    if (pView->m_nActualPointerLine != -1)
          pView->DrawMark(pView->m_nActualPointerLine,MT_POINTER,FALSE,(HDC)wParam);
    if (pView->m_nActualErrMarkLine != -1)
          pView->DrawMark(pView->m_nActualErrMarkLine,MT_ERROR,FALSE,(HDC)wParam);
      } */
      return ret;
    }

    default:
      return (*m_pfnOldProc)(hWnd,msg,wParam,lParam);
  }

}


//=============================================================================
/*
void CSrc6502View::drawMark(CDC &dc, int line, MarkType type, bool scroll)
{
  int h, y= ScrollToLine(line,h,scroll);
  if (y < 0)
    return;
  y++;

  switch (type)
  {
    case MT_ERASE:	// zmazanie znacznika
    {
      dc.GetWindow()->RedrawWindow(CRect(1, y, 1 + h, y + h));
      break;
    }
    case MT_POINTER:	// narysowanie strza³ki wskazuj¹cej inst. do wykonania
      draw_pointer(dc,1,y,h);
      break;
    case MT_BREAKPOINT:	// narysowanie aktywnego miejsca przerwania
      draw_breakpoint(dc,1,y,h,TRUE);
      break;
    case MT_DISBRKP:	// narysowanie wy³¹czonego miejsca przerwania
      draw_breakpoint(dc,1,y,h,FALSE);
      break;
    case MT_ERROR:	// narysowanie strza³ki wskazuj¹cej b³¹d
      draw_mark(dc,4,y,h);
      break;
    default:
      ASSERT(FALSE);	// b³êdna wartoœæ typu
      break;
  }
}
*/

int CSrc6502View::ScrollToLine(int line, int &height, bool scroll)
{
	ASSERT(line >= 0);
#ifdef USE_CRYSTAL_EDIT
	GoToLine(line);
	return 0;
#else
  CEdit &edit= GetEditCtrl();
  if (line > edit.GetLineCount()-1)
  {
    ASSERT(FALSE);	// za du¿y numer wiersza
    return -1;
  }
  int top_line= edit.GetFirstVisibleLine();
  if (line < top_line)
  {
    if (!scroll)
      return -2;
    edit.LineScroll(line-top_line);	// wiersz nie jest widoczny - przesuniêcie zawartoœci okna
    top_line = edit.GetFirstVisibleLine();
    int top_char= edit.LineIndex(top_line);
    edit.SendMessage(EM_SETSEL,top_char,top_char);	// przesuniêcie karetki do wiersza 'top_line'
  }
  CClientDC dc(&edit);
  CFont* pOld= dc.SelectObject(&m_Font);
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);
  dc.SelectObject(pOld);
  int h= (int)tm.tmHeight + (int)tm.tmExternalLeading;
  int y= (line - top_line) * h;		// po³o¿enie wiersza w pionie
  CRect rect;
  edit.GetClientRect(rect);
  if (y+h-1 >= rect.bottom)
  {
    if (!scroll)
      return -2;
    edit.LineScroll(1 + (y+h-1 - rect.bottom) / h);	// iloœæ wierszy do przesuniêcia
//    edit.SendMessage(EM_SCROLLCARET,0,0);
    top_line = edit.GetFirstVisibleLine();
    int bottom_char= edit.LineIndex(line);
    edit.SendMessage(EM_SETSEL,bottom_char,bottom_char);// przesuniêcie karetki do wiersza 'line'
  }
  y = (line - top_line) * h;		// po³o¿enie wiersza w pionie
  if (y+h-1 >= rect.bottom)
  {
    ASSERT(FALSE);	// b³êdne przesuniêcia albo obliczenia w tej funkcji
    return -1;
  }

  height = h;
  return y;
#endif
}


// edit view info
//
void CSrc6502View::GetDispInfo(int& nTopLine, int& nLineCount, int& nLineHeight)
{
#ifdef USE_CRYSTAL_EDIT
	nLineHeight = GetLineHeight();
	return;
#else
  CEdit& edit= GetEditCtrl();
  nLineCount = edit.GetLineCount();
  nTopLine = edit.GetFirstVisibleLine();

  CClientDC dc(&edit);
  CFont* pOld= dc.SelectObject(&m_Font);
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);
  nLineHeight = (int)tm.tmHeight + (int)tm.tmExternalLeading;
  dc.SelectObject(pOld);
#endif
}

/*
void CSrc6502View::DrawMark(int line, MarkType type, bool scroll, HDC hDC)
{
  if (m_wndLeftBar.m_hWnd == 0)
    return;

  CClientDC dc(&m_wndLeftBar);

  drawMark(dc, line, type, scroll);

#if 0
  if (hDC == NULL)
  {
    CClientDC dc(&GetEditCtrl());
    drawMark(dc,line,type,scroll);
  }
  else
    drawMark(*CDC::FromHandle(hDC),line,type,scroll);
#endif
} */

/*
void CSrc6502View::draw_breakpoints(HDC hDC)
{
  if (m_mapBreakpoints.IsEmpty())
    return;
  POSITION pos= m_mapBreakpoints.GetStartPosition();
  int line;
  BYTE bp;
  do
  {
    m_mapBreakpoints.GetNextAssoc(pos,line,bp);
    DrawMark(line,bp & CAsm::BPT_DISABLED ? MT_DISBRKP : MT_BREAKPOINT,FALSE,hDC);
  } while (pos);
}
*/

//-----------------------------------------------------------------------------


void CSrc6502View::SetPointer(int line, bool scroll)	// narysowanie/zmazanie strza³ki w wierszu
{
  if (m_nActualPointerLine != -1)
  {
    int tmp_line= m_nActualPointerLine;
    m_nActualPointerLine = -1;
    EraseMark(tmp_line);	// zmazanie starej strza³ki
  }
  m_nActualPointerLine = line;
  if (line != -1)
  {
    int h;
    ScrollToLine(line,h,TRUE);
    RedrawMarks(line);
  }
}


void CSrc6502View::SetErrMark(int line)		// narysowanie/zmazanie strza³ki wsk. b³¹d
{
  if (m_nActualErrMarkLine != -1)
  {
    int tmp= m_nActualErrMarkLine;
    m_nActualErrMarkLine = -1;
    EraseMark(tmp);			// zmazanie starej strza³ki
  }
  m_nActualErrMarkLine = line;
  if (line != -1)
  {
    int h;
    ScrollToLine(line,h,TRUE);
    RedrawMarks(line);
#ifdef USE_CRYSTAL_EDIT
	GoToLine(line);
#else
    CEdit &edit= GetEditCtrl();
    int char_index= edit.LineIndex(line);
    edit.SendMessage(EM_SETSEL,char_index,char_index);	// przesuniêcie karetki do wiersza 'line'
#endif
  }
}


void CSrc6502View::OnEnUpdate()		// po zmianie tekstu
{
  if (m_nActualErrMarkLine != -1)
  {
    SetErrMark(-1);			// zmazujemy wsk. b³êdnego wiersza
    CMainFrame *pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;
    pMain->m_wndStatusBar.SetPaneText(0,NULL);	// i komunikat b³êdu
  }
}


void CSrc6502View::SelectEditFont()
{
#ifdef USE_CRYSTAL_EDIT
	SetFont(m_LogFont);
//	m_wndLeftBar.SetWidth(0);
	SetTabSize(m_nTabStep);
#else
  SetFont(&m_Font);
  CEdit &edit= GetEditCtrl();
  CClientDC dc(&edit);
  dc.SelectObject(&m_Font);
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);
  int h= (int)tm.tmHeight + (int)tm.tmExternalLeading;
//  DWORD margins= edit.GetMargins();
//  edit.SetMargins(h+1,UINT(HIWORD(margins)));	// ustawienie lewego marginesu
  m_wndLeftBar.SetWidth(h + 1);
  dynamic_cast<CFrameWnd*>(GetParent())->RecalcLayout();
#endif
}


int CSrc6502View::GetCurrLineNo()	// aktualny wiersz
{
#ifdef USE_CRYSTAL_EDIT
	return GetCursorPos().y;
#else
  CEdit &edit= GetEditCtrl();
  int idx= edit.LineIndex();
  ASSERT(idx != -1);
  return edit.LineFromChar(idx);
#endif
}


void CSrc6502View::AddBreakpoint(int line, CAsm::Breakpoint bp, bool draw)
{
  m_mapBreakpoints[line] = (BYTE)bp;
  if (draw)
    RedrawMarks(line);
}


void CSrc6502View::RemoveBreakpoint(int line, bool draw)
{
  m_mapBreakpoints.RemoveKey(line);
  if (draw)
    RedrawMarks(line);
}


void CSrc6502View::RedrawMarks(int line/*= -1*/)
{
#ifdef USE_CRYSTAL_EDIT
	if (line >= 0)
		InvalidateLines(line, line, true);
#else
  if (line == -1)	// przerysowaæ znaczniki we wszystkich wierszach?
  {
    m_wndLeftBar.RedrawWindow();
/*    draw_breakpoints();
    if (m_nActualPointerLine != -1)
      DrawMark(m_nActualPointerLine,MT_POINTER);
    if (m_nActualErrMarkLine != -1)
      DrawMark(m_nActualErrMarkLine,MT_ERROR); */
  }
  else  // redraw only given line
  {
    ASSERT(line >= 0);
    m_wndLeftBar.RedrawLine(line);
/*
    BYTE bp;
    if (m_mapBreakpoints.Lookup(line, bp))
      DrawMark(line, bp & CAsm::BPT_DISABLED ? MT_DISBRKP : MT_BREAKPOINT);// znaczek miejsca przerwania
    else
      DrawMark(line, MT_ERASE);					// zmazanie znaku przerwania
    if (m_nActualPointerLine == line)
      DrawMark(m_nActualPointerLine, MT_POINTER);
    if (m_nActualErrMarkLine == line)
      DrawMark(m_nActualErrMarkLine, MT_ERROR); */
  }
#endif
}


void CSrc6502View::EraseMark(int line)
{
//  ASSERT(line >= 0);
//  DrawMark(line, MT_ERASE);
  RedrawMarks(line);
}



void CSrc6502View::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  CMenu menu;
  if (!menu.LoadMenu(IDR_POPUP_EDIT))
    return;
  CMenu *pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);

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
}


afx_msg LRESULT CSrc6502View::OnRemoveErrMark(WPARAM wParam, LPARAM lParam)
{
  SetErrMark(-1);			// zmazujemy wsk. b³êdnego wiersza
  return 1;
}


HBRUSH CSrc6502View::CtlColor(CDC* pDC, UINT nCtlColor) 
{
/*  pDC->SetTextColor(m_rgbTextColor);
  pDC->SetBkColor(m_rgbBkgndColor); */
  //this is wrong: return (HBRUSH)CreateSolidBrush(m_rgbBkgndColor);

  // TODO: Change any attributes of the DC here
  
  // TODO: Return a non-NULL brush if the parent's handler should not be called
  return NULL;
}

void CSrc6502View::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
/*
  if (nAdjustType != 0)
  {
    // default behavior for in-place editing handles scrollbars
    DWORD dwStyle = GetStyle();
    if (dwStyle & WS_VSCROLL)
      lpClientRect->right += afxData.cxVScroll - CX_BORDER;
    if (dwStyle & WS_HSCROLL)
      lpClientRect->bottom += afxData.cyHScroll - CY_BORDER;
    return;
  }
*/
  ::AdjustWindowRectEx(lpClientRect, GetStyle() /*| WS_BORDER*/, FALSE,
    GetExStyle() & ~(WS_EX_CLIENTEDGE));
	
//	CBaseView::CalcWindowRect(lpClientRect, nAdjustType);
}


// return breakpoint info for line 'nLine'
//
BYTE CSrc6502View::GetBreakpoint(int nLine) const
{
  BYTE bp= 0;
  m_mapBreakpoints.Lookup(nLine, bp);
  return bp;
}


void CSrc6502View::GetText(CString& strText)
{
#ifdef USE_CRYSTAL_EDIT
	GetDocument()->GetText(strText);
#else
	GetEditCtrl().GetWindowText(strText);
#endif
}


#ifdef USE_CRYSTAL_EDIT
CCrystalTextBuffer* CSrc6502View::LocateTextBuffer()
{
	return GetDocument()->GetBuffer();
}


void CSrc6502View::DrawMarginMarker(int nLine, CDC* pDC, const CRect &rect)
{
	int nLeft= rect.left + rect.Width() / 6;

	if (BYTE bp= GetBreakpoint(nLine))
		s_LeftMarginMarker.draw_breakpoint(*pDC, nLeft, rect.top, rect.Height(), !(bp & CAsm::BPT_DISABLED));

	if (nLine == GetPointerLine())
		s_LeftMarginMarker.draw_pointer(*pDC, nLeft, rect.top, rect.Height());

	if (nLine == GetErrorMarkLine())
		s_LeftMarginMarker.draw_mark(*pDC, nLeft, rect.top, rect.Height());
}


CCrystalEditView::LineChange CSrc6502View::NotifyEnterPressed(CPoint ptCursor, CString& strLine)
{
	LineChange eChange= CCrystalEditView::NOTIF_NO_CHANGES;

	if (m_bAutoSyntax || m_bAutoUppercase)
	{
		int start= 0, fin= 0;
		CAsm::Stat stat= CAsm::OK;
		CString strMsg;

		check_line(strLine, stat, start, fin, strMsg);

		if (m_bAutoUppercase && start > 0 && fin > 0)	// jest instrukcja do zamiany na du¿e litery?
		{
			for (int nIndex= start; nIndex < fin; ++nIndex)
			{
				TCHAR c= strLine[nIndex];
				TCHAR u= toupper(c);
				if (c != u)
				{
					strLine.SetAt(nIndex, u);
					eChange = NOTIF_LINE_MODIFIED;
				}
			}
		}

		if (m_bAutoSyntax && stat != CAsm::OK)
		{
			disp_warning(ptCursor.y, strMsg);
			eChange = NOTIF_LINE_ERROR;
		}
	}

	return eChange;
}


void CSrc6502View::NotifyTextChanged()
{
	OnEnUpdate();
}


COLORREF CSrc6502View::GetColor(int nColorIndex)
{
	switch (nColorIndex)
	{
	case COLORINDEX_WHITESPACE:
	case COLORINDEX_BKGND:
		return m_rgbBkgndColor;

	case COLORINDEX_NORMALTEXT:
		return m_rgbTextColor;

	case COLORINDEX_KEYWORD:		// instructions
		return m_vrgbColorSyntax[0];
	case COLORINDEX_PREPROCESSOR:	// directives
		return m_vrgbColorSyntax[1];
	case COLORINDEX_COMMENT:
		return m_vrgbColorSyntax[2];
	case COLORINDEX_NUMBER:
		return m_vrgbColorSyntax[3];
	case COLORINDEX_STRING:
		return m_vrgbColorSyntax[4];
	case COLORINDEX_OPERATOR:
		return RGB(128, 0, 0);
	case COLORINDEX_SELBKGND:
		return m_vrgbColorSyntax[5];

	default:
		return CBaseView::GetColor(nColorIndex);
	}
}


BOOL CSrc6502View::GetBold(int nColorIndex)
{
	switch (nColorIndex)
	{
	case COLORINDEX_KEYWORD:		// instructions
		return m_vbyFontStyle[0] & 1;
	case COLORINDEX_PREPROCESSOR:	// directives
		return m_vbyFontStyle[1] & 1;
	case COLORINDEX_COMMENT:
		return m_vbyFontStyle[2] & 1;
	case COLORINDEX_NUMBER:
		return m_vbyFontStyle[3] & 1;
	case COLORINDEX_STRING:
		return m_vbyFontStyle[4] & 1;

	default:
		return CBaseView::GetBold(nColorIndex);
	}
}


void CSrc6502View::CaretMoved(const CString& strLine, int nWordStart, int nWordEnd)
{
	if (m_pMainFrame)
		m_pMainFrame->ShowDynamicHelp(strLine, nWordStart, nWordEnd);

	//	TRACE("%s\n", strWord.IsEmpty() ? "=":(const char*)strWord);
}


#endif
