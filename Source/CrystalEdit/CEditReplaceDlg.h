////////////////////////////////////////////////////////////////////////////
//	File:		CEditReplaceDlg.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Declaration of the CEditReplaceDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEDITREPLACEDLG_H__759417E3_7B18_11D2_8C50_0080ADB86836__INCLUDED_)
#define AFX_CEDITREPLACEDLG_H__759417E3_7B18_11D2_8C50_0080ADB86836__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CEditReplaceDlg.h : header file
//

#include "cedefs.h"
#include "editcmd.h"

class CCrystalEditView;

/////////////////////////////////////////////////////////////////////////////
// CEditReplaceDlg dialog

class CEditReplaceDlg : public CDialog
{
private:
	CCrystalEditView *m_pBuddy;
	BOOL m_bFound;
	CPoint m_ptFoundAt;
	BOOL DoHighlightText();

// Construction
public:
	CEditReplaceDlg(CCrystalEditView *pBuddy);

	BOOL m_bEnableScopeSelection;
	CPoint m_ptCurrentPos;
	CPoint m_ptBlockBegin, m_ptBlockEnd;

// Dialog Data
	//{{AFX_DATA(CEditReplaceDlg)
	enum { IDD = IDD_EDIT_REPLACE };
	BOOL	m_bMatchCase;
	BOOL	m_bWholeWord;
	CString	m_sText;
	CString	m_sNewText;
	int		m_nScope;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditReplaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditReplaceDlg)
	afx_msg void OnChangeEditText();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditReplace();
	afx_msg void OnEditReplaceAll();
	afx_msg void OnEditSkip();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEDITREPLACEDLG_H__759417E3_7B18_11D2_8C50_0080ADB86836__INCLUDED_)
