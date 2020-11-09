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

// MemoryView.h : header file
//

#ifndef _MemoryView_
#define _MemoryView_

#include "MemoryDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryView view

class CMemoryView : public CView
{
  int m_nCx;	// ilo�� kolumn
  int m_nCy;	// ilo�� wierszy
  int m_nChrW;	// szeroko�� znak�w (font mono)
  int m_nChrH;	// wysoko�� znak�w

  void calc(CDC *pDC);
  void scroll(UINT nSBCODE, int nPos, int nRepeat= 1);
  int set_scroll_range();
  void get_view_rect(RECT &rect)
  { GetClientRect(&rect); }
  int bytes_in_line();
  int find_prev_addr(UINT16 &addr, const COutputMem &mem, int cnt= 1, int bytes= 0);
  int find_next_addr(UINT16 &addr, const COutputMem &mem, int cnt= 1, int bytes= 0);
  int find_delta(UINT16 &addr, UINT16 dest, const COutputMem &mem, int max_lines);

  enum Dump { FULL, HEX, TEXT } m_eDump;
protected:
  CMemoryView();           // protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CMemoryView)
    
    // Attributes
public:
  static CFont m_Font;
  static LOGFONT m_LogFont;
  static COLORREF m_rgbTextColor;
  static COLORREF m_rgbBkgndColor;
  
  // Operations
public:
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMemoryView)
	public:
  virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
  virtual void OnInitialUpdate();
	protected:
  virtual void OnDraw(CDC* pDC);      // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
  
  // Implementation
protected:
  virtual ~CMemoryView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CMemoryView)
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnUpdateMemoryGoto(CCmdUI* pCmdUI);
  afx_msg void OnMemoryGoto();
  afx_msg void OnUpdateMemoryChg(CCmdUI* pCmdUI);
  afx_msg void OnMemoryChg();
  afx_msg void OnMemoryFull();
  afx_msg void OnMemoryHex();
  afx_msg void OnMemoryText();
  afx_msg void OnUpdateMemoryFull(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMemoryHex(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMemoryText(CCmdUI* pCmdUI);
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
