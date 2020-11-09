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

#if !defined(AFX_INTREQUESTGENERATORDLG_H__1AEFC032_2D6B_4A3D_8DE5_5BB89CB76F80__INCLUDED_)
#define AFX_INTREQUESTGENERATORDLG_H__1AEFC032_2D6B_4A3D_8DE5_5BB89CB76F80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntRequestGeneratorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntRequestGeneratorDlg dialog

class CIntRequestGeneratorDlg : public CDialog
{
// Construction
public:
	CIntRequestGeneratorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIntRequestGeneratorDlg)
	enum { IDD = IDD_INTERRUPTS };
	BOOL	m_bGenerateIRQ;
	BOOL	m_bGenerateNMI;
	UINT	m_uIRQTimeLapse;
	UINT	m_uNMITimeLapse;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntRequestGeneratorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIntRequestGeneratorDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTREQUESTGENERATORDLG_H__1AEFC032_2D6B_4A3D_8DE5_5BB89CB76F80__INCLUDED_)
