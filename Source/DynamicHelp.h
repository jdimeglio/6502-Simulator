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

#if !defined(AFX_ELEMENTBAR_H__3BAE8904_36AA_11D2_809C_ABC7258C6120__INCLUDED_)
#define AFX_ELEMENTBAR_H__3BAE8904_36AA_11D2_809C_ABC7258C6120__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PropertyBar.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CDynamicHelp window
struct TEXTBLOCK;


class CDynamicHelp : public CControlBar
{
	typedef CControlBar inherited;
	static PCSTR s_pcszWndClass;
	void SizeToolBar(int nLength, bool bVert= false);
	CToolTipCtrl m_ToolTip;
	CRichEditCtrl m_wndHelp;
	int m_nHeaderHeight;
	CToolBarCtrl m_wndClose;

// Construction
public:
	CDynamicHelp();

// Attributes
public:
	void SetContextHelp(const TCHAR* pcszText, const TCHAR* pcszHeader= 0);

	void DisplayHelp(const CString& strLine, int nWordStart, int nWordEnd);

// Operations
public:
	bool Create(CWnd* pParentWnd, UINT nID);


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDynamicHelp)
  //}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDynamicHelp();
	virtual void DoPaint(CDC* pDC);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	CSize CalcLayout(DWORD dwMode, int nLength);
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

  // Generated message map functions
protected:
	//{{AFX_MSG(CDynamicHelp)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCloseWnd();
	//}}AFX_MSG
	afx_msg BOOL OnToolTipGetText(UINT uId, NMHDR* pNmHdr, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	static CString s_strWndClass;
	void RegisterWndClass();
	CSize m_sizeDefault;
	void Resize();

	LRESULT OnDelayedResize(WPARAM, LPARAM);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELEMENTBAR_H__3BAE8904_36AA_11D2_809C_ABC7258C6120__INCLUDED_)
