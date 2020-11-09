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

// ColorButton.h : header file
//

#ifndef _color_button_
#define _color_button_

/////////////////////////////////////////////////////////////////////////////
// CColorButton window

class CColorButton : public CButton
{
  const COLORREF *m_prgbColor;
  int m_nDx;
  int m_nDy;

  void PaintIt(int offset);

  // Construction
public:
  CColorButton();

  // Attributes
public:
  void SetColorRef(const COLORREF *pColor);
  afx_msg LRESULT OnSetState(WPARAM wParam, LPARAM /* lParam */);

  // Operations
public:
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CColorButton)
  //}}AFX_VIRTUAL
  
  // Implementation
public:
  virtual ~CColorButton();
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CColorButton)
  afx_msg void OnPaint();
	//}}AFX_MSG
  
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
