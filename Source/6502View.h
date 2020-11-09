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

// 6502View.h : interface of the CSrc6502View class
//
/////////////////////////////////////////////////////////////////////////////
#include "DrawMarks.h"
#include "LeftBar.h"
class CSrc6502Doc;
class CMainFrame;
#ifdef USE_CRYSTAL_EDIT
	#include "CCrystalEditView.h"
	typedef CCrystalEditView CBaseView;
#else
	typedef CEditView CBaseView;
#endif


class CSrc6502View : public CBaseView, public CMarks
{
  void set_position_info(HWND hWnd);

  CMap<int, int, BYTE, BYTE> m_mapBreakpoints;	//TODO: move to the doc

  static LRESULT (CALLBACK *m_pfnOldProc)(HWND,UINT,WPARAM,LPARAM);

  static LRESULT CALLBACK EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void drawMark(CDC &dc, int line, MarkType type, bool scroll);
  void draw_breakpoints(HDC hDC= NULL);
  int ScrollToLine(int line, int &height, bool scroll= FALSE);

  void DrawMark(int line, MarkType type, bool scroll= FALSE, HDC hDC= NULL);

  void RedrawMarks(int line= -1);
  void EraseMark(int line);

  int m_nActualPointerLine;	//TODO: move to the doc
  int m_nActualErrMarkLine;	//TODO: move to the doc

  static void check_line(const TCHAR *buf, CAsm::Stat &stat, int &start, int &fin, CString &msg);
  void disp_warning(int line, CString &msg);
  afx_msg LRESULT OnRemoveErrMark(WPARAM wParam, LPARAM lParam);
public:
  static CFont m_Font;
  static LOGFONT m_LogFont;
  static COLORREF m_rgbTextColor;
  static COLORREF m_rgbBkgndColor;
  static bool m_bAutoIndent;
  static int m_nTabStep;
  static bool m_bAutoSyntax;
  static bool m_bAutoUppercase;
  static COLORREF m_vrgbColorSyntax[];
  static BYTE m_vbyFontStyle[];

  void SelectEditFont();

protected: // create from serialization only
  CSrc6502View();
  DECLARE_DYNCREATE(CSrc6502View)

// Attributes
public:
  void RemoveBreakpoint(int line, bool draw= TRUE);
  void AddBreakpoint(int line, CAsm::Breakpoint bp, bool draw= TRUE);
  int GetCurrLineNo();
  void SetErrMark(int line);		// narysowanie/zmazanie strza³ki wsk. b³¹d
  void SetPointer(int line, bool scroll= FALSE);
  CSrc6502Doc* GetDocument();

  afx_msg LRESULT OnPaintPointer(WPARAM /* wParam */, LPARAM /* lParam */);

  // edit view info
  void GetDispInfo(int& nTopLine, int& nLineCount, int& nLineHeight);

  int GetPointerLine() const      { return m_nActualPointerLine; }
  int GetErrorMarkLine() const    { return m_nActualErrMarkLine; }

  // return breakpoint info for line 'nLine'
  BYTE GetBreakpoint(int nLine) const;

#ifdef USE_CRYSTAL_EDIT
  using CBaseView::GetLineCount;
#else
  int GetLineCount()			{ return GetEditCtrl().GetLineCount(); }
#endif
  void GetText(CString& strText);

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CSrc6502View)
	public:
  virtual void OnDraw(CDC* pDC);  // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void OnInitialUpdate();
	protected:
  virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
  virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
  virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
  virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CSrc6502View();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CSrc6502View)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnEnUpdate();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
private:
  CLeftBar m_wndLeftBar;
  CMainFrame* m_pMainFrame;

#ifdef USE_CRYSTAL_EDIT
	virtual CCrystalTextBuffer* LocateTextBuffer();
	virtual DWORD ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK* pBuf, int& nActualItems);
	virtual void DrawMarginMarker(int nLine, CDC* pDC, const CRect &rect);
	virtual LineChange NotifyEnterPressed(CPoint ptCursor, CString& strLine);
	virtual void NotifyTextChanged();
	virtual COLORREF GetColor(int nColorIndex);
	virtual BOOL GetBold(int nColorIndex);
	virtual void CaretMoved(const CString& strLine, int nWordStart, int nWordEnd);
#endif
};

#ifndef _DEBUG  // debug version in 6502View.cpp
inline CSrc6502Doc* CSrc6502View::GetDocument()
   { return (CSrc6502Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
