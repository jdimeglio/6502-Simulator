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

#if !defined(AFX_LEFTBAR_H__6B81B5B6_5388_11D3_B4BA_000000000000__INCLUDED_)
#define AFX_LEFTBAR_H__6B81B5B6_5388_11D3_B4BA_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeftBar.h : header file
//
#include "DrawMarks.h"
class CSrc6502View;

/////////////////////////////////////////////////////////////////////////////
// CLeftBar window

class CLeftBar : public CControlBar, public CMarks
{
// Construction
public:
  CLeftBar();

// Attributes
public:

// Operations
public:
  bool Create(CWnd* pParent, CSrc6502View* pView);

  void DrawMark();
  void SetWidth(int nWidth);

  void RedrawLine(int nLine);

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CLeftBar)
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CLeftBar();

  virtual CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz )
  { return MySize(); }
  virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode )
  { return MySize(); }
  virtual void OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
  { }
  CSize MySize();

  virtual void DoPaint(CDC* pDC);

  COLORREF GetBkColor();

  // Generated message map functions
protected:
  //{{AFX_MSG(CLeftBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  int m_nBarWidth;
  CSrc6502View* m_pEditView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEFTBAR_H__6B81B5B6_5388_11D3_B4BA_000000000000__INCLUDED_)
