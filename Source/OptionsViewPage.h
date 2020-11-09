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

// OptionsViewPage.h : header file
//

#include "ColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsViewPage dialog

class COptionsViewPage : public CPropertyPage
{
  CColorButton m_ColorButtonText;
  CColorButton m_ColorButtonBkgnd;
  BOOL m_bSubclassed;
  int m_nSelection;
  COLORREF m_rgbBkgndCol;
  COLORREF m_rgbTextCol;

  void repaint_example();

  DECLARE_DYNCREATE(COptionsViewPage)
    // Construction
public:
	struct TextDef
	{
		COLORREF text, bkgnd;
		CFont font;
		int changed;
		CBrush brush;
	};
	static TextDef m_Text[];

  COptionsViewPage();
  ~COptionsViewPage();
  
  // Dialog Data
  //{{AFX_DATA(COptionsViewPage)
  enum { IDD = IDD_PROPPAGE_VIEW };
	// NOTE - ClassWizard will add data members here.
	//    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsViewPage)
public:
  virtual BOOL OnSetActive();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsViewPage)
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnSelchangeViewWnd();
  afx_msg void OnViewTxtCol();
  afx_msg void OnViewBkgndCol();
  afx_msg void OnViewFontBtn();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
    
};
