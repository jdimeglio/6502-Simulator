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

// DialEditBrkp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint dialog

class CDialEditBreakpoint : public CDialog, CAsm
{
  // Construction
public:
  CDialEditBreakpoint(Breakpoint bp, CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CDialEditBreakpoint)
  enum { IDD = IDD_EDIT_BREAKPOINT };
  bool	m_Disabled;
  bool	m_Execute;
  bool	m_Read;
  bool	m_Write;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDialEditBreakpoint)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CDialEditBreakpoint)
		// NOTE: the ClassWizard will add member functions here
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
