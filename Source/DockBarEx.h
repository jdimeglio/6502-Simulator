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

#if !defined(AFX_DOCKBAREX_H__CCF8FACC_8FB1_4DCF_825E_9E754C796CB8__INCLUDED_)
#define AFX_DOCKBAREX_H__CCF8FACC_8FB1_4DCF_825E_9E754C796CB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DockBarEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDockBarEx window

class CDockBarEx : public CDockBar
{
// Construction
public:
	CDockBarEx();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDockBarEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDockBarEx();
//	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDockBarEx)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExitSizeMove(WPARAM wParam, LPARAM lParam);

	CRect ResizeArea();
	bool m_bResize;
	CPoint m_ptStart;
	int m_nStartWidth;
	void ResizeBars(int nWidth);
	bool m_bResizing;
	int m_nDeltaWidth;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKBAREX_H__CCF8FACC_8FB1_4DCF_825E_9E754C796CB8__INCLUDED_)
