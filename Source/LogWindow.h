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

#if !defined(AFX_LOGWINDOW_H__8C02C384_F043_404A_A895_3D02CB27FAAA__INCLUDED_)
#define AFX_LOGWINDOW_H__8C02C384_F043_404A_A895_3D02CB27FAAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogWindow.h : header file
//
struct CmdInfo;


/////////////////////////////////////////////////////////////////////////////
// CLogWindow frame

class CLogWindow : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CLogWindow)

// Attributes
public:
	void SetText(const CommandLog& log);
	void Invalidate();
	CRect m_WndRect;
	bool m_bHidden;

// Operations
public:
	bool Create();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogWindow)
protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	CLogWindow();
	virtual ~CLogWindow();

	// Generated message map functions
	//{{AFX_MSG(CLogWindow)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont m_fntMono;
	CBrush m_brBackground;
	afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */);
	afx_msg LRESULT OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGWINDOW_H__8C02C384_F043_404A_A895_3D02CB27FAAA__INCLUDED_)
