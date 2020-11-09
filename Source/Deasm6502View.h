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

// Deasm6502View.h : header file
//

#ifndef _Deasm6502View_
#define _Deasm6502View_

#include "DrawMarks.h"
#include "Asm.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View view

class CDeasm6502View : public CView, public CMarks, CAsm
{
  int m_nFontHeight;
  int m_nFontWidth;

  int no_of_lines(RECT &prect);
  void scroll(UINT nSBCODE, int nPos, int nRepeat= 1);
  void set_scroll_range();

  void get_view_rect(RECT &rect)
  {
    GetClientRect(&rect);
//    if (rect.bottom > m_nFontHeight)
//      rect.bottom -= rect.bottom % m_nFontHeight;	// obszar zajêty przez napisy
  }
  void ScrollToLine(UINT16 addr);

public:
  static COLORREF m_rgbAddress;
  static COLORREF m_rgbCode;
  static COLORREF m_rgbInstr;
  static COLORREF m_rgbBkgnd;
  static bool m_bDrawCode;
  static CFont m_Font;
  static LOGFONT m_LogFont;

protected:
  CDeasm6502View();           // protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CDeasm6502View)
    
    // Attributes
public:
  
  // Operations
public:
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDeasm6502View)
public:
  virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
  virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
  virtual void OnInitialUpdate();
protected:
  virtual void OnDraw(CDC* pDC);      // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
  virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  virtual ~CDeasm6502View();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  
  afx_msg LRESULT OnExitDebugger(WPARAM /* wParam */, LPARAM /* lParam */);
  // Generated message map functions
protected:
  //{{AFX_MSG(CDeasm6502View)
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnDeasmGoto();
  afx_msg void OnUpdateDeasmGoto(CCmdUI* pCmdUI);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
