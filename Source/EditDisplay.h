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

// EditDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditDisplay window
class CSrc6502View;

class CEditDisplay : public CEdit
{
  CSrc6502View *m_pView;

  void draw_pointer(CDC &dc, int x, int y, int h);
  void draw_breakpoint(CDC &dc, int x, int y, int h, bool active);

  void MarkBreakpoint(int line);

// Construction
public:
  CEditDisplay();
//  CEditDisplay(CSrc6502View *pView);
  
  // Attributes
public:
  
  // Operations
public:
  void Subclass(CSrc6502View *pView);

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CEditDisplay)
  //}}AFX_VIRTUAL
  
  // Implementation
public:
  virtual ~CEditDisplay();
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CEditDisplay)
  afx_msg void OnPaint();
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  //}}AFX_MSG
  
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
