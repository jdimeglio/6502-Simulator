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

#if !defined(AFX_SYNTAXEXAMPLE_H__95642D27_FE87_4AD4_8404_9EB9599F46FA__INCLUDED_)
#define AFX_SYNTAXEXAMPLE_H__95642D27_FE87_4AD4_8404_9EB9599F46FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SyntaxExample.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSyntaxExample window

class CSyntaxExample : public CButton
{
// Construction
public:
	CSyntaxExample();

// Attributes
public:
	int m_nTabStop;
	HFONT m_hEditorFont;
	COLORREF m_rgbBackground;
	COLORREF m_rgbText;
	COLORREF m_rgbInstruction;
	COLORREF m_rgbDirective;
	COLORREF m_rgbComment;
	COLORREF m_rgbNumber;
	COLORREF m_rgbString;
	COLORREF m_rgbSelection;
	bool m_vbBold[5];

// Operations
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSyntaxExample)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSyntaxExample();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSyntaxExample)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYNTAXEXAMPLE_H__95642D27_FE87_4AD4_8404_9EB9599F46FA__INCLUDED_)
